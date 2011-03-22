/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <pulseaudio@zonque.org>
 
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

#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PAVirtualDeviceUserClientTypes.h"

#include "driverClient.h"
#include "deviceClient.h"

#define PADriverClassName "org_pulseaudio_driver"

io_connect_t driverDataPort;
CFRunLoopSourceRef runLoopSource;
io_iterator_t driverAddedIter;
io_iterator_t driverRemovedIter;


IOReturn addDeviceFromInfo (struct PAVirtualDeviceInfo *info)
{
	IOReturn ret;
	
	ret = IOConnectCallStructMethod(driverDataPort,			// an io_connect_t returned from IOServiceOpen().
					kPADriverUserClientAddDevice,	// selector of the function to be called via the user client.
					info,				// pointer to the input struct parameter.
					sizeof(*info),			// the size of the input structure parameter.
					NULL,				// pointer to the output struct parameter.
					NULL				// pointer to the size of the output structure parameter.
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
	
	if (driverDataPort) {
		printf("%s(): Ooops - more than one driver instance!?\n", __func__);
		return;
	}
	
	ret = IOServiceOpen(serviceObject, mach_task_self(), 0, &driverDataPort);
	if (ret) {
		printf("%s(): IOServiceOpen() returned %08x\n", __func__, ret);
		return;
	}
	
	ret = deviceClientStart();
	if (ret) {
		printf("deviceClientStart() returned %d\n", ret);
		return;
	}

	/* HACK */
	struct PAVirtualDeviceInfo info;
	
	memset(&info, 0, sizeof(info));
	strlcpy(info.name, "gagax", sizeof(info.name));
	strlcpy(info.server, "localhost", sizeof(info.server));	
	info.channelsIn = 2;
	info.channelsOut = 2;
	info.blockSize = 512;
	info.streamCreationType = kPADeviceStreamCreationPermanent;
	info.audioContentType = kPADeviceAudioContentMixdown;

	ret = addDeviceFromInfo(&info);
}

static void 
serviceTerminated (void *refCon, io_iterator_t iterator)
{
	io_service_t serviceObject;

	if (!(serviceObject = IOIteratorNext(iterator)))
		return;

	deviceClientStop();

	if (driverDataPort) {
		IOServiceClose(driverDataPort);
		driverDataPort = 0;
	}
}

IOReturn driverClientStart(void)
{
	kern_return_t		ret;
	mach_port_t		masterPort;
	CFMutableDictionaryRef	classToMatch;
	IONotificationPortRef	gNotifyPort;
	
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
					       &driverAddedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}

	// Iterate once to get already-present devices and arm the notification
	serviceMatched(NULL, driverAddedIter);

	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOTerminatedNotification,
					       classToMatch,
					       serviceTerminated,
					       NULL,
					       &driverRemovedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceTerminated(NULL, driverRemovedIter);

	mach_port_deallocate(mach_task_self(), masterPort);

	return kIOReturnSuccess;
}

void driverClientStop(void)
{
	if (runLoopSource) {
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
		CFRelease(runLoopSource);
		runLoopSource = NULL;
	}
	
	if (driverAddedIter) {
		IOObjectRelease(driverAddedIter);
		driverAddedIter = 0;
	}

	if (driverRemovedIter) {
		IOObjectRelease(driverRemovedIter);
		driverRemovedIter = 0;
	}
}
