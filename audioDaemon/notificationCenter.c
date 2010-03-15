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

#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PAVirtualDeviceUserClientTypes.h"

#include "driverClient.h"

static void sendDeviceList (CFNotificationCenterRef center,
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
	
#if 0
	n = 1;
	ret = IOConnectCallScalarMethod(data_port,				// an io_connect_t returned from IOServiceOpen().
					kPAUserClientGetNumberOfDevices,	// selector of the function to be called via the user client.
					NULL,					// array of scalar (64-bit) input values.
					0,					// the number of scalar input values.
					&scalar,				// array of scalar (64-bit) output values.
					&n					// pointer to the number of scalar output values.
					);
	if (ret) {
		printf("Unable to get number of devices. ret = %08x\n", (int) ret);
		return;
	}
	
	nDevices = scalar;
	array = CFArrayCreateMutable(kCFAllocatorDefault, nDevices, NULL);
	
	for (n = 0; n < nDevices; n++) {
		CFMutableDictionaryRef dict;
		struct PAVirtualDeviceInfo info;
		size_t size = sizeof(info);
		scalar = n;
		
		ret = IOConnectCallMethod(data_port,			// an io_connect_t returned from IOServiceOpen().
					  kPAUserClientGetDeviceInfo,	// selector of the function to be called via the user client.
					  &scalar,			// array of scalar (64-bit) input values.
					  1,				// the number of scalar input values.
					  NULL,				// a pointer to the struct input parameter.
					  0,				// the size of the input structure parameter.
					  NULL,				// array of scalar (64-bit) output values.
					  NULL,				// pointer to the number of scalar output values.
					  &info,			// pointer to the struct output parameter.
					  &size				// pointer to the size of the output structure parameter.
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
#endif
	
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

static void addDevice (CFNotificationCenterRef center,
		       void *observer,
		       CFStringRef name,
		       const void *object,
		       CFDictionaryRef userInfo)
{
	struct PAVirtualDeviceInfo info;
	CFNumberRef num;
	CFStringRef str;
	
	num = CFDictionaryGetValue(userInfo, CFSTR("channelsIn"));
	CFNumberGetValue(num, kCFNumberIntType, &info.channelsIn);
	
	num = CFDictionaryGetValue(userInfo, CFSTR("channelsOut"));
	CFNumberGetValue(num, kCFNumberIntType, &info.channelsOut);
	
	num = CFDictionaryGetValue(userInfo, CFSTR("blockSize"));
	CFNumberGetValue(num, kCFNumberIntType, &info.blockSize);
	
	str = CFDictionaryGetValue(userInfo, CFSTR("name"));
	CFStringGetCString(str, info.name, sizeof(info.name), kCFStringEncodingUTF8);
	
	addDeviceFromInfo(&info);
}

static void removeDevice (CFNotificationCenterRef center,
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
	
	IOConnectCallScalarMethod(driver_data_port,			// an io_connect_t returned from IOServiceOpen().
				  kPADriverUserClientRemoveDevice,	// selector of the function to be called via the user client.
				  &scalar,				// array of scalar (64-bit) input values.
				  1,					// the number of scalar input values.
				  NULL,					// array of scalar (64-bit) output values.
				  NULL					// pointer to the number of scalar output values.
				  );
}

IOReturn notificationCenterStart(void)
{
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
	return 0;
}