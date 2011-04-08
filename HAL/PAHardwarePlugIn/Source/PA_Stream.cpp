#include "PA_Stream.h"

#define super PA_Object

PA_Object *
PA_Stream::findObjectById(AudioObjectID searchID)
{
	printf("PA_Stream::%s() ... searching for %d, mine %d\n", __func__, searchID, GetObjectID());
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
	AudioObjectPropertyAddress addr;
	OSStatus ret;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = 0;
	
	ret = IsPropertySettable(&addr, outWritable);
	if (ret != kAudioHardwareNoError)
		return ret;
	
	return GetPropertyDataSize(&addr, 0, NULL, outSize);
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
	
	return SetPropertyData(&addr, 0, NULL, inPropertyDataSize, inPropertyData);
}

#pragma mark ### properties ###

Boolean
PA_Stream::HasProperty(const AudioObjectPropertyAddress *inAddress)
{	
	return super::HasProperty(inAddress);
}

OSStatus
PA_Stream::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_Stream::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 inQualifierDataSize,
			       const void *inQualifierData,
			       UInt32 *outDataSize)
{
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_Stream::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 *ioDataSize,
			   void *outData)
{
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Stream::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

void
PA_Stream::Initialize()
{
}

PA_Stream::PA_Stream(AudioStreamID inAudioStreamID,
		     AudioHardwarePlugInRef inPlugIn,
		     PA_Device *inOwningDevice,
		     bool inIsInput,
		     UInt32 inStartingDeviceChannelNumber)
{
	SetObjectID(inAudioStreamID);
	printf("%s() .. inAudioStreamID %d\n", __func__, inAudioStreamID);
}

PA_Stream::~PA_Stream()
{

}
