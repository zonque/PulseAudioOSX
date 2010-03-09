/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAUSERCLIENT_TYPES_H
#define PAUSERCLIENT_TYPES_H

#define DEVICENAME_MAX	64

/* clockDirection values */
enum {
	kPADeviceClockFromKernel = 0,
	kPADeviceClockFromClient,
};

/* synchronous functions */
enum {
	kPAUserClientGetNumberOfDevices	= 0,
	kPAUserClientAddDevice			= 1,
	kPAUserClientRemoveDevice		= 2,
	kPAUserClientGetDeviceInfo		= 3,
	kPAUserClientSetSampleRate		= 4,
};

enum {
	kPAUserClientAsyncReadSamplePointer	= 0,
	kPAUserClientAsyncReadNotification	= 1,
};

enum {
	kPAUserClientNotificationEngineStarted		= 0,
	kPAUserClientNotificationEngineStopped		= 1,
	kPAUserClientNotificationSampleRateChanged	= 2,	
};

/* shared memory */
enum {
	kPAMemoryInputSampleData = 0,
	kPAMemoryOutputSampleData
};

struct PAVirtualDevice {
	char name[DEVICENAME_MAX];
	UInt32 index;
	
	UInt32 channelsIn, channelsOut;
	UInt32 currentSamplerate;
	UInt32 nUsers;
	UInt32 clockDirection;
	UInt32 audioBufferSize;
	UInt32 blockSize;
};

struct samplePointerUpdateEvent {
	UInt32 timeStampSec;
	UInt32 timeStampNanoSec;
	UInt32 index;
	UInt32 samplePointer;
};

struct notificationBlock {
	UInt32 timeStampSec;
	UInt32 timeStampNanoSec;
	UInt32 notificationType;
	UInt32 value;
};

#endif /* PAUSERCLIENT_TYPES_H */

