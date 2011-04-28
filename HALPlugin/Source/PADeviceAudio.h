/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <PulseAudio/PulseAudio.h>

@class PADevice;

typedef struct IOProcTracker IOProcTracker;

@interface PADeviceAudio : NSObject
{
	PADevice *device;
	IOProcTracker *tracker;
	NSLock *lock;
	UInt32 ioProcBufferSize;
	UInt32 bytesPerSampleFrame;
	Float64 sampleRate;
}

@property (assign) UInt32 ioProcBufferSize;
@property (readonly) UInt32 bytesPerSampleFrame;
@property (assign) Float64 sampleRate;

- (id) initWithPADevice: (PADevice *) device;

- (AudioDeviceIOProcID) createIOProcID: (AudioDeviceIOProc) proc
			    clientData: (void *) clientData;
- (BOOL) hasActiveProcs;
- (UInt32) countActiveChannels;
- (BOOL) channelIsActive: (UInt32) channel;
- (void) setChannelActive: (UInt32) channel
		   active: (BOOL) active;

- (void) removeIOProcID: (AudioDeviceIOProcID) procID;
- (void) setIOProcIDEnabled: (AudioDeviceIOProcID) procID
		    enabled: (BOOL) enabled;
- (void) setAllIOProcs: (BOOL) enabled;
- (void) setStartTimeForProcID: (AudioDeviceIOProcID) procID
		     timeStamp: (AudioTimeStamp *) timeStamp
			 flags: (UInt32) flags;

- (UInt32) PAServerConnection: (PAServerConnection *) connection
	      hasPlaybackData: (Byte *) playbackData
		   recordData: (const Byte *) recordData
		     byteSize: (UInt32) byteSize;

@end
