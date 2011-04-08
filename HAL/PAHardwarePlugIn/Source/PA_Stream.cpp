#include "PA_Stream.h"


PA_Object *
PA_Stream::findObjectById(AudioObjectID searchID)
{
	if (GetObjectID() == searchID)
		return this;
	
	return NULL;
}

OSStatus
PA_Stream::GetPropertyInfo(UInt32 inChannel,
			 AudioDevicePropertyID inPropertyID,
			 UInt32 *outSize,
			 Boolean *outWritable)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Stream::GetProperty(UInt32 inChannel,
		     AudioDevicePropertyID inPropertyID,
		     UInt32 *ioPropertyDataSize,
		     void *outPropertyData)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Stream::SetProperty(const AudioTimeStamp *inWhen,
		     UInt32 inChannel,
		     AudioDevicePropertyID inPropertyID,
		     UInt32 inPropertyDataSize,
		     const void *inPropertyData)
{
	return kAudioHardwareNoError;
}

PA_Stream::PA_Stream()
{

}

PA_Stream::~PA_Stream()
{

}
