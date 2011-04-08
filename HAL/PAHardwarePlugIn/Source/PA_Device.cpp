#include "PA_Device.h"

PA_Device::PA_Device()
{
	
}

PA_Device::~PA_Device()
{
	
}

void
PA_Device::Initialize()
{
	
}

void
PA_Device::Teardown()
{

}

PA_Stream *
PA_Device::GetStreamById(AudioObjectID inObjectID)
{
	SInt32 i;
	
	for (i = 0; i < CFArrayGetCount(streams); i++) {
		PA_Stream *stream = (PA_Stream *) CFArrayGetValueAtIndex(streams, i);
		if (stream->GetObjectID() == inObjectID)
			return stream;
	}
	
	return NULL;
}

PA_Object *
PA_Device::findObjectById(AudioObjectID searchID)
{
	if (GetObjectID() == searchID)
		return this;

	SInt32 i;
	PA_Object *o = NULL;
	
	for (i = 0; i < CFArrayGetCount(streams); i++) {
		PA_Stream *stream = (PA_Stream *) CFArrayGetValueAtIndex(streams, i);
		o = stream->findObjectById(searchID);

		if (o)
			break;
	}
	
	return o;
}

OSStatus
PA_Device::CreateIOProcID(AudioDeviceIOProc inProc,
			  void *inClientData,
			  AudioDeviceIOProcID *outIOProcID)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::DestroyIOProcID(AudioDeviceIOProcID inIOProcID)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::AddIOProc(AudioDeviceIOProc inProc, 
		     void *inClientData)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::RemoveIOProc(AudioDeviceIOProc inProc)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::Start(AudioDeviceIOProc inProc)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::StartAtTime(AudioDeviceIOProc inProc,
		       AudioTimeStamp *ioRequestedStartTime,
		       UInt32 inFlags)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::Stop(AudioDeviceIOProc inProc)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::Read(const AudioTimeStamp *inStartTime,
		AudioBufferList *outData)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetCurrentTime(AudioTimeStamp* outTime)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::TranslateTime(const AudioTimeStamp *inTime,
			 AudioTimeStamp *outTime)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetNearestStartTime(AudioTimeStamp *ioRequestedStartTime,
			       UInt32 inFlags)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetPropertyInfo(UInt32 inChannel,
			   Boolean isInput,
			   AudioDevicePropertyID
			   inPropertyID,
			   UInt32 *outSize,
			   Boolean *outWritable)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetProperty(UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32* ioPropertyDataSize,
		       void* outPropertyData)
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::SetProperty(const AudioTimeStamp *inWhen,
		       UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 inPropertyDataSize,
		       const void *inPropertyData)
{
	return kAudioHardwareNoError;
}

