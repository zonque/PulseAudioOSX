#define CLASS_NAME "PA_MuteControl"

#include "PA_MuteControl.h"

#define super PA_Object

PA_MuteControl::PA_MuteControl(PA_Plugin *inPlugin,
			       PA_Stream *inOwningStream) :
	plugin(inPlugin),
	stream(inOwningStream)
{
}

PA_MuteControl::~PA_MuteControl()
{
}

const char *
PA_MuteControl::ClassName()
{
	return CLASS_NAME;
}

void
PA_MuteControl::Initialize()
{
	AudioObjectID oid;

	AudioObjectCreate(plugin->GetInterface(),
			  stream->GetObjectID(),
			  kAudioMuteControlClassID, &oid);	
	SetObjectID(oid);
}

void
PA_MuteControl::Teardown()
{	
}

PA_Object *
PA_MuteControl::FindObjectByID(AudioObjectID searchID)
{
	if (searchID == GetObjectID())
		return this;
	
	return NULL;
}

#pragma mark ### properties ###

Boolean
PA_MuteControl::HasProperty(const AudioObjectPropertyAddress *inAddress)
{
	return super::HasProperty(inAddress);
}

OSStatus
PA_MuteControl::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
				   Boolean *outIsSettable)
{
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_MuteControl::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
				    UInt32 inQualifierDataSize,
				    const void *inQualifierData,
				    UInt32 *outDataSize)
{
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_MuteControl::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
				UInt32 inQualifierDataSize,
				const void *inQualifierData,
				UInt32 *ioDataSize,
				void *outData)
{
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_MuteControl::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
				UInt32 inQualifierDataSize,
				const void *inQualifierData,
				UInt32 inDataSize,
				const void *inData)
{
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}
