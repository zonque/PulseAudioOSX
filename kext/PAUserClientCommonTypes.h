/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PA_USERCLIENT_COMMON_TYPES_H
#define PA_USERCLIENT_COMMON_TYPES_H

#define DEVICENAME_MAX	64

struct PAVirtualDeviceInfo {
	/* to be provided upon creation */
	char name[DEVICENAME_MAX];
	UInt32 channelsIn, channelsOut;
	UInt32 clockDirection;
	UInt32 blockSize;

	/* fields to read back from virtual device */
	UInt32 index;
	UInt32 currentSamplerate;
	UInt32 nUsers;
	UInt32 audioBufferSize;
	
};

/* clockDirection values */
enum {
	kPADeviceClockFromKernel = 0,
	kPADeviceClockFromClient,
};

#endif /* PA_USERCLIENT_COMMON_TYPES_H */
