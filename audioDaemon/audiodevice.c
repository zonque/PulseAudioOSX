/*
 *  audiodevice.c
 *  audioDaemon
 *
 *  Created by caiaq on 7/18/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "audiodevice.h"

enum {
	kPulseAudioMemoryInputSampleData = 0,
	kPulseAudioMemoryOutputSampleData
};

struct audiodevice {
	mach_port_t     async_port;
	io_connect_t    data_port;
	vm_address_t	audio_in_buf;
	vm_address_t	audio_out_buf;
};

struct audiodevice *audiodevice_create(io_service_t service)
{
	struct audiodevice *dev = malloc(sizeof(struct audiodevice));
    IOReturn ret;
	vm_size_t size;

	ret = IOServiceOpen(service, mach_task_self(), 0, &dev->data_port);
    if (ret) {
		printf ("IOServiceOpen() returned %08x\n", ret);
		free(dev);
		return NULL;
    }

    ret = IOCreateReceivePort(kOSAsyncCompleteMessageID, &dev->async_port);
    if (ret) {
		printf ("IOCreateReceiverPort() returned %08x\n", ret);
		free(dev);
		return NULL;
    }

	size = 1000;

	IOConnectMapMemory(dev->data_port, kPulseAudioMemoryInputSampleData, mach_task_self(),
					   &dev->audio_in_buf, &size, kIOMapAnywhere);

	IOConnectMapMemory(dev->data_port, kPulseAudioMemoryOutputSampleData, mach_task_self(),
					   &dev->audio_out_buf, &size, kIOMapAnywhere);

	printf("%s: buffers: %p in, %p out\n", __func__, dev->audio_in_buf, dev->audio_out_buf);
	
	return dev;
}
