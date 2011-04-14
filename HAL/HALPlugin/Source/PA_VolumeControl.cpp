#define CLASS_NAME "PA_VolumeControl"

#include "PA_VolumeControl.h"

#define super PA_Object

PA_VolumeControl::PA_VolumeControl(PA_Plugin *inPlugin,
				   PA_Stream *inOwningStream) :
	plugin(inPlugin),
	stream(inOwningStream)
{
}

PA_VolumeControl::~PA_VolumeControl()
{
}

const char *
PA_VolumeControl::ClassName()
{
	return CLASS_NAME;
}

void
PA_VolumeControl::Initialize()
{	
}

void
PA_VolumeControl::Teardown()
{	
}

PA_Object *
PA_VolumeControl::FindObjectByID(AudioObjectID searchID)
{
	if (searchID == GetObjectID())
		return this;
	
	return NULL;
}

#pragma mark ### properties ###

Boolean
PA_VolumeControl::HasProperty(const AudioObjectPropertyAddress *inAddress)
{
	return super::HasProperty(inAddress);	
}

OSStatus
PA_VolumeControl::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
				     Boolean *outIsSettable)
{
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_VolumeControl::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
				      UInt32 inQualifierDataSize,
				      const void *inQualifierData,
				      UInt32 *outDataSize)
{
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_VolumeControl::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
				  UInt32 inQualifierDataSize,
				  const void *inQualifierData,
				  UInt32 *ioDataSize,
				  void *outData)
{
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_VolumeControl::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
				  UInt32 inQualifierDataSize,
				  const void *inQualifierData,
				  UInt32 inDataSize,
				  const void *inData)
{
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}
