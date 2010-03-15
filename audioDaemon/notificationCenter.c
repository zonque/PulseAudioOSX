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
#include "deviceClient.h"

void notificationCenterSendDeviceList (void)
{
	CFMutableDictionaryRef userInfo;

	userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
					     &kCFCopyStringDictionaryKeyCallBacks,
					     &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(userInfo, CFSTR("array"), deviceArray);

	CFNotificationCenterPostNotification (CFNotificationCenterGetDistributedCenter(),
					      CFSTR("updateDeviceList"),
					      CFSTR("PADaemon"),
					      userInfo,
					      true);
	CFRelease(userInfo);
}


static void sendDeviceListCallback (CFNotificationCenterRef center,
				    void *observer,
				    CFStringRef name,
				    const void *object,
				    CFDictionaryRef _userInfo)
{
	notificationCenterSendDeviceList();
}

static void addDeviceCallback (CFNotificationCenterRef center,
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

static void removeDeviceCallback (CFNotificationCenterRef center,
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
					 sendDeviceListCallback,
					 CFSTR("sendDeviceList"),
					 CFSTR("PAPreferencePane"),
					 CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver (CFNotificationCenterGetDistributedCenter(),
					 NULL, //const void *observer,
					 addDeviceCallback,
					 CFSTR("addDevice"),
					 CFSTR("PAPreferencePane"),
					 CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver (CFNotificationCenterGetDistributedCenter(),
					 NULL, //const void *observer,
					 removeDeviceCallback,
					 CFSTR("removeDevice"),
					 CFSTR("PAPreferencePane"),
					 CFNotificationSuspensionBehaviorDeliverImmediately);
	return 0;
}