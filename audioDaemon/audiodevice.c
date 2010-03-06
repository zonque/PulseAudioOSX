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

struct audiodevice {
	vm_address_t	audio_in_buf;
	vm_address_t	audio_out_buf;
};

struct audiodevice *audiodevice_create(io_connect_t port, int index)
{
	struct audiodevice *dev = malloc(sizeof(struct audiodevice));
	vm_size_t size;

	size = 1000;

	IOConnectMapMemory(port, kPAMemoryInputSampleData, mach_task_self(),
					   &dev->audio_in_buf, &size, kIOMapAnywhere);

	IOConnectMapMemory(port, kPAMemoryOutputSampleData, mach_task_self(),
					   &dev->audio_out_buf, &size, kIOMapAnywhere);

	printf("%s: buffers: %p in, %p out\n", __func__, dev->audio_in_buf, dev->audio_out_buf);
	
	return dev;
}
