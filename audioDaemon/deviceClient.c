/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <pulseaudio@zonque.org>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFNumber.h>

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PADeviceUserClientTypes.h"

#include "notificationCenter.h"
#include "deviceClient.h"
#include "pulseAudio.h"

#define PADeviceUserClass "org_pulseaudio_device"

CFMutableArrayRef deviceArray;
io_iterator_t deviceAddedIter;
io_iterator_t deviceRemovedIter;
CFRunLoopSourceRef deviceClientRunLoopSource;

struct audioDevice {
	io_connect_t data_port;
	mach_port_t async_port;
	CFRunLoopSourceRef runLoopSource;

	struct PAVirtualDeviceInfo info;
	io_connect_t port;
};

static void
addDevice(io_service_t serviceObject)
{
	OSStatus ret;
	io_connect_t data_port;
	struct PAVirtualDeviceInfo info;
	size_t size = sizeof(info);

	ret = IOServiceOpen(serviceObject, mach_task_self(), 0, &data_port);
	if (ret) {
		printf("%s(): IOServiceOpen() returned %08x\n", __func__, (int) ret);
		return;
	}
		
	ret = IOConnectCallStructMethod(data_port,				// an io_connect_t returned from IOServiceOpen().
					kPADeviceUserClientGetDeviceInfo,	// selector of the function to be called via the user client.
					NULL,					// pointer to the input struct parameter.
					0,					// the size of the input structure parameter.
					&info,					// pointer to the output struct parameter.
					&size					// pointer to the size of the output structure parameter.
					);

	printf(" XXXXX AUDIODEVICE ADDED: >%s< (%d)\n", info.name, serviceObject);

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								&kCFCopyStringDictionaryKeyCallBacks,
								&kCFTypeDictionaryValueCallBacks);
	
	CFDictionarySetValue(dict, CFSTR("name"), CFStringCreateWithCString(kCFAllocatorDefault, info.name, kCFStringEncodingUTF8));
	CFDictionarySetValue(dict, CFSTR("server"), CFStringCreateWithCString(kCFAllocatorDefault, info.server, kCFStringEncodingUTF8));
	CFDictionarySetValue(dict, CFSTR("channelsIn"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.channelsIn));
	CFDictionarySetValue(dict, CFSTR("channelsOut"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.channelsOut));
	CFDictionarySetValue(dict, CFSTR("audioContentType"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.audioContentType));
	CFDictionarySetValue(dict, CFSTR("streamCreationType"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.streamCreationType));
	CFDictionarySetValue(dict, CFSTR("serviceObject"), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &serviceObject));
	CFArrayAppendValue(deviceArray, dict);	
	notificationCenterSendDeviceList();
}

static void 
serviceMatched (void *refCon, io_iterator_t iterator)
{
	io_service_t serviceObject;
	
	while ((serviceObject = IOIteratorNext(iterator)))
		addDevice(serviceObject);
}

static void 
serviceTerminated (void *refCon, io_iterator_t iterator)
{
	io_service_t serviceObject;
	
	while ((serviceObject = IOIteratorNext(iterator))) {
		int i;
		
		for (i = 0; i < CFArrayGetCount(deviceArray); i++) {
			io_service_t deviceServiceObject;
			CFDictionaryRef dict = CFArrayGetValueAtIndex(deviceArray, i);
			CFNumberRef number = CFDictionaryGetValue(dict, CFSTR("serviceObject"));
			CFNumberGetValue(number, kCFNumberLongType, &deviceServiceObject);
			
			if (serviceObject == deviceServiceObject) {
				CFArrayRemoveValueAtIndex(deviceArray, i);
				break;
			}
		}
	}
}

IOReturn deviceClientStart(void)
{
	kern_return_t		ret;
	mach_port_t		masterPort;
	CFMutableDictionaryRef	classToMatch;
	IONotificationPortRef	gNotifyPort;
	
	deviceArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
	
	ret = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
	if (ret != kIOReturnSuccess) {
		printf("IOMasterPort returned %d\n", (int) ret);
		return false;
	}
	
	classToMatch = IOServiceMatching(PADeviceUserClass);
	if (!classToMatch) {
		printf("%s(): IOServiceMatching returned a NULL dictionary.\n", __func__);
		return kIOReturnError;
	}
	
	// increase the reference count by 1 since the dict is used twice.
	classToMatch = (CFMutableDictionaryRef) CFRetain(classToMatch);
	
	gNotifyPort = IONotificationPortCreate(masterPort);
	deviceClientRunLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), deviceClientRunLoopSource, kCFRunLoopDefaultMode);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOFirstMatchNotification,
					       classToMatch,
					       serviceMatched,
					       NULL,
					       &deviceAddedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceMatched(NULL, deviceAddedIter);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOTerminatedNotification,
					       classToMatch,
					       serviceTerminated,
					       NULL,
					       &deviceRemovedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceTerminated(NULL, deviceRemovedIter);
	
	mach_port_deallocate(mach_task_self(), masterPort);
	
	return 0;
}

void deviceClientStop(void)
{
	if (deviceAddedIter) {
		IOObjectRelease(deviceAddedIter);
		deviceAddedIter = 0;
	}
	
	if (deviceRemovedIter) {
		IOObjectRelease(deviceRemovedIter);
		deviceRemovedIter = 0;
	}
	
	if (deviceClientRunLoopSource) {
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), deviceClientRunLoopSource, kCFRunLoopDefaultMode);
		CFRelease(deviceClientRunLoopSource);
		deviceClientRunLoopSource = NULL;
	}
}
