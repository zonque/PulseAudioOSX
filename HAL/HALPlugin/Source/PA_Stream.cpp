/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#define CLASS_NAME "PA_Stream"

#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/AudioHardware.h>
#include <sys/param.h>

#include "PA_Stream.h"

#define super PA_Object

PA_Object *
PA_Stream::FindObjectByID(AudioObjectID searchID)
{
	if (GetObjectID() == searchID)
		return this;
	
	return NULL;
}

#pragma mark ### Properties (legacy interface) ###

OSStatus
PA_Stream::GetPropertyInfo(UInt32 inChannel,
			   AudioDevicePropertyID inPropertyID,
			   UInt32 *outSize,
			   Boolean *outWritable)
{
	AudioObjectPropertyAddress addr;
	OSStatus ret = kAudioHardwareNoError;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = 0;
	
	if (!HasProperty(&addr))
		return kAudioHardwareUnknownPropertyError;
	
	if (outWritable)
		ret = IsPropertySettable(&addr, outWritable);
	
	if (ret != kAudioHardwareNoError)
		return ret;
	
	if (outSize)
		ret = GetPropertyDataSize(&addr, 0, NULL, outSize);
	
	return ret;
}

OSStatus
PA_Stream::GetProperty(UInt32 inChannel,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 *ioPropertyDataSize,
		       void *outPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = 0;

	if (!HasProperty(&addr))
		return kAudioHardwareUnknownPropertyError;

	return GetPropertyData(&addr, 0, NULL, ioPropertyDataSize, outPropertyData);
}

OSStatus
PA_Stream::SetProperty(const AudioTimeStamp * /* inWhen */,
		       UInt32 inChannel,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 inPropertyDataSize,
		       const void *inPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = 0;

	if (!HasProperty(&addr))
		return kAudioHardwareUnknownPropertyError;

	return SetPropertyData(&addr, 0, NULL, inPropertyDataSize, inPropertyData);
}

#pragma mark ### properties ###

Boolean
PA_Stream::HasProperty(const AudioObjectPropertyAddress *inAddress)
{
	switch (inAddress->mSelector) {
		case kAudioStreamPropertyDirection:
		case kAudioStreamPropertyTerminalType:
		case kAudioStreamPropertyStartingChannel:
		case kAudioStreamPropertyLatency:
		case kAudioStreamPropertyVirtualFormat:
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioStreamPropertyAvailablePhysicalFormats:			
		case kAudioDevicePropertySupportsMixing:
			return true;
	}

	return super::HasProperty(inAddress);
}

OSStatus
PA_Stream::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	// none of them is settable
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_Stream::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 inQualifierDataSize,
			       const void *inQualifierData,
			       UInt32 *outDataSize)
{
	switch (inAddress->mSelector) {
		case kAudioStreamPropertyDirection:
		case kAudioStreamPropertyTerminalType:
		case kAudioStreamPropertyStartingChannel:
		case kAudioStreamPropertyLatency:
		case kAudioDevicePropertySupportsMixing:
			*outDataSize = sizeof(UInt32);
			return kAudioHardwareNoError;

		case kAudioStreamPropertyVirtualFormat:
		case kAudioStreamPropertyPhysicalFormat:
			*outDataSize = sizeof(AudioStreamBasicDescription);
			return kAudioHardwareNoError;

		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
			*outDataSize = sizeof(AudioStreamRangedDescription);
			return kAudioHardwareNoError;			
	}
	
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_Stream::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 *ioDataSize,
			   void *outData)
{
	switch (inAddress->mSelector) {
		case kAudioStreamPropertyDirection:
			*static_cast<UInt32*>(outData) = isInput;
			return kAudioHardwareNoError;

		case kAudioStreamPropertyTerminalType:
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;

		case kAudioStreamPropertyStartingChannel:
			*static_cast<UInt32*>(outData) = startingChannel;
			return kAudioHardwareNoError;

		case kAudioStreamPropertyLatency:
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;
		
		case kAudioStreamPropertyVirtualFormat:
		case kAudioStreamPropertyPhysicalFormat:
			*ioDataSize = MIN(*ioDataSize, sizeof(AudioStreamBasicDescription));
			memcpy(outData, &device->streamDescription, *ioDataSize);
			return kAudioHardwareNoError;

		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
			*ioDataSize = MIN(*ioDataSize, sizeof(AudioStreamRangedDescription));
			memcpy(outData, &device->physicalFormat, *ioDataSize);
			return kAudioHardwareNoError;
			
		// "always true"
		case kAudioDevicePropertySupportsMixing:
			*static_cast<UInt32*>(outData) = 1;
			return kAudioHardwareNoError;
	}
	
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Stream::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
#if 0
	bool newIsMixable;
	AudioStreamBasicDescription newFormat;
	const AudioStreamBasicDescription *formatDataPtr = static_cast<const AudioStreamBasicDescription*>(inData);

	switch (inAddress->mSelector) {
		case kAudioDevicePropertySupportsMixing:
		case kAudioStreamPropertyVirtualFormat:
		case kAudioStreamPropertyPhysicalFormat:
			break;
	}
#endif
	
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### Construct/Deconstruct

void
PA_Stream::Initialize()
{
	OSStatus ret;
	AudioObjectID oid;
	
	ret = AudioObjectCreate(plugin->GetInterface(),
				device->GetObjectID(),
				kAudioStreamClassID, &oid);
	if (ret != kAudioHardwareNoError) {
		DebugLog("AudioObjectCreate() failed with %d", (int) ret);
		return;
	}
		       
	SetObjectID(oid);
	DebugLog("New stream has ID %d", (int) oid);
}

void
PA_Stream::Teardown()
{
}

PA_Stream::PA_Stream(PA_Plugin *inPlugIn,
		     PA_Device *inOwningDevice,
		     bool inIsInput,
		     UInt32 inStartingDeviceChannelNumber) :
	plugin(inPlugIn),
	device(inOwningDevice),
	isInput(inIsInput),
	startingChannel(inStartingDeviceChannelNumber)
{
}

PA_Stream::~PA_Stream()
{
}
