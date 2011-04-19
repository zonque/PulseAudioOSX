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

#include "../kext/PADriverUserClientTypes.h"
#include "../kext/PAUserClientCommonTypes.h"
#include "../kext/PAVirtualDeviceUserClientTypes.h"

#include "virtualDeviceClient.h"
#include "pulseAudio.h"

#define PAVirtualDeviceUserClass "org_pulseaudio_virtualdevice"

CFMutableArrayRef virtualDeviceArray;
io_iterator_t virtualDeviceAddedIter;
io_iterator_t virtualDeviceRemovedIter;
CFRunLoopSourceRef virtualDeviceClientRunLoopSource;

struct audioDevice {
	io_connect_t data_port;
	mach_port_t async_port;
	CFRunLoopSourceRef runLoopSource;

	struct PAVirtualDeviceInfo info;
	io_connect_t port;
	struct notificationBlock notificationBlock;

	vm_address_t audio_in_buf, audio_out_buf;
	int in_pos, out_pos;
	int running;
	
	pa_context *pa_context;
	pa_stream *s;
};

static char *notificationName[kPAVirtualDeviceUserClientNotificationMax] = {
	"kPAVirtualDeviceUserClientNotificationEngineStarted",
	"kPAVirtualDeviceUserClientNotificationEngineStopped",
	"kPAVirtualDeviceUserClientNotificationSampleRateChanged",
};

static void noop_cb (pa_stream  *s, int success, void *userdata)
{
}

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
	
	switch (no->notificationType) {
		case kPAVirtualDeviceUserClientNotificationEngineStarted:
			dev->running = 1;
			if (pa_context_get_state(dev->pa_context) != PA_CONTEXT_READY)
				pa_context_connect(dev->pa_context, dev->info.server, 0, NULL);
			else if (dev->s)
				pa_stream_cork(dev->s, 0, noop_cb, dev);
			break;
		case kPAVirtualDeviceUserClientNotificationEngineStopped:
			dev->running = 0;
			if (dev->s)
				pa_stream_cork(dev->s, 1, noop_cb, dev);
			if (dev->info.streamCreationType == kPADeviceStreamCreationOnDemand)
				pa_context_disconnect(dev->pa_context);
			break;
	}
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

static void deviceWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	struct audioDevice *dev = userdata;
	char *buf = (char *) dev->audio_out_buf;
	int maxBytes = dev->info.audioBufferSize; 

	//pa_stream_begin_write(stream, &userdata, &nbytes);

	if (dev->out_pos + nbytes > maxBytes) {
		/* wrap case */
		pa_stream_write(stream, buf + dev->out_pos, maxBytes - dev->out_pos, NULL, 0, 0);
		dev->out_pos += nbytes;
		dev->out_pos %= maxBytes;
		pa_stream_write(stream, buf, dev->out_pos, NULL, 0, 0);
	} else {
		pa_stream_write(stream, buf + dev->out_pos, nbytes, NULL, 0, 0);
		dev->out_pos += nbytes;
	}

	//printf(" >>>> out_pos %d (+%d)\n", dev->out_pos, nbytes);
	
	struct samplePointerUpdateEvent ev;
	ev.samplePointer = dev->out_pos / (2 * sizeof(float));	
	IOConnectCallStructMethod(dev->data_port, kPAVirtualDeviceUserClientWriteSamplePointer, &ev, sizeof(ev), NULL, NULL);
}

