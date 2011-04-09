#include "PA_Object.h"

/*
void
PA_Object::AddProperty(const AudioObjectPropertyAddress *inAddress)
{
	mutex->Lock();
	CFMutableDictionaryRef property =
		CFDictionaryCreateMutable(NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);

	CFArrayAppendValue(properties, property);
	mutex->Unlock();
}
*/

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

	return kAudioHardwareUnknownPropertyError;
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
