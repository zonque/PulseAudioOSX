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

#define PADriverClassName "org_pulseaudio_ioaudiodriver"

static mach_port_t async_port;
static io_connect_t data_port;

static IOReturn addDeviceFromInfo (struct PAVirtualDevice *info)
{
	IOReturn ret;
	
	ret = IOConnectCallStructMethod(data_port,						// an io_connect_t returned from IOServiceOpen().
									kPAUserClientAddDevice,			// selector of the function to be called via the user client.
									info,							// pointer to the input struct parameter.
									sizeof(*info),					// the size of the input structure parameter.
									NULL,							// pointer to the output struct parameter.
									NULL							// pointer to the size of the output structure parameter.
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

	if (data_port) {
		printf ("%s(): Ooops - more than one driver instance!?\n", __func__);
		return;
	}

	ret = IOServiceOpen(serviceObject, mach_task_self(), 0, &data_port);
    if (ret) {
		printf ("%s(): IOServiceOpen() returned %08x\n", __func__, ret);
		return;
    }
	
    ret = IOCreateReceivePort(kOSAsyncCompleteMessageID, &async_port);
    if (ret) {
		printf ("%s(): IOCreateReceiverPort() returned %08x\n", __func__, ret);
		return;
    }
	
	struct PAVirtualDevice info;
	strcpy(info.name, "gaga.");
	info.channelsIn = 2;
	info.channelsOut = 2;

	ret = addDeviceFromInfo(&info);
	
	printf ("%s(): %08x\n", __func__, ret);
}

static void 
serviceTerminated (void *refCon, io_iterator_t iterator)
{
    io_service_t serviceObject;
	
    while ((serviceObject = IOIteratorNext(iterator))) {
		// TODO
	}
}

IOReturn listenToService(mach_port_t masterPort)
{
    OSStatus				ret;
    CFMutableDictionaryRef	classToMatch;
    IONotificationPortRef	gNotifyPort;
    io_iterator_t			gNewDeviceAddedIter;
    io_iterator_t			gNewDeviceRemovedIter;
	CFRunLoopSourceRef		runLoopSource;

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
	
	return kIOReturnSuccess;
}

void sendDeviceList (CFNotificationCenterRef center,
					 void *observer,
					 CFStringRef name,
					 const void *object,
					 CFDictionaryRef _userInfo)
{
    OSStatus ret;
	uint64_t scalar;
	unsigned int nDevices, n;
	CFMutableArrayRef array;
	CFMutableDictionaryRef userInfo;

	n = 1;
	ret = IOConnectCallScalarMethod(data_port,							// an io_connect_t returned from IOServiceOpen().
									kPAUserClientGetNumberOfDevices,	// selector of the function to be called via the user client.
									NULL,								// array of scalar (64-bit) input values.
									0,									// the number of scalar input values.
									&scalar,							// array of scalar (64-bit) output values.
									&n									// pointer to the number of scalar output values.
									);
	if (ret) {
		printf("Unable to get number of devices. ret = %08x\n", (int) ret);
		return;
	}
	
	nDevices = scalar;
	array = CFArrayCreateMutable(kCFAllocatorDefault, nDevices, NULL);

	for (n = 0; n < nDevices; n++) {
		CFMutableDictionaryRef dict;
		struct PAVirtualDevice info;
		size_t size = sizeof(info);
		scalar = n;

		ret = IOConnectCallMethod(data_port,					// an io_connect_t returned from IOServiceOpen().
								  kPAUserClientGetDeviceInfo,	// selector of the function to be called via the user client.
								  &scalar,						// array of scalar (64-bit) input values.
								  1,							// the number of scalar input values.
								  NULL,							// a pointer to the struct input parameter.
								  0,							// the size of the input structure parameter.
								  NULL,							// array of scalar (64-bit) output values.
								  NULL,							// pointer to the number of scalar output values.
								  &info,						// pointer to the struct output parameter.
								  &size							// pointer to the size of the output structure parameter.
								  );
		
		if (ret) {
			printf("Unable to get info for device #%d\n", n);
			continue;
		}
		
		dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
										 &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		CFDictionarySetValue(dict, CFSTR("name"), CFStringCreateWithCString(kCFAllocatorDefault, info.name, kCFStringEncodingUTF8));
		CFDictionarySetValue(dict, CFSTR("channelsIn"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.channelsIn));
		CFDictionarySetValue(dict, CFSTR("channelsOut"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &info.channelsOut));

		CFArrayAppendValue(array, dict);
		//CFRelease(dict);
	}
	
	userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
										 &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(userInfo, CFSTR("array"), array);

	CFShow(userInfo);
	
	CFNotificationCenterPostNotification (CFNotificationCenterGetDistributedCenter(),
										  CFSTR("updateDeviceList"),
										  CFSTR("PADaemon"),
										  userInfo,
										  true);
	CFRelease(userInfo);
	CFRelease(array);
}

void addDevice (CFNotificationCenterRef center,
				void *observer,
				CFStringRef name,
				const void *object,
				CFDictionaryRef userInfo)
{
	struct PAVirtualDevice info;
	CFNumberRef num;
	CFStringRef str;
	
	num = CFDictionaryGetValue(userInfo, CFSTR("channelsIn"));
	CFNumberGetValue(num, kCFNumberIntType, &info.channelsIn);

	num = CFDictionaryGetValue(userInfo, CFSTR("channelsOut"));
	CFNumberGetValue(num, kCFNumberIntType, &info.channelsOut);
	
	str = CFDictionaryGetValue(userInfo, CFSTR("name"));
	CFStringGetCString(str, info.name, sizeof(info.name), kCFStringEncodingUTF8);
	
	addDeviceFromInfo(&info);
}

void removeDevice (CFNotificationCenterRef center,
				   void *observer,
				   CFStringRef name,
				   const void *object,
				   CFDictionaryRef userInfo)
{
	CFNumberRef num = CFDictionaryGetValue(userInfo, CFSTR("index"));
	uint64_t scalar = 0;
	
	if (!num)
		return;

	CFNumberGetValue(num, kCFNumberIntType, &scalar);
	
	IOConnectCallScalarMethod(data_port,					// an io_connect_t returned from IOServiceOpen().
							  kPAUserClientRemoveDevice,	// selector of the function to be called via the user client.
							  &scalar,						// array of scalar (64-bit) input values.
							  1,							// the number of scalar input values.
							  NULL,							// array of scalar (64-bit) output values.
							  NULL							// pointer to the number of scalar output values.
							  );
}

int main (int argc, const char **argv) {

    mach_port_t		masterPort;
    kern_return_t	kernResult;

	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
    if (kernResult != kIOReturnSuccess) {
        printf("IOMasterPort returned %d\n", kernResult);
        return false;
    }

	listenToService(masterPort);
	mach_port_deallocate(mach_task_self(), masterPort);
	
	CFNotificationCenterAddObserver (CFNotificationCenterGetDistributedCenter(),
									 NULL, //const void *observer,
									 sendDeviceList,
									 CFSTR("sendDeviceList"),
									 CFSTR("PAPreferencePane"),
									 CFNotificationSuspensionBehaviorDeliverImmediately);

	CFNotificationCenterAddObserver (CFNotificationCenterGetDistributedCenter(),
									 NULL, //const void *observer,
									 addDevice,
									 CFSTR("addDevice"),
									 CFSTR("PAPreferencePane"),
									 CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver (CFNotificationCenterGetDistributedCenter(),
									 NULL, //const void *observer,
									 removeDevice,
									 CFSTR("removeDevice"),
									 CFSTR("PAPreferencePane"),
									 CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFRunLoopRun();
	
	return 0;
}