static void contextStateCallback(pa_context *c, void *userdata)
{
	struct audioDevice *dev = userdata;
	vm_size_t memsize;

	pa_sample_spec ss;
	pa_buffer_attr buf_attr;
	
	ss.format = PA_SAMPLE_FLOAT32;
	ss.rate = 44100;

	buf_attr.maxlength = 1024000; //dev->info.audioBufferSize / (8 * sizeof(float)); // fixme
	buf_attr.minreq = 2048;
	buf_attr.prebuf = 2048;
	buf_attr.tlength = -1;

	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_READY:
			printf("Connection ready.\n");
			
			if (dev->info.channelsIn) {
				memsize = dev->info.audioBufferSize;
				ss.channels = dev->info.channelsIn;

				IOConnectMapMemory(dev->data_port, kPAMemoryInputSampleData, mach_task_self(),
						   &dev->audio_in_buf, &memsize, kIOMapAnywhere);
				
				if (!dev->audio_in_buf) {
					printf("%s(): unable to map virtual audio IN memory. (size %d)\n", __func__, (int) memsize);
					break;
				}
			}
			
			if (dev->info.channelsOut) {
				memsize = dev->info.audioBufferSize;
				ss.channels = dev->info.channelsOut;

				IOConnectMapMemory(dev->data_port, kPAMemoryOutputSampleData, mach_task_self(),
						   &dev->audio_out_buf, &memsize, kIOMapAnywhere);
				
				if (!dev->audio_out_buf) {
					printf("%s(): unable to map virtual audio OUT memory. (size %d)\n", __func__, (int) memsize);
					break;
				}
				
				dev->info.audioBufferSize = memsize;
				
				printf("mapped %d bytes @%p memsize %d\n", dev->info.audioBufferSize, dev->audio_out_buf, memsize);
				memset(dev->audio_out_buf, 0, dev->info.audioBufferSize);

				//PA_STREAM_START_CORKED
				dev->s = pa_stream_new(c, dev->info.name, &ss, NULL);
				pa_stream_set_write_callback(dev->s, deviceWriteCallback, dev);
				pa_stream_connect_playback(dev->s, NULL, &buf_attr,
							   !dev->running ? PA_STREAM_START_CORKED : 0, NULL, NULL);
			}
			
			triggerAsyncRead(dev);
			
			if (dev->running && dev->s)
				pa_stream_cork(dev->s, 0, noop_cb, dev);
			
			break;
		case PA_CONTEXT_TERMINATED:
			break;
		case PA_CONTEXT_FAILED:
		default:
			break;
	}
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
		printf("%s(): IOServiceOpen() returned %08x\n", __func__, (int) ret);
		return;
	}
	
	ret = IOCreateReceivePort(kOSAsyncCompleteMessageID, &dev->async_port);
	if (ret) {
		printf("%s(): IOCreateReceiverPort() returned %08x\n", __func__, (int) ret);
		return;
	}
	
	CFMachPortContext context;
	Boolean shouldFreeInfo;
	CFMachPortRef cfPort;
	
	context.version = 1;
	context.info = dev;
	context.retain = NULL;
	context.release = NULL;
	context.copyDescription = NULL;
	
	cfPort = CFMachPortCreateWithPort(NULL, dev->async_port,
					  (CFMachPortCallBack) IODispatchCalloutFromMessage,
					  &context, &shouldFreeInfo);
	
	dev->runLoopSource = CFMachPortCreateRunLoopSource(NULL, cfPort, 0);
	CFRelease(cfPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), dev->runLoopSource, kCFRunLoopDefaultMode);

	getDeviceInfo(dev);
	printf(" XXXXX VIRTUAL AUDIODEVICE ADDED: >%s< (%d)  @%p\n", dev->info.name, serviceObject, dev);

	/* FIXME */
//	if (dev->info.channelsIn != dev->info.channelsOut)
//		return;

	if (dev->info.channelsOut == 0)
		return;

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								&kCFCopyStringDictionaryKeyCallBacks,
								&kCFTypeDictionaryValueCallBacks);
	
	unsigned long devn = (unsigned long) dev;
	
	CFDictionarySetValue(dict, CFSTR("serviceObject"), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &serviceObject));
	CFDictionarySetValue(dict, CFSTR("dev"), CFNumberCreate(kCFAllocatorDefault, kCFNumberLongLongType, &devn));
	CFArrayAppendValue(virtualDeviceArray, dict);	

	/* PA connection */
	dev->pa_context = pa_context_new(pulseAudioAPI(), "audioDaemon");
	pa_context_set_state_callback(dev->pa_context, contextStateCallback, dev);

	if (dev->info.audioContentType == kPADeviceAudioContentIndividual)
		dev->running = true;

	if (dev->info.streamCreationType == kPADeviceStreamCreationPermanent ||
	    dev->info.audioContentType == kPADeviceAudioContentIndividual)
		pa_context_connect(dev->pa_context, NULL /* dev->info.server */, 0, NULL);
}

