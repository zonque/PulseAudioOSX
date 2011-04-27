/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
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
