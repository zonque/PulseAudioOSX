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

@interface PAPlugin : PAObject {
	NSMutableArray *devicesArray;
}

#pragma mark ### PAObject ###

- (id) initWithPluginRef: (AudioHardwarePlugInRef) pluginRef;
- (void) publishOwnedObjects;
- (void) depublishOwnedObjects;

- (PAObject *) findObjectByID: (AudioObjectID) searchID;

#pragma mark ### Plugin interface ###

- (OSStatus) initialize;
- (OSStatus) initializeWithObjectID: (AudioObjectID) oid;

- (void) objectShow: (AudioObjectID) oid;

- (BOOL) objectHasProperty: (AudioObjectID) oid
		  propertyID: (const AudioObjectPropertyAddress *) property;

- (OSStatus) objectIsPropertySettable: (AudioObjectID) oid
			   propertyID: (const AudioObjectPropertyAddress *) property
		outIsPropertySettable: (BOOL *) outIsPropertySettable;

- (OSStatus) objectGetPropertyDataSize: (AudioObjectID) inObjectID
			    propertyID: (const AudioObjectPropertyAddress *) inAddress
		     qualifierDataSize: (UInt32) qualifierDataSize
			 qualifierData: (const void *) qualifierData
		   outPropertyDataSize: (UInt32 *) outDataSize;

- (OSStatus) objectGetPropertyData: (AudioObjectID) oid
			propertyID: (const AudioObjectPropertyAddress *) property
		 qualifierDataSize: (UInt32) qualifierDataSize
		     qualifierData: (const void *) qualifierData
			ioDataSize: (UInt32 *) ioDataSize
			   outData: (void *) outData;

- (OSStatus) objectSetPropertyData: (AudioObjectID) oid
			propertyID: (const AudioObjectPropertyAddress *) property
		 qualifierDataSize: (UInt32) qualifierDataSize
		     qualifierData: (const void *) qualifierData
			  dataSize: (UInt32) dataSize
			      data: (const void *) data;

- (OSStatus) deviceCreateIOProcID: (AudioDeviceID) did
			     proc: (AudioDeviceIOProc) proc
		       clientData: (void *) clientData
		      outIOProcID: (AudioDeviceIOProcID *) outIOProcID;

- (OSStatus) deviceDestroyIOProcID: (AudioDeviceID) did
			    procID: (AudioDeviceIOProcID) procID;

- (OSStatus) deviceAddIOProc: (AudioDeviceID) did
			proc: (AudioDeviceIOProc) proc
		  clientData: (void *) clientData;

- (OSStatus) deviceRemoveIOProc: (AudioDeviceID) did
			   proc: (AudioDeviceIOProc) proc;

- (OSStatus) deviceStart: (AudioDeviceID) did
		  procID: (AudioDeviceIOProcID) procID;

- (OSStatus) deviceStartAtTime: (AudioDeviceID) did
			procID: (AudioDeviceIOProcID) procID
	    requestedStartTime: (AudioTimeStamp *) requestedStartTime
			 flags: (UInt32) flags;

- (OSStatus) deviceStop: (AudioDeviceID) did
		 procID: (AudioDeviceIOProcID) procID;

- (OSStatus) deviceRead: (AudioDeviceID) did
	      startTime: (const AudioTimeStamp *) startTime
		outData: (AudioBufferList *) outData;

- (OSStatus) deviceGetCurrentTime: (AudioDeviceID) did
			  outTime: (AudioTimeStamp *) outTime;

- (OSStatus) deviceTranslateTime: (AudioDeviceID) did
			  inTime: (const AudioTimeStamp *) inTime
			 outTime: (AudioTimeStamp *) outTime;

- (OSStatus) deviceGetNearestStartTime: (AudioDeviceID) did
		    requestedStartTime: (AudioTimeStamp *) requestedStartTime
				 flags: (UInt32) flags;

- (OSStatus) deviceGetPropertyInfo: (AudioDeviceID) did
			   channel: (UInt32) channel
			   isInput: (BOOL) isInput
			propertyID: (AudioDevicePropertyID) property
			   outSize: (UInt32 *) outSize
		     outIsWritable: (BOOL *) outIsWritable;

- (OSStatus) deviceGetProperty: (AudioDeviceID) did
		       channel: (UInt32) channel
		       isInput: (BOOL) isInput
		    propertyID: (AudioDevicePropertyID) property
		    ioDataSize: (UInt32 *) ioDataSize
		       outData: (void *) outData;

- (OSStatus) deviceSetProperty: (AudioDeviceID) did
			  when: (const AudioTimeStamp *) when
		       channel: (UInt32) channel
		       isInput: (BOOL) isInput
		    propertyID: (AudioDevicePropertyID) property
		      dataSize: (UInt32) dataSize
			  data: (const void *) data;

- (OSStatus) streamGetPropertyInfo: (AudioStreamID) sid
			   channel: (UInt32) channel
			propertyID: (AudioDevicePropertyID) property
			   outSize: (UInt32 *) outSize
		     outIsWritable: (BOOL *) outWritable;

- (OSStatus) streamGetProperty: (AudioStreamID) sid
		       channel: (UInt32) channel
		    propertyID: (AudioDevicePropertyID) property
		    ioDataSize: (UInt32 *) ioDataSize
		       outData: (void *) outData;

- (OSStatus) streamSetProperty: (AudioStreamID) sid
			  when: (const AudioTimeStamp *) when
		       channel: (UInt32) channel
		    propertyID: (AudioDevicePropertyID) property
		      dataSize: (UInt32) dataSize
			  data: (const void *) data;

@end
