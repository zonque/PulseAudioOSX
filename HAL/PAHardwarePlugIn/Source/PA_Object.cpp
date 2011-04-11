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
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
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
	properties = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	mutex = new CAMutex("PA_Object");
}

PA_Object::~PA_Object()
{
	CFRelease(properties);
	delete mutex;
}
