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

#include "audioDevice.h"

#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PAVirtualDeviceUserClientTypes.h"

#define PADriverClassName "org_pulseaudio_driver"

mach_port_t driver_async_port;
io_connect_t driver_data_port;

IOReturn addDeviceFromInfo (struct PAVirtualDeviceInfo *info)
{
	IOReturn ret;
	
	ret = IOConnectCallStructMethod(driver_data_port,				// an io_connect_t returned from IOServiceOpen().
					kPADriverUserClientAddDevice,			// selector of the function to be called via the user client.
					info,					// pointer to the input struct parameter.
					sizeof(*info),				// the size of the input structure parameter.
					NULL,					// pointer to the output struct parameter.
					NULL					// pointer to the size of the output structure parameter.
					);
	return ret;
}

static void 
serviceMatched (void *refCon, io_iterator_t iterator)
{
	io_service_t serviceObject;
	IOReturn ret;
	
	serviceObject = IOIteratorNext(iterator);
	if (!serviceObject)
		return;
	
	if (driver_data_port) {
		printf ("%s(): Ooops - more than one driver instance!?\n", __func__);
		return;
	}
	
	ret = IOServiceOpen(serviceObject, mach_task_self(), 0, &driver_data_port);
	if (ret) {
		printf ("%s(): IOServiceOpen() returned %08x\n", __func__, ret);
		return;
	}
	
	ret = IOCreateReceivePort(kOSAsyncCompleteMessageID, &driver_async_port);
	if (ret) {
		printf ("%s(): IOCreateReceiverPort() returned %08x\n", __func__, ret);
		return;
	}
	
	CFRunLoopSourceRef      runLoopSource;
	CFMachPortContext       context;
	Boolean                 shouldFreeInfo;
	CFMachPortRef           cfPort;
	
	context.version = 1;
	//context.info = this;
	context.retain = NULL;
	context.release = NULL;
	context.copyDescription = NULL;
	
	cfPort = CFMachPortCreateWithPort(NULL, driver_async_port,
					  (CFMachPortCallBack) IODispatchCalloutFromMessage,
					  &context, &shouldFreeInfo);
	
	runLoopSource = CFMachPortCreateRunLoopSource(NULL, cfPort, 0);
	CFRelease(cfPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
	
	struct PAVirtualDeviceInfo info;
	
	memset(&info, 0, sizeof(info));
	strcpy(info.name, "gaga.");
	info.channelsIn = 2;
	info.channelsOut = 2;
	info.blockSize = 512;
	
	ret = addDeviceFromInfo(&info);
}

static void 
serviceTerminated (void *refCon, io_iterator_t iterator)
{
	io_service_t serviceObject;
	
	while ((serviceObject = IOIteratorNext(iterator))) {
		// TODO
	}
}

IOReturn driverClientStart(void)
{
	kern_return_t		ret;
	mach_port_t		masterPort;
	CFMutableDictionaryRef	classToMatch;
	IONotificationPortRef	gNotifyPort;
	io_iterator_t		gNewDeviceAddedIter;
	io_iterator_t		gNewDeviceRemovedIter;
	CFRunLoopSourceRef	runLoopSource;

	ret = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
	if (ret != kIOReturnSuccess) {
		printf("IOMasterPort returned %d\n", (int) ret);
		return false;
	}

	classToMatch = IOServiceMatching(PADriverClassName);
	if (!classToMatch) {
		printf("%s(): IOServiceMatching returned a NULL dictionary.\n", __func__);
		return kIOReturnError;
	}
	
	// increase the reference count by 1 since the dict is used twice.
	classToMatch = (CFMutableDictionaryRef) CFRetain(classToMatch);
	
	gNotifyPort = IONotificationPortCreate(masterPort);
	runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOFirstMatchNotification,
					       classToMatch,
					       serviceMatched,
					       NULL,
					       &gNewDeviceAddedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceMatched(NULL, gNewDeviceAddedIter);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOTerminatedNotification,
					       classToMatch,
					       serviceTerminated,
					       NULL,
					       &gNewDeviceRemovedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceTerminated(NULL, gNewDeviceRemovedIter);

	mach_port_deallocate(mach_task_self(), masterPort);

	return kIOReturnSuccess;
}
