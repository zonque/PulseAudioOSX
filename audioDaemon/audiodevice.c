/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "audiodevice.h"
#include "../kext/PAUserClientTypes.h"

struct audioDevice {
	struct PAVirtualDevice info;
	vm_address_t audio_in_buf;
	vm_address_t audio_out_buf;
	io_connect_t port;
};

struct audioDevice *audioDevice_create(io_connect_t port, struct PAVirtualDevice *info)
{
	struct audioDevice *dev = malloc(sizeof(struct audioDevice));
	size_t retsize;
	vm_size_t memsize; 
	OSStatus ret;
	int index;

	if (!dev) {
		printf("%s(): unable to allocate memory\n", __func__);
		return NULL;
	}

	dev->port = port;
	
	retsize = sizeof(dev->info);
	ret = IOConnectCallStructMethod(port,				// an io_connect_t returned from IOServiceOpen().
					kPAUserClientAddDevice,		// selector of the function to be called via the user client.
					info,				// pointer to the input struct parameter.
					sizeof(*info),			// the size of the input structure parameter.
					&dev->info,			// pointer to the output struct parameter.
					&retsize			// pointer to the size of the output structure parameter.
					);

	if (ret) {
		printf("%s(): unable to create virtual audio device. ret = %08x\n", __func__, (int) ret);
		return NULL;
	}

	index = dev->info.index;
	memsize = dev->info.audioBufferSize;
	IOConnectMapMemory(port, kPAMemoryInputSampleData | (index << 8), mach_task_self(),
			   &dev->audio_in_buf, &memsize, kIOMapAnywhere);

	if (!dev->audio_in_buf) {
		printf("%s(): unable to map virtual audio IN memory. (size %d)\n", __func__, (int) memsize);
		return NULL;		
	}
	
	memsize = dev->info.audioBufferSize;
	IOConnectMapMemory(port, kPAMemoryOutputSampleData | (index << 8), mach_task_self(),
			   &dev->audio_in_buf, &memsize, kIOMapAnywhere);
	
	if (!dev->audio_out_buf) {
		printf("%s(): unable to map virtual audio OUT memory. (size %d)\n", __func__, (int) memsize);
		return NULL;		
	}

	return dev;
}

void audioDeviceRemove(struct audioDevice *dev)
{
	int index = dev->info.index;
	uint64_t scalar;

	if (dev->audio_in_buf)
		IOConnectUnmapMemory(dev->port, kPAMemoryInputSampleData | (index << 8),
				     mach_task_self(), dev->audio_in_buf);
	if (dev->audio_out_buf)
		IOConnectUnmapMemory(dev->port, kPAMemoryOutputSampleData | (index << 8),
				     mach_task_self(), dev->audio_out_buf);

	scalar = index;	
	IOConnectCallScalarMethod(dev->port,			// an io_connect_t returned from IOServiceOpen().
				  kPAUserClientRemoveDevice,	// selector of the function to be called via the user client.
				  &scalar,			// array of scalar (64-bit) input values.
				  1,				// the number of scalar input values.
				  NULL,				// array of scalar (64-bit) output values.
				  NULL				// pointer to the number of scalar output values.
				  );

	memset(dev, 0, sizeof(*dev));
	free(dev);
}

int audioDeviceGetIndex(struct audioDevice *dev)
{
	return dev->info.index;
}

io_connect_t audioDeviceGetPort(struct audioDevice *dev)
{
	return dev->port;
}
