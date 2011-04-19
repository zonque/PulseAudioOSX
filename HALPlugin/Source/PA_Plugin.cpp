/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#define CLASS_NAME "PA_Plugin"

#include "PA_Plugin.h"
#include "PA_Device.h"
#include "PA_Stream.h"

#include <pulse/pulseaudio.h>

#define TraceCall(x) printf("%s::%s() :%d\n", CLASS_NAME, __func__, __LINE__);

#if 0
#define DebugProperty(x...) DebugLog(x)
#else
#define DebugProperty(x...) do {} while(0)
#endif

// All methods of this class are just wrappers to call our child classes

#pragma mark ### internal Operations ###

PA_Plugin::PA_Plugin(CFAllocatorRef inAllocator,
		     AudioHardwarePlugInRef inPlugin) :
	allocator(inAllocator),
	plugin(inPlugin)
{
	if (allocator)
		CFRetain(allocator);
}

PA_Plugin::~PA_Plugin()
{
	if (allocator)
		CFRelease(allocator);	
}

void
PA_Plugin::ReportOwnedObjects(std::vector<AudioObjectID> &arr)
{
	for (SInt32 i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		arr.push_back(dev->GetObjectID());
	}
}

const char *
PA_Plugin::ClassName() {
	return CLASS_NAME;
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
PA_Plugin::FindObjectByID(AudioObjectID searchID)
{
	SInt32 i;
	
	if (GetObjectID() == searchID)
		return this;
	
	PA_Object *o = NULL;
	
	for (i = 0; i < CFArrayGetCount(devices); i++) {
		PA_Device *dev = (PA_Device *) CFArrayGetValueAtIndex(devices, i);
		o = dev->FindObjectByID(searchID);
		
		if (o)
			break;
	}
	
	return o;
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
	DebugLog("inObjectID = %d\n", (int) inObjectID);
	SetObjectID(inObjectID);

	devices = CFArrayCreateMutable(allocator, 0, NULL);
	PA_Device *dev = new PA_Device(this);
	CFArrayAppendValue(devices, dev);
	dev->Initialize();

	std::vector<AudioStreamID> deviceIDs;
	deviceIDs.clear();
	ReportOwnedObjects(deviceIDs);
		
	// publish device objects
	if (deviceIDs.size() != 0)
		AudioObjectsPublishedAndDied(GetInterface(),
					     kAudioObjectSystemObject,
					     deviceIDs.size(), &(deviceIDs.front()),
					     0, NULL);
	
	dev->PublishObjects(true);

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
	PA_Object *o = FindObjectByID(inObjectID);
	
	if (o)
		o->Show();
	else
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
}

Boolean
PA_Plugin::ObjectHasProperty(AudioObjectID inObjectID,
			     const AudioObjectPropertyAddress *inAddress)
{
	PA_Object *o = FindObjectByID(inObjectID);
	Boolean ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
		return false;
	}

	o->Lock();
	ret = o->HasProperty(inAddress);
	o->Unlock();
	
	if (!ret) {
		DebugProperty("id %d (%s) has NO property '%c%c%c%c'",
			      (int) inObjectID, o->ClassName(),
			      ((int) inAddress->mSelector >> 24) & 0xff,
			      ((int) inAddress->mSelector >> 16) & 0xff,
			      ((int) inAddress->mSelector >> 8)  & 0xff,
			      ((int) inAddress->mSelector >> 0)  & 0xff);
	}	

	return ret;
}

OSStatus
PA_Plugin::ObjectIsPropertySettable(AudioObjectID inObjectID,
				    const AudioObjectPropertyAddress *inAddress,
				    Boolean *outIsSettable)
{
	PA_Object *o = FindObjectByID(inObjectID);
	OSStatus ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
		return kAudioHardwareBadObjectError;
	}

	o->Lock();
	ret = o->IsPropertySettable(inAddress, outIsSettable);
	o->Unlock();

	DebugProperty("asked id %d (%s) for '%c%c%c%c', -> %s",
		      (int) inObjectID, o->ClassName(),
		      ((int) inAddress->mSelector >> 24) & 0xff,
		      ((int) inAddress->mSelector >> 16) & 0xff,
		      ((int) inAddress->mSelector >> 8)  & 0xff,
		      ((int) inAddress->mSelector >> 0)  & 0xff,
		      *outIsSettable ? "YES" : "NO");

	return ret;
}

