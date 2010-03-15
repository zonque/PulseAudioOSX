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

#define PAVirtualDeviceUserClass "org_pulseaudio_virtualdevice"

struct audioDevice {
	io_connect_t data_port;
	mach_port_t async_port;

	struct PAVirtualDeviceInfo info;
	vm_address_t audio_in_buf;
	vm_address_t audio_out_buf;
	io_connect_t port;
	struct samplePointerUpdateEvent samplePointerUpdateEvent;
	struct notificationBlock notificationBlock;
};

static void 
samplePointerUpdateCallback(void *refcon, IOReturn result, void **args, int numArgs) 
{
	struct audioDevice *dev = refcon;
	struct samplePointerUpdateEvent *ev = &dev->samplePointerUpdateEvent;
	return;
	
	printf(">>> %08x.%08x  ... %08x\n",
	       (int) ev->timeStampSec,
	       (int) ev->timeStampNanoSec,
	       (int) ev->samplePointer);
}

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
	
	scalarRefs[0] = (uint64_t) &dev->samplePointerUpdateEvent;
	scalarRefs[1] = (uint64_t) sizeof(dev->samplePointerUpdateEvent);
	
	asyncRef[kIOAsyncCalloutFuncIndex] = &samplePointerUpdateCallback;
	asyncRef[kIOAsyncCalloutRefconIndex] = dev;

	ret = IOConnectCallAsyncScalarMethod(dev->data_port,					// mach_port_t      connection,         // In
					     kPAVirtualDeviceUserClientAsyncReadSamplePointer,	// uint32_t	    selector			// In
					     dev->async_port,					// mach_port_t      wake_port			// In
					     asyncRef,						// uint64_t        *reference			// In
					     kOSAsyncRef64Count,				// uint32_t         referenceCnt		// In
					     scalarRefs,					// const uint64_t  *input				// In
					     2,							// uint32_t         inputCnt			// In
					     NULL,						// uint64_t        *output				// Out
					     NULL						// uint32_t        *outputCnt			// In/Out
					     );

	if (ret != kIOReturnSuccess)
		printf("%s():%d IOConnectCallAsyncScalarMethod() returned %08x\n", __func__, __LINE__, (int) ret);
	
	scalarRefs[0] = (uint64_t) &dev->notificationBlock;
	scalarRefs[1] = (uint64_t) sizeof(dev->notificationBlock);
	
	asyncRef[kIOAsyncCalloutFuncIndex] = &notificationCallback;
	asyncRef[kIOAsyncCalloutRefconIndex] = dev;

	ret = IOConnectCallAsyncScalarMethod(dev->data_port,					// mach_port_t      connection,         // In
					     kPAVirtualDeviceUserClientAsyncReadNotification,	// uint32_t	    selector			// In
					     dev->async_port,					// mach_port_t      wake_port			// In
					     asyncRef,						// uint64_t        *reference			// In
					     kOSAsyncRef64Count,				// uint32_t         referenceCnt		// In
					     scalarRefs,					// const uint64_t  *input				// In
					     2,							// uint32_t         inputCnt			// In
					     NULL,						// uint64_t        *output				// Out
					     NULL						// uint32_t        *outputCnt			// In/Out
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
		printf(" <<<<<<< DEVICE REMOVED %d\n", serviceObject);
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