static void deviceRemoved(struct audioDevice *dev)
{
	//pa_stream_disconnect
	printf(" XXXXX VIRTUAL AUDIODEVICE REMOVED: >%s<  @%p\n", dev->info.name, dev);

	if (dev->runLoopSource) {
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), dev->runLoopSource, kCFRunLoopDefaultMode);
		CFRelease(dev->runLoopSource);
		dev->runLoopSource = NULL;
	}

	if (dev->audio_out_buf) {
		IOConnectUnmapMemory(dev->data_port, kPAMemoryOutputSampleData,
				     mach_task_self(), dev->audio_out_buf);
		dev->audio_out_buf = 0;
	}

	if (dev->audio_in_buf) {
		IOConnectUnmapMemory(dev->data_port, kPAMemoryInputSampleData,
				     mach_task_self(), dev->audio_in_buf);
		dev->audio_in_buf = 0;
	}

	if (dev->async_port) {
		mach_port_deallocate(mach_task_self(), dev->async_port);
		dev->async_port = 0;
	}
	
	if (dev->data_port) {
		IOServiceClose(dev->data_port);
		dev->data_port = 0;
	}
	
	free(dev);
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
		
		for (i = 0; i < CFArrayGetCount(virtualDeviceArray); i++) {
			io_service_t deviceServiceObject;
			CFDictionaryRef dict = CFArrayGetValueAtIndex(virtualDeviceArray, i);
			CFNumberRef number = CFDictionaryGetValue(dict, CFSTR("serviceObject"));
			CFNumberGetValue(number, kCFNumberLongType, &deviceServiceObject);
			
			if (serviceObject == deviceServiceObject) {
				unsigned long devn;
				CFNumberRef number = CFDictionaryGetValue(dict, CFSTR("dev"));
				CFNumberGetValue(number, kCFNumberLongLongType, &devn);				
				deviceRemoved((struct audioDevice *) devn);
				CFArrayRemoveValueAtIndex(virtualDeviceArray, i);
			}
		}
	}
}

IOReturn virtualDeviceClientStart(void)
{
	kern_return_t		ret;
	mach_port_t		masterPort;
	CFMutableDictionaryRef	classToMatch;
	IONotificationPortRef	gNotifyPort;

	virtualDeviceArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);

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
	virtualDeviceClientRunLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), virtualDeviceClientRunLoopSource, kCFRunLoopDefaultMode);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOFirstMatchNotification,
					       classToMatch,
					       serviceMatched,
					       NULL,
					       &virtualDeviceAddedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceMatched(NULL, virtualDeviceAddedIter);
	
	ret = IOServiceAddMatchingNotification(gNotifyPort,
					       kIOTerminatedNotification,
					       classToMatch,
					       serviceTerminated,
					       NULL,
					       &virtualDeviceRemovedIter);
	if (ret) {
		printf("%s(): IOServiceAddMatchingNotification() returned %08x\n", __func__, (int) ret);
		return ret;
	}
	
	// Iterate once to get already-present devices and arm the notification
	serviceTerminated(NULL, virtualDeviceRemovedIter);
	
	mach_port_deallocate(mach_task_self(), masterPort);
	
	return 0;
}

void virtualDeviceClientStop(void)
{
	if (virtualDeviceAddedIter) {
		IOObjectRelease(virtualDeviceAddedIter);
		virtualDeviceAddedIter = 0;
	}
	
	if (virtualDeviceRemovedIter) {
		IOObjectRelease(virtualDeviceRemovedIter);
		virtualDeviceRemovedIter = 0;
	}
	
	if (virtualDeviceClientRunLoopSource) {
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), virtualDeviceClientRunLoopSource, kCFRunLoopDefaultMode);
		CFRelease(virtualDeviceClientRunLoopSource);
		virtualDeviceClientRunLoopSource = NULL;
	}
}
