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

#import "PAStream.h"
#import "PADevice.h"

@implementation PAStream

- (id) initWithDevice: (PADevice *) _device
	      isInput: (BOOL) _isInput
      startingChannel: (UInt32) _startingChannel
{
	[super init];

	owningDevice = _device;
	isInput = isInput;
	startingChannel = _startingChannel;
	pluginRef = owningDevice.pluginRef;

	OSStatus ret = AudioObjectCreate(pluginRef,
					 owningDevice.objectID,
					 kAudioStreamClassID, &objectID);

	if (ret != kAudioHardwareNoError) {
		DebugLog("AudioObjectCreate() failed with %d", (int) ret);
		return nil;
	}
	
	return self;
}

#pragma mark ### PAObject ###

#pragma mark ### properties ###

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address
{
	switch (address->mSelector) {
		case kAudioStreamPropertyDirection:
		case kAudioStreamPropertyTerminalType:
		case kAudioStreamPropertyStartingChannel:
		case kAudioStreamPropertyLatency:
		case kAudioDevicePropertySupportsMixing:
			return YES;
			
			// handled by device
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
			return [owningDevice hasProperty: address];
	}

	return [super hasProperty: address];
}

- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address
{
	switch (address->mSelector) {
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioDevicePropertyStreamFormat:
			return [owningDevice isPropertySettable: address];
	}

	return [super isPropertySettable: address];
}

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
	       qualifierDataSize: (UInt32) qualifierDataSize
		   qualifierData: (const void *) qualifierData
		     outSize: (UInt32 *) outDataSize
{
	switch (address->mSelector) {
		case kAudioStreamPropertyDirection:
		case kAudioStreamPropertyTerminalType:
		case kAudioStreamPropertyStartingChannel:
		case kAudioStreamPropertyLatency:
		case kAudioDevicePropertySupportsMixing:
			*outDataSize = sizeof(UInt32);
			return kAudioHardwareNoError;
			
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
			return [owningDevice getPropertyDataSize: address
					       qualifierDataSize: qualifierDataSize
						   qualifierData: qualifierData
						     outSize: outDataSize];
	}

	return [super getPropertyDataSize: address
			qualifierDataSize: qualifierDataSize
			    qualifierData: qualifierData
			      outSize: outDataSize];
}

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		  ioDataSize: (UInt32 *) ioDataSize
		     outData: (void *) outData
{
	switch (address->mSelector) {
		case kAudioStreamPropertyDirection:
			*(UInt32 *) outData = isInput;
			return kAudioHardwareNoError;
			
		case kAudioStreamPropertyTerminalType:
			*(UInt32 *) outData = 0;
			return kAudioHardwareNoError;
			
		case kAudioStreamPropertyStartingChannel:
			*(UInt32 *) outData = startingChannel;
			return kAudioHardwareNoError;
			
		case kAudioStreamPropertyLatency:
			*(UInt32 *) outData = 0;
			return kAudioHardwareNoError;
			
		// "always true"
		case kAudioDevicePropertySupportsMixing:
			*(UInt32 *) outData = 1;
			return kAudioHardwareNoError;
			
		// pass to device
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
			return [owningDevice getPropertyData: address
					   qualifierDataSize: qualifierDataSize
					       qualifierData: qualifierData
						  ioDataSize: ioDataSize
						     outData: outData];
	}

	return [super getPropertyData: address
		    qualifierDataSize: qualifierDataSize
			qualifierData: qualifierData
			   ioDataSize: ioDataSize
			      outData: outData];
}

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
	   qualifierDataSize: (UInt32) qualifierDataSize
	       qualifierData: (const void *) qualifierData
		    dataSize: (UInt32) dataSize
			data: (const void *) data
{
	switch (address->mSelector) {
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
			return [owningDevice setPropertyData: address
					   qualifierDataSize: qualifierDataSize
					       qualifierData: qualifierData
						    dataSize: dataSize
							data: data];
	}

	return [super setPropertyData: address
		    qualifierDataSize: qualifierDataSize
			qualifierData: qualifierData
			     dataSize: dataSize
				 data: data];
}

#pragma mark ### properties (legacy interface) ###

- (OSStatus) getPropertyInfo: (UInt32) channel
		  propertyID: (AudioDevicePropertyID) propertyID
		     outSize: (UInt32 *) outSize
	       outIsWritable: (BOOL *) outIsWritable
{
	AudioObjectPropertyAddress addr;
	OSStatus ret = kAudioHardwareNoError;
	
	addr.mSelector = propertyID;
	addr.mElement = channel;
	addr.mScope = 0;
	
	if (![self hasProperty: &addr])
		return kAudioHardwareUnknownPropertyError;
	
	if (outIsWritable)
		*outIsWritable = [self isPropertySettable: &addr];
	
	if (outSize)
		ret = [self getPropertyDataSize: &addr
			      qualifierDataSize: 0
				  qualifierData: NULL
				    outSize: outSize];
	
	return ret;
}

- (OSStatus) getProperty: (UInt32) channel
	      propertyID: (AudioDevicePropertyID) propertyID
	      ioDataSize: (UInt32 *) ioDataSize
		    data: (void *) outData
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = propertyID;
	addr.mElement = channel;
	addr.mScope = 0;
	
	if (![self hasProperty: &addr])
		return kAudioHardwareUnknownPropertyError;
	
	return [self getPropertyData: &addr
		   qualifierDataSize: 0
		       qualifierData: NULL
			  ioDataSize: ioDataSize
			     outData: outData];
}

- (OSStatus) setProperty: (const AudioTimeStamp *) when
		 channel: (UInt32) channel
	      propertyID: (AudioDevicePropertyID) propertyID
		dataSize: (UInt32) dataSize
		    data: (const void *) data
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = propertyID;
	addr.mElement = channel;
	addr.mScope = 0;
	
	if (![self hasProperty: &addr])
		return kAudioHardwareUnknownPropertyError;
	
	return [self setPropertyData: &addr
		   qualifierDataSize: 0
		       qualifierData: NULL
			    dataSize: dataSize
				data: data];
}

@end
