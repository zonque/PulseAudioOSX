#include "PA_Plugin.h"
#include "PA_Device.h"
#include "PA_Stream.h"

#include <pulse/pulseaudio.h>

#define DebugLog(str) printf("%s():%d :: %s\n", __func__, __LINE__, str);

#pragma mark ### internal Operations ###

PA_Device *
PA_Plugin::GetDeviceById(AudioObjectID inObjectID)
{
	SInt32 i;
	
	for (i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		if (dev->GetObjectID() == inObjectID)
			return dev;
	}

	return NULL;
}

PA_Stream *
PA_Plugin::GetStreamById(AudioObjectID inObjectID)
{
	SInt32 i;
	PA_Stream *stream = NULL;
	
	for (i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		stream = dev->GetStreamById(inObjectID);
		
		if (stream)
			break;
	}
	
	return stream;
}

PA_Object *
PA_Plugin::findObjectById(AudioObjectID searchID)
{
	SInt32 i;
	
	for (i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		if (dev->GetObjectID() == searchID)
			return dev;
	}
	
	return NULL;	
}

#pragma mark ### PlugIn Operations ###

ULONG
PA_Plugin::AddRef()
{
	return kAudioHardwareNoError;
}

ULONG
PA_Plugin::Release()
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Plugin::Initialize()
{
	return kAudioHardwareNoError;
}

OSStatus
PA_Plugin::InitializeWithObjectID(AudioObjectID inObjectID)
{
	devices = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
	
	PA_Device *dev = new PA_Device();
	dev->Initialize();
	CFArrayAppendValue(devices, dev);

	return kAudioHardwareNoError;
}

OSStatus
PA_Plugin::Teardown()
{
	SInt32 i;
	
	for (i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		dev->Teardown();
		delete dev;
	}
	
	CFArrayRemoveAllValues(devices);
	CFRelease(devices);
	
	return kAudioHardwareNoError;
}

#pragma mark ### AudioObject Operations ###

void
PA_Plugin::ObjectShow(AudioObjectID inObjectID)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (o)
		o->Show();
	else
		DebugLog("Illegal inObjectID");
}

Boolean
PA_Plugin::ObjectHasProperty(AudioObjectID inObjectID,
			     const AudioObjectPropertyAddress *inAddress)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return false;
	}
	
	return o->HasProperty(inAddress);
}

OSStatus
PA_Plugin::ObjectIsPropertySettable(AudioObjectID inObjectID,
				    const AudioObjectPropertyAddress *inAddress,
				    Boolean *outIsSettable)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}

	return o->IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_Plugin::ObjectGetPropertyDataSize(AudioObjectID inObjectID,
				     const AudioObjectPropertyAddress *inAddress,
				     UInt32 inQualifierDataSize,
				     const void *inQualifierData,
				     UInt32 *outDataSize)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}
	
	return o->GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_Plugin::ObjectGetPropertyData(AudioObjectID inObjectID,
				 const AudioObjectPropertyAddress *inAddress,
				 UInt32 inQualifierDataSize,
				 const void *inQualifierData,
				 UInt32 *ioDataSize,
				 void *outData)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}
	
	return o->GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Plugin::ObjectSetPropertyData(AudioObjectID inObjectID,
				 const AudioObjectPropertyAddress *inAddress,
				 UInt32 inQualifierDataSize,
				 const void *inQualifierData,
				 UInt32 inDataSize,
				 const void *inData)
{
	PA_Object *o = findObjectById(inObjectID);
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}
	
	return o->SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### AudioDevice Operations ###

OSStatus
PA_Plugin::DeviceCreateIOProcID(AudioDeviceID inDeviceID,
				AudioDeviceIOProc inProc,
				void *inClientData,
				AudioDeviceIOProcID *outIOProcID)
{
	PA_Device *device = GetDeviceById(inDeviceID);

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	return device->CreateIOProcID(inProc, inClientData, outIOProcID);
}

OSStatus
PA_Plugin::DeviceDestroyIOProcID(AudioDeviceID inDeviceID,
				 AudioDeviceIOProcID inIOProcID)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->DestroyIOProcID(inIOProcID);
}

OSStatus
PA_Plugin::DeviceAddIOProc(AudioDeviceID inDeviceID,
			   AudioDeviceIOProc inProc, 
			   void *inClientData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	return device->AddIOProc(inProc, inClientData);
}

