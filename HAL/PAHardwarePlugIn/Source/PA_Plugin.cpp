#include "PA_Plugin.h"
#include "PA_Device.h"
#include "PA_Stream.h"

#include <pulse/pulseaudio.h>

#define TraceCall(x) printf("PA_Plugin::%s() :%d\n", __func__, __LINE__);

// All methods of this class are just wrappers to call our child classes

#pragma mark ### internal Operations ###

PA_Plugin::PA_Plugin(AudioHardwarePlugInRef inPlugin) : plugin(inPlugin)
{
	
}

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
	return InitializeWithObjectID(kAudioObjectUnknown);
}

OSStatus
PA_Plugin::InitializeWithObjectID(AudioObjectID inObjectID)
{
	TraceCall();
	devices = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
	
	PA_Device *dev = new PA_Device(plugin);
	dev->Initialize();
	CFArrayAppendValue(devices, dev);
	
	SetObjectID(inObjectID);

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
	Boolean ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return false;
	}

	o->Lock();
	ret = o->HasProperty(inAddress);
	o->Unlock();

	return ret;
}

OSStatus
PA_Plugin::ObjectIsPropertySettable(AudioObjectID inObjectID,
				    const AudioObjectPropertyAddress *inAddress,
				    Boolean *outIsSettable)
{
	PA_Object *o = findObjectById(inObjectID);
	OSStatus ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}

	o->Lock();
	ret = o->IsPropertySettable(inAddress, outIsSettable);
	o->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::ObjectGetPropertyDataSize(AudioObjectID inObjectID,
				     const AudioObjectPropertyAddress *inAddress,
				     UInt32 inQualifierDataSize,
				     const void *inQualifierData,
				     UInt32 *outDataSize)
{
	PA_Object *o = findObjectById(inObjectID);
	OSStatus ret;

	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}
	
	o->Lock();
	ret = o->GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
	o->Unlock();
	
	return ret;
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
	OSStatus ret;

	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}
	
	o->Lock();
	ret = o->GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
	o->Unlock();
	
	return ret;
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
	OSStatus ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID");
		return kAudioHardwareBadObjectError;
	}

	o->Lock();
	ret = o->SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
	o->Unlock();

	return ret;
}

#pragma mark ### AudioDevice Operations ###

OSStatus
PA_Plugin::DeviceCreateIOProcID(AudioDeviceID inDeviceID,
				AudioDeviceIOProc inProc,
				void *inClientData,
				AudioDeviceIOProcID *outIOProcID)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->CreateIOProcID(inProc, inClientData, outIOProcID);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceDestroyIOProcID(AudioDeviceID inDeviceID,
				 AudioDeviceIOProcID inIOProcID)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->DestroyIOProcID(inIOProcID);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceAddIOProc(AudioDeviceID inDeviceID,
			   AudioDeviceIOProc inProc, 
			   void *inClientData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->AddIOProc(inProc, inClientData);
	device->Unlock();

	return ret;
}

OSStatus
PA_Plugin::DeviceRemoveIOProc(AudioDeviceID inDeviceID,
			      AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->RemoveIOProc(inProc);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceStart(AudioDeviceID inDeviceID,
		       AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->Start(inProc);
	device->Unlock();

	return ret;
}

OSStatus
PA_Plugin::DeviceStartAtTime(AudioDeviceID inDeviceID,
			     AudioDeviceIOProc inProc,
			     AudioTimeStamp *ioRequestedStartTime,
			     UInt32 inFlags)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->StartAtTime(inProc, ioRequestedStartTime, inFlags);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceStop(AudioDeviceID inDeviceID,
		      AudioDeviceIOProc inProc)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->Stop(inProc);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceRead(AudioDeviceID inDeviceID,
		      const AudioTimeStamp *inStartTime,
		      AudioBufferList *outData)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->Read(inStartTime, outData);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceGetCurrentTime(AudioDeviceID inDeviceID,
				AudioTimeStamp* outTime)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->GetCurrentTime(outTime);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceTranslateTime(AudioDeviceID inDeviceID,
			       const AudioTimeStamp *inTime,
			       AudioTimeStamp *outTime)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->TranslateTime(inTime, outTime);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceGetNearestStartTime(AudioDeviceID inDeviceID,
				     AudioTimeStamp* ioRequestedStartTime,
				     UInt32 inFlags)
{
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->GetNearestStartTime(ioRequestedStartTime, inFlags);
	device->Unlock();
	
	return ret;
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
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->GetPropertyInfo(inChannel, isInput, inPropertyID, outSize, outWritable);
	device->Unlock();
	
	return ret;
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
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->GetProperty(inChannel, isInput, inPropertyID, ioPropertyDataSize, outPropertyData);
	device->Unlock();
	
	return ret;
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
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID");
		return kAudioHardwareBadDeviceError;
	}
	
	device->Lock();
	ret = device->SetProperty(inWhen, inChannel, isInput, inPropertyID, inPropertyDataSize, inPropertyData);
	device->Unlock();

	return ret;
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
	OSStatus ret;

	if (!stream) {
		DebugLog("Illegal inStreamID");
		return kAudioHardwareBadStreamError;
	}

	stream->Lock();
	ret = stream->GetProperty(inChannel, inPropertyID, ioPropertyDataSize, outPropertyData);
	stream->Unlock();
	
	return ret;
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
	OSStatus ret;

	if (!stream) {
		DebugLog("Illegal inStreamID");
		return kAudioHardwareBadStreamError;
	}
	
	stream->Lock();
	ret = stream->SetProperty(inWhen, inChannel, inPropertyID, inPropertyDataSize, inPropertyData);
	stream->Unlock();
	
	return ret;
}
