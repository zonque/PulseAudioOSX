#include "PA_Stream.h"

#define super PA_Object

PA_Object *
PA_Stream::FindObjectByID(AudioObjectID searchID)
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
	switch (inAddress->mSelector) {
		case kAudioStreamPropertyDirection:
		case kAudioStreamPropertyTerminalType:
		case kAudioStreamPropertyStartingChannel:
		case kAudioStreamPropertyLatency:
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
			return sizeof(UInt32);
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
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Stream::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
	bool newIsMixable;
	AudioStreamBasicDescription newFormat;
	const AudioStreamBasicDescription *formatDataPtr = static_cast<const AudioStreamBasicDescription*>(inData);

	switch (inAddress->mSelector) {
		case kAudioDevicePropertySupportsMixing:
		case kAudioStreamPropertyVirtualFormat:
		case kAudioStreamPropertyPhysicalFormat:
			break;
	}
	
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### Construct/Deconstruct

void
PA_Stream::Initialize()
{
	OSStatus ret;
	AudioObjectID oid;
	
	ret = AudioObjectCreate(plugin, device->GetObjectID(), kAudioStreamClassID, &oid);
	if (ret != kAudioHardwareNoError) {
		printf("AudioObjectCreate() failed with %d\n", ret);
		return;
	}
		       
	SetObjectID(oid);
	
#if 0
	AudioStreamRangedDescription physicalFormat;
	
	// the first is 16 bit stereo
	physicalFormat.mFormat.mSampleRate = 44100;
	physicalFormat.mSampleRateRange.mMinimum = 44100;
	physicalFormat.mSampleRateRange.mMaximum = 44100;
	physicalFormat.mFormat.mFormatID = kAudioFormatLinearPCM;
	physicalFormat.mFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger	|
					      kAudioFormatFlagsNativeEndian		|
					      kAudioFormatFlagIsPacked;
	physicalFormat.mFormat.mBytesPerPacket = 4;
	physicalFormat.mFormat.mFramesPerPacket = 1;
	physicalFormat.mFormat.mBytesPerFrame = 4;
	physicalFormat.mFormat.mChannelsPerFrame = 2;
	physicalFormat.mFormat.mBitsPerChannel = 16;
	mFormatList->AddPhysicalFormat(physicalFormat);

	AudioStreamBasicDescription physicalFormat;
	physicalFormat.mSampleRate = 44100;
	physicalFormat.mFormatID = kAudioFormatLinearPCM;
	physicalFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat		|
				      kAudioFormatFlagsNativeEndian		|
				      kAudioFormatFlagIsPacked;
	physicalFormat.mFramesPerPacket = 1;
	physicalFormat.mChannelsPerFrame = 2;
	physicalFormat.mBitsPerChannel = 32;
	physicalFormat.mBytesPerPacket = (physicalFormat.mBitsPerChannel *
					  physicalFormat.mChannelsPerFrame) / 8;
	physicalFormat.mBytesPerFrame = physicalFormat.mBytesPerPacket *
					physicalFormat.mFramesPerPacket;
	mFormatList->SetCurrentPhysicalFormat(physicalFormat, false);
#endif

}

PA_Stream::PA_Stream(AudioHardwarePlugInRef inPlugIn,
		     PA_Device *inOwningDevice,
		     bool inIsInput,
		     UInt32 inStartingDeviceChannelNumber) :
	plugin(inPlugIn),
	device(inOwningDevice)
{
}

PA_Stream::~PA_Stream()
{

}