OSStatus
PA_Plugin::DeviceRemoveIOProc(AudioDeviceID inDeviceID,
			      AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	return device->RemoveIOProc(inProc);
}

OSStatus
PA_Plugin::DeviceStart(AudioDeviceID inDeviceID,
		       AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->Start(inProc);
}

OSStatus
PA_Plugin::DeviceStartAtTime(AudioDeviceID inDeviceID,
			     AudioDeviceIOProc inProc,
			     AudioTimeStamp *ioRequestedStartTime,
			     UInt32 inFlags)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	return device->StartAtTime(inProc, ioRequestedStartTime, inFlags);
}

OSStatus
PA_Plugin::DeviceStop(AudioDeviceID inDeviceID,
		      AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->Stop(inProc);
}

OSStatus
PA_Plugin::DeviceRead(AudioDeviceID inDeviceID,
		      const AudioTimeStamp *inStartTime,
		      AudioBufferList *outData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->Read(inStartTime, outData);
}

OSStatus
PA_Plugin::DeviceGetCurrentTime(AudioDeviceID inDeviceID,
				AudioTimeStamp* outTime)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->GetCurrentTime(outTime);
}

OSStatus
PA_Plugin::DeviceTranslateTime(AudioDeviceID inDeviceID,
			       const AudioTimeStamp *inTime,
			       AudioTimeStamp *outTime)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->TranslateTime(inTime, outTime);
}

OSStatus
PA_Plugin::DeviceGetNearestStartTime(AudioDeviceID inDeviceID,
				     AudioTimeStamp* ioRequestedStartTime,
				     UInt32 inFlags)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->GetNearestStartTime(ioRequestedStartTime, inFlags);
}

OSStatus
PA_Plugin::DeviceGetPropertyInfo(AudioDeviceID inDeviceID,
				 UInt32 inChannel,
				 Boolean isInput,
				 AudioDevicePropertyID
				 inPropertyID,
				 UInt32 *outSize,
				 Boolean *outWritable)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->GetPropertyInfo(inChannel, isInput, inPropertyID, outSize, outWritable);
}

OSStatus
PA_Plugin::DeviceGetProperty(AudioDeviceID inDeviceID,
			     UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32* ioPropertyDataSize,
			     void* outPropertyData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->GetProperty(inChannel, isInput, inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus
PA_Plugin::DeviceSetProperty(AudioDeviceID inDeviceID,
			     const AudioTimeStamp *inWhen,
			     UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 inPropertyDataSize,
			     const void *inPropertyData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	return device->SetProperty(inWhen, inChannel, isInput, inPropertyID, inPropertyDataSize, inPropertyData);
}

#pragma mark ### AudioStream Operations ###

OSStatus
PA_Plugin::StreamGetPropertyInfo(AudioStreamID inStreamID,
				 UInt32 inChannel,
				 AudioDevicePropertyID inPropertyID,
				 UInt32 *outSize,
				 Boolean *outWritable)
{
	PA_Stream *stream = GetStreamById(inStreamID);
	
	if (!stream) {
		DebugLog("Illegal inStreamID");
		return kAudioHardwareBadStreamError;
	}

	return stream->GetPropertyInfo(inChannel, inPropertyID, outSize, outWritable);
}

OSStatus
PA_Plugin::StreamGetProperty(AudioStreamID inStreamID,
			     UInt32 inChannel,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 *ioPropertyDataSize,
			     void *outPropertyData)
{
	PA_Stream *stream = GetStreamById(inStreamID);
	
	if (!stream) {
		DebugLog("Illegal inStreamID");
		return kAudioHardwareBadStreamError;
	}

	return stream->GetProperty(inChannel, inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus
PA_Plugin::StreamSetProperty(AudioStreamID inStreamID,
			     const AudioTimeStamp *inWhen,
			     UInt32 inChannel,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 inPropertyDataSize,
			     const void *inPropertyData)
{
	PA_Stream *stream = GetStreamById(inStreamID);
	
	if (!stream) {
		DebugLog("Illegal inStreamID");
		return kAudioHardwareBadStreamError;
	}
	
	return stream->SetProperty(inWhen, inChannel, inPropertyID, inPropertyDataSize, inPropertyData);
}
