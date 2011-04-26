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

@class PADevice;

@interface PAStream : PAObject {
	PADevice *owningDevice;
	BOOL isInput;
	UInt32 startingChannel;
}

- (id) initWithDevice: (PADevice *) device
	      isInput: (BOOL) isInput
      startingChannel: (UInt32) startingChannel;

- (PAObject *) findObjectByID: (AudioObjectID) searchID;

#pragma mark ### properties ###

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address;
- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address;

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
	       qualifierDataSize: (UInt32) qualifierDataSize
		   qualifierData: (const void *) qualifierData
			 outSize: (UInt32 *) outDataSize;

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		  ioDataSize: (UInt32 *) ioDataSize
		     outData: (void *) outData;

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		    dataSize: (UInt32) inDataSize
			data: (const void *) data;

#pragma mark ### properties (legacy interface) ###

- (OSStatus) getPropertyInfo: (UInt32) channel
		  propertyID: (AudioDevicePropertyID) propertyID
		     outSize: (UInt32 *) outSize
	       outIsWritable: (BOOL *) outIsWritable;

- (OSStatus) getProperty: (UInt32) channel
	      propertyID: (AudioDevicePropertyID) propertyID
	      ioDataSize: (UInt32 *) ioDataSize
		    data: (void *) data;

- (OSStatus) setProperty: (const AudioTimeStamp *) when
		 channel: (UInt32) channel
	      propertyID: (AudioDevicePropertyID) propertyID
		dataSize: (UInt32) dataSize
		    data: (const void *) data;

@end
