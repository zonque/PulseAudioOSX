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

#define CLASS_NAME "PA_Object"

#include "PA_Object.h"

AudioObjectID
PA_Object::GetObjectID()
{
	return objectID;
}

void
PA_Object::SetObjectID(AudioObjectID i)
{
	objectID = i;
}

Boolean
PA_Object::HasProperty(const AudioObjectPropertyAddress *inAddress)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyListenerAdded:
		case kAudioObjectPropertyListenerRemoved:
			return true;
	}

	return false;
}

OSStatus
PA_Object::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyListenerAdded:
		case kAudioObjectPropertyListenerRemoved:
			*outIsSettable = true;
			return kAudioHardwareNoError;
	}

	*outIsSettable = false;
	return kAudioHardwareNoError;
}

OSStatus
PA_Object::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 /* inQualifierDataSize */,
			       const void * /* inQualifierData */,
			       UInt32 *outDataSize)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyListenerAdded:
		case kAudioObjectPropertyListenerRemoved:
			*outDataSize = sizeof(AudioObjectPropertyAddress);
			return kAudioHardwareNoError;
	}

	*outDataSize = 0;
	
	DebugLog("Unhandled property for id %d: '%c%c%c%c'",
		 (int) GetObjectID(),
		 ((int) inAddress->mSelector >> 24) & 0xff,
		 ((int) inAddress->mSelector >> 16) & 0xff,
		 ((int) inAddress->mSelector >> 8) & 0xff,
		 ((int) inAddress->mSelector >> 0) & 0xff);
	
	return kAudioHardwareUnknownPropertyError;
}

OSStatus
PA_Object::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 /* inQualifierDataSize */,
			   const void * /* inQualifierData */,
			   UInt32 *ioDataSize,
			   void *outData)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyListenerAdded:
		case kAudioObjectPropertyListenerRemoved:
			//ASSERT
			memset(outData, 0, *ioDataSize);
			return kAudioHardwareNoError;
	}

	DebugLog("Unhandled property for id %d: '%c%c%c%c'",
		 (int) GetObjectID(),
		 ((int) inAddress->mSelector >> 24) & 0xff,
		 ((int) inAddress->mSelector >> 16) & 0xff,
		 ((int) inAddress->mSelector >> 8) & 0xff,
		 ((int) inAddress->mSelector >> 0) & 0xff);
	
	return kAudioHardwareUnknownPropertyError;
}

OSStatus
PA_Object::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 /* inQualifierDataSize */,
			   const void * /* inQualifierData */,
			   UInt32 /* inDataSize */,
			   const void * /* inData */)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyListenerAdded:
		case kAudioObjectPropertyListenerRemoved:
			return kAudioHardwareNoError;
	}

	DebugLog("Unhandled property for id %d: '%c%c%c%c'",
		 (int) GetObjectID(),
		 ((int) inAddress->mSelector >> 24) & 0xff,
		 ((int) inAddress->mSelector >> 16) & 0xff,
		 ((int) inAddress->mSelector >> 8) & 0xff,
		 ((int) inAddress->mSelector >> 0) & 0xff);
	
	return kAudioHardwareUnknownPropertyError;
}

void
PA_Object::Show()
{
	// implement me
}

void
PA_Object::Lock()
{
	mutex->Lock();
}

void
PA_Object::Unlock()
{
	mutex->Unlock();
}

PA_Object::PA_Object()
{
	mutex = new CAMutex("PA_Object");
}

PA_Object::~PA_Object()
{
	delete mutex;
}
