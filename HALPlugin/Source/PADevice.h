/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <Foundation/Foundation.h>
#import "PAObject.h"
#import "PAStream.h"
#import "PAPlugin.h"

@interface PADevice : PAObject {
	NSMutableArray *inputStreamArray;
	NSMutableArray *outputStreamArray;

	NSString *deviceName;
	NSString *deviceManufacturer;
	NSString *modelUID;
	NSString *deviceUID;
	
	BOOL isRunning;
	
	UInt32 ioBufferFrameSize;
	Float64 sampleRate;
	
	AudioStreamBasicDescription streamDescription;
	AudioStreamRangedDescription physicalFormat;
}

- (PAStream *) findStreamByID: (AudioObjectID) searchID;

- (void) publishOwnedObjects;
- (void) depublishOwnedObjects;

#pragma mark ### Plugin interface ###

- (OSStatus) createIOProcID: (AudioDeviceIOProc) proc
		 clientData: (void *) clientData
		outIOProcID: (AudioDeviceIOProcID *) outIOProcID;

- (OSStatus) destroyIOProcID: (AudioDeviceIOProcID) inIOProcID;

- (OSStatus) addIOProc: (AudioDeviceIOProc) proc
	    clientData: (void *) clientData;

- (OSStatus) removeIOProc: (AudioDeviceIOProc) proc;

- (OSStatus) start: (AudioDeviceIOProc) inProcID;

- (OSStatus) startAtTime: (AudioDeviceIOProc) procID
    ioRequestedStartTime: (AudioTimeStamp *) ioRequestedStartTime
		   flags: (UInt32) flags;

- (OSStatus) stop: (AudioDeviceIOProc) procID;

- (OSStatus) read: (const AudioTimeStamp *) startTime
	  outData: (AudioBufferList *) outData;

- (OSStatus) getCurrentTime: (AudioTimeStamp *) outTime;

- (OSStatus) translateTime: (const AudioTimeStamp *) inTime
		   outTime: (AudioTimeStamp *) outTime;

- (OSStatus) getNearestStartTime: (AudioTimeStamp *) ioRequestedStartTime
			   flags: (UInt32) flags;

#pragma mark ### properties ###

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address;
- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address;

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
	       qualifierDataSize: (UInt32) qualifierDataSize
		   qualifierData: (const void *) qualifierData
			 outSize: (UInt32 *) outSize;

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		  ioDataSize: (UInt32 *) ioDataSize
		     outData: (void *) outData;

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		    dataSize: (UInt32) dataSize
			data: (const void *) data;

#pragma mark ### properties (legacy interface) ###

- (OSStatus) getPropertyInfo: (UInt32) channel
		     isInput: (BOOL) isInput
		  propertyID: (AudioDevicePropertyID) propertyID
		     outSize: (UInt32 *) outSize
		 outWritable: (BOOL *) outWritable;

- (OSStatus) getProperty: (UInt32) channel
		 isInput: (BOOL) isInput
	      propertyID: (AudioDevicePropertyID) property
	      ioDataSize: (UInt32 *) ioPropertyDataSize
		 outData: (void *) data;

- (OSStatus) setProperty: (const AudioTimeStamp *) when
		 channel: (UInt32) channel
		 isInput: (BOOL) isInput
	      propertyID: (AudioDevicePropertyID) propertyID
		dataSize: (UInt32) dataSize
		    data: (const void *) data;

@end
