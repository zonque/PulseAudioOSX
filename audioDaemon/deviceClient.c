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

#import <pulse/pulseaudio.h>
#import <pulse/mainloop.h>
#import <pulse/simple.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PAVirtualDeviceUserClientTypes.h"

#include "notificationCenter.h"
#include "deviceClient.h"

#define PAVirtualDeviceUserClass "org_pulseaudio_virtualdevice"

CFMutableArrayRef deviceArray;

struct audioDevice {
	io_connect_t data_port;
	mach_port_t async_port;

	struct PAVirtualDeviceInfo info;
	io_connect_t port;
	struct notificationBlock notificationBlock;

	vm_address_t audio_in_buf, audio_out_buf;
	int in_pos, out_pos;	
	
	pa_simple *s;
};

static char *notificationName[kPAVirtualDeviceUserClientNotificationMax] = {
	"kPAVirtualDeviceInfoUserClientNotificationEngineStarted",
	"kPAVirtualDeviceInfoUserClientNotificationEngineStopped",
	"kPAVirtualDeviceInfoUserClientNotificationSampleRateChanged",
};

static void 
notificationCallback(void *refcon, IOReturn result, void **args, int numArgs) 
{
	struct audioDevice *dev = (struct audioDevice *) refcon;
	struct notificationBlock *no = &dev->notificationBlock;
	
	if (no->notificationType >= kPAVirtualDeviceUserClientNotificationMax) {
		printf("%s(): bogus event %d\n", __func__, (int) no->notificationType);
		return;
	}
	
	printf(">>> notification >%s<, %08x.%08x value %d\n",
	       notificationName[no->notificationType],
	       (int) no->timeStampSec,
	       (int) no->timeStampNanoSec,
	       (int) no->value);
}

static OSStatus
triggerAsyncRead(struct audioDevice *dev)
{
	OSStatus ret;
	io_async_ref64_t asyncRef;
	uint64_t scalarRefs[8];
	
	scalarRefs[0] = CAST_USER_ADDR_T(&dev->notificationBlock);
	scalarRefs[1] = (uint64_t) sizeof(dev->notificationBlock);
	
	asyncRef[kIOAsyncCalloutFuncIndex] = CAST_USER_ADDR_T(&notificationCallback);
	asyncRef[kIOAsyncCalloutRefconIndex] = CAST_USER_ADDR_T(dev);

	ret = IOConnectCallAsyncScalarMethod(dev->data_port,					// mach_port_t      connection
					     kPAVirtualDeviceUserClientAsyncReadNotification,	// uint32_t	    selector
					     dev->async_port,					// mach_port_t      wake_port
					     asyncRef,						// uint64_t        *reference
					     kOSAsyncRef64Count,				// uint32_t         referenceCnt
					     scalarRefs,					// const uint64_t  *input
					     2,							// uint32_t         inputCnt
					     NULL,						// uint64_t        *output
					     NULL						// uint32_t        *outputCnt
					     );

	if (ret != kIOReturnSuccess)
		printf("%s():%d IOConnectCallAsyncScalarMethod() returned %08x\n", __func__, __LINE__, (int) ret);

	return kIOReturnSuccess;
}

static IOReturn
getDeviceInfo(struct audioDevice *dev)
{
	IOReturn ret;
	size_t size = sizeof(dev->info);
	
	ret = IOConnectCallStructMethod(dev->data_port,					// an io_connect_t returned from IOServiceOpen().
					kPAVirtualDeviceUserClientGetDeviceInfo,	// selector of the function to be called via the user client.
					NULL,						// pointer to the input struct parameter.
					0,						// the size of the input structure parameter.
					&dev->info,					// pointer to the output struct parameter.
					&size						// pointer to the size of the output structure parameter.
					);
	return ret;
}

