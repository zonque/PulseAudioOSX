#include "PA_Object.h"

Boolean
PA_Object::HasProperty(const AudioObjectPropertyAddress *inAddress)
{
	return false;
}

OSStatus
PA_Object::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Object::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 inQualifierDataSize,
			       const void *inQualifierData,
			       UInt32 *outDataSize)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Object::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 *ioDataSize,
			   void *outData)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Object::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
	return kAudioHardwareNoError;
}

void
PA_Object::Show()
{
	// implement me
}

PA_Object::PA_Object()
{
	properties = CFArrayCreateMutable(NULL, 0, kCFTypeArrayCallBacks);
}

PA_Object::~PA_Object()
{
	CFRelease(properties);
}