OSStatus
PA_Plugin::ObjectGetPropertyDataSize(AudioObjectID inObjectID,
				     const AudioObjectPropertyAddress *inAddress,
				     UInt32 inQualifierDataSize,
				     const void *inQualifierData,
				     UInt32 *outDataSize)
{
	PA_Object *o = FindObjectByID(inObjectID);
	OSStatus ret;

	if (!o) {
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
		return kAudioHardwareBadObjectError;
	}
	
	o->Lock();
	ret = o->GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
	o->Unlock();

	DebugProperty("asked id %d (%s) for '%c%c%c%c' -> %d",
		      (int) inObjectID,  o->ClassName(),
		      ((int) inAddress->mSelector >> 24) & 0xff,
		      ((int) inAddress->mSelector >> 16) & 0xff,
		      ((int) inAddress->mSelector >> 8)  & 0xff,
		      ((int) inAddress->mSelector >> 0)  & 0xff,
		      outDataSize ? (int) *outDataSize : 0);
		
	if (*outDataSize == 0)
		DebugLog("!!!!!!!!!!! outDataSize == 0");
	
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
	PA_Object *o = FindObjectByID(inObjectID);
	OSStatus ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
		return kAudioHardwareBadObjectError;
	}

	DebugProperty("asked id %d (%s) for '%c%c%c%c'",
		      (int) inObjectID,  o->ClassName(),
		      ((int) inAddress->mSelector >> 24) & 0xff,
		      ((int) inAddress->mSelector >> 16) & 0xff,
		      ((int) inAddress->mSelector >> 8)  & 0xff,
		      ((int) inAddress->mSelector >> 0)  & 0xff);
		
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
	PA_Object *o = FindObjectByID(inObjectID);
	OSStatus ret;
	
	if (!o) {
		DebugLog("Illegal inObjectID %d", (int) inObjectID);
		return kAudioHardwareBadObjectError;
	}
	
	DebugProperty("asked id %d (%s) for '%c%c%c%c'",
		      (int) inObjectID, o->ClassName(),
		      ((int) inAddress->mSelector >> 24) & 0xff,
		      ((int) inAddress->mSelector >> 16) & 0xff,
		      ((int) inAddress->mSelector >> 8)  & 0xff,
		      ((int) inAddress->mSelector >> 0)  & 0xff);
		
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->RemoveIOProc(inProc);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceStart(AudioDeviceID inDeviceID,
		       AudioDeviceIOProcID inProcID)
{
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->Start(inProcID);
	device->Unlock();

	return ret;
}

OSStatus
PA_Plugin::DeviceStartAtTime(AudioDeviceID inDeviceID,
			     AudioDeviceIOProcID inProcID,
			     AudioTimeStamp *ioRequestedStartTime,
			     UInt32 inFlags)
{
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
		return kAudioHardwareBadDeviceError;
	}

	device->Lock();
	ret = device->StartAtTime(inProcID, ioRequestedStartTime, inFlags);
	device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceStop(AudioDeviceID inDeviceID,
		      AudioDeviceIOProcID inProcID)
{
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
		return kAudioHardwareBadDeviceError;
	}
	
	//device->Lock();
	ret = device->Stop(inProcID);
	//device->Unlock();
	
	return ret;
}

OSStatus
PA_Plugin::DeviceRead(AudioDeviceID inDeviceID,
		      const AudioTimeStamp *inStartTime,
		      AudioBufferList *outData)
{
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;
	
	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Device *device = GetDeviceById(inDeviceID);
	OSStatus ret;

	if (!device) {
		DebugLog("Illegal inDeviceID %d", (int) inDeviceID);
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
	TraceCall();
	PA_Stream *stream = GetStreamById(inStreamID);
	
	if (!stream) {
		DebugLog("Illegal inStreamID %d", (int) inStreamID);
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
	TraceCall();
	PA_Stream *stream = GetStreamById(inStreamID);
	OSStatus ret;

	if (!stream) {
		DebugLog("Illegal inStreamID %d", (int) inStreamID);
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
	TraceCall();
	PA_Stream *stream = GetStreamById(inStreamID);
	OSStatus ret;

	if (!stream) {
		DebugLog("Illegal inStreamID %d", (int) inStreamID);
		return kAudioHardwareBadStreamError;
	}
	
	stream->Lock();
	ret = stream->SetProperty(inWhen, inChannel, inPropertyID, inPropertyDataSize, inPropertyData);
	stream->Unlock();
	
	return ret;
}