static void
addDevice(io_service_t serviceObject)
{
	OSStatus ret;

	struct audioDevice *dev = malloc(sizeof(struct audioDevice));

	if (!dev) {
		printf("Unable to malloc dev!?\n");
		return;
	}

	memset(dev, 0, sizeof(*dev));			      
	
	ret = IOServiceOpen(serviceObject, mach_task_self(), 0, &dev->data_port);
	if (ret) {
		printf ("%s(): IOServiceOpen() returned %08x\n", __func__, (int) ret);
		return;
	}
	
	ret = IOCreateReceivePort(kOSAsyncCompleteMessageID, &dev->async_port);
	if (ret) {
		printf ("%s(): IOCreateReceiverPort() returned %08x\n", __func__, (int) ret);
		return;
	}
	
	CFRunLoopSourceRef      runLoopSource;
	CFMachPortContext       context;
	Boolean                 shouldFreeInfo;
	CFMachPortRef           cfPort;
	
	context.version = 1;
	context.info = dev;
	context.retain = NULL;
	context.release = NULL;
	context.copyDescription = NULL;
	
	cfPort = CFMachPortCreateWithPort(NULL, dev->async_port,
					  (CFMachPortCallBack) IODispatchCalloutFromMessage,
					  &context, &shouldFreeInfo);
	
	runLoopSource = CFMachPortCreateRunLoopSource(NULL, cfPort, 0);
	CFRelease(cfPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

	getDeviceInfo(dev);
	printf(" XXXXX AUDIODEVICE ADDED: >%s< (%d)\n", dev->info.name, serviceObject);

	triggerAsyncRead(dev);

	/* FIXME */
	if (dev->info.channelsIn != dev->info.channelsOut)
		return;

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								&kCFCopyStringDictionaryKeyCallBacks,
								&kCFTypeDictionaryValueCallBacks);
	
	CFDictionarySetValue(dict, CFSTR("name"), CFStringCreateWithCString(kCFAllocatorDefault, dev->info.name, kCFStringEncodingUTF8));
	CFDictionarySetValue(dict, CFSTR("channelsIn"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &dev->info.channelsIn));
	CFDictionarySetValue(dict, CFSTR("channelsOut"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &dev->info.channelsOut));
	CFDictionarySetValue(dict, CFSTR("serviceObject"), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &serviceObject));
	CFArrayAppendValue(deviceArray, dict);	
	notificationCenterSendDeviceList();
	
	vm_size_t memsize;

	if (dev->info.channelsIn) {
		memsize = dev->info.audioBufferSize;
		IOConnectMapMemory(dev->data_port, kPAMemoryInputSampleData, mach_task_self(),
				   &dev->audio_in_buf, &memsize, kIOMapAnywhere);
		
		if (!dev->audio_in_buf)
			printf("%s(): unable to map virtual audio IN memory. (size %d)\n", __func__, (int) memsize);
	}

	if (dev->info.channelsOut) {
		memsize = dev->info.audioBufferSize;
		IOConnectMapMemory(dev->data_port, kPAMemoryOutputSampleData, mach_task_self(),
				   &dev->audio_out_buf, &memsize, kIOMapAnywhere);
		
		if (!dev->audio_out_buf)
			printf("%s(): unable to map virtual audio OUT memory. (size %d)\n", __func__, (int) memsize);

	}	
	
	pa_sample_spec ss;
	
	ss.format = PA_SAMPLE_FLOAT32;
	ss.channels = dev->info.channelsOut;
	ss.rate = 44100;

	dev->s = pa_simple_new(NULL,			// Use the default server.
			       "audioDaemon",		// Our application's name.
			       PA_STREAM_PLAYBACK,
			       NULL,			// Use the default device.
			       dev->info.name,		// Description of our stream.
			       &ss,			// Our sample format.
			       NULL,			// Use default channel map
			       NULL,			// Use default buffering attributes.
			       NULL			// Ignore error code.
			       );	
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
			
			if (serviceObject == deviceServiceObject)
				CFArrayRemoveValueAtIndex(deviceArray, i);
		}
	}
}

IOReturn deviceClientStart(void)
{
	kern_return_t		ret;
	mach_port_t		masterPort;
	CFMutableDictionaryRef	classToMatch;
	IONotificationPortRef	gNotifyPort;
	io_iterator_t		gNewDeviceAddedIter;
	io_iterator_t		gNewDeviceRemovedIter;
	CFRunLoopSourceRef	runLoopSource;

	deviceArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);

	ret = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
	if (ret != kIOReturnSuccess) {
		printf("IOMasterPort returned %d\n", (int) ret);
		return false;
	}
	
	classToMatch = IOServiceMatching(PAVirtualDeviceUserClass);
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
