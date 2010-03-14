/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PA_VIRTUALDEVICE_USERCLIENT_TYPES_H
#define PA_VIRTUALDEVICE_USERCLIENT_TYPES_H

#define DEVICENAME_MAX	64

/* synchronous functions */
enum {
	kPAVirtualDeviceUserClientGetDeviceInfo	= 0,
	kPAVirtualDeviceUserClientSetSampleRate	= 1,
};

/* asynchronous functions */
enum {
	kPAVirtualDeviceUserClientAsyncReadSamplePointer	= 0,
	kPAVirtualDeviceUserClientAsyncReadNotification		= 1,
};

/* notification types */
enum {
	kPAVirtualDeviceUserClientNotificationEngineStarted	= 0,
	kPAVirtualDeviceUserClientNotificationEngineStopped	= 1,
	kPAVirtualDeviceUserClientNotificationSampleRateChanged	= 2,
	kPAVirtualDeviceUserClientNotificationMax,
};

/* shared memory */
enum {
	kPAMemoryInputSampleData = 0,
	kPAMemoryOutputSampleData
};

/* types for asyncronous callback events */
struct samplePointerUpdateEvent {
	UInt32 timeStampSec;
	UInt32 timeStampNanoSec;
	UInt32 samplePointer;
};

struct notificationBlock {
	UInt32 timeStampSec;
	UInt32 timeStampNanoSec;
	UInt32 notificationType;
	UInt32 value;
};

#endif /* PA_VIRTUALDEVICE_USERCLIENT_TYPES_H */

