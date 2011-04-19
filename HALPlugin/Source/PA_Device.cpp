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

#define CLASS_NAME "PA_Device"

//#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>

#include "PA_Device.h"
#include "PA_Stream.h"
#include "PA_DeviceBackend.h"
#include "PA_DeviceControl.h"
#include "CAAudioBufferList.h"
#include "CAHostTimeBase.h"

#define super PA_Object
#define TraceCall(x) printf("PA_Device::%s() :%d\n", __func__, __LINE__);

#if 1
#define DebugIOProc(x...) DebugLog(x)
#else
#define DebugIOProc(x...) do {} while(0)
#endif

enum {
	ASYC_MSG_DEVICE_STOP = 0
};

#pragma mark ### Construct / Deconstruct ###

PA_Device::PA_Device(PA_Plugin *inPlugin) : plugin(inPlugin)
{
}

PA_Device::~PA_Device()
{
}

const char *
PA_Device::ClassName() {
	return CLASS_NAME;
}

void
PA_Device::ReportOwnedObjects(std::vector<AudioObjectID> &arr)
{
	for (UInt32 i = 0; i < nInputStreams; i++)
		if (inputStreams[i])
			arr.push_back(inputStreams[i]->GetObjectID());
	
	for (UInt32 i = 0; i < nOutputStreams; i++)
		if (outputStreams[i])
			arr.push_back(outputStreams[i]->GetObjectID());
}

OSStatus
PA_Device::PublishObjects(Boolean active)
{
	std::vector<AudioStreamID> streamIDs;
	streamIDs.clear();
	ReportOwnedObjects(streamIDs);

	if (streamIDs.size() != 0)
		if (active) {
			return AudioObjectsPublishedAndDied(plugin->GetInterface(),
							    GetObjectID(),
							    streamIDs.size(),
							    &(streamIDs.front()),
							    0, NULL);	
		} else {
			return AudioObjectsPublishedAndDied(plugin->GetInterface(),
							    GetObjectID(),
							    streamIDs.size(),
							    0, NULL,
							    &(streamIDs.front()));
		}

	for (UInt32 i = 0; i < nInputStreams; i++)
		inputStreams[i]->PublishObjects(active);

	for (UInt32 i = 0; i < nOutputStreams; i++)
		outputStreams[i]->PublishObjects(active);

	return kAudioHardwareNoError;
}

void
PA_Device::Initialize()
{
	UInt32 i;
	
	TraceCall();
	
	bufferFrameSize = 1024;
	sampleRate = 44100.0f;
	isRunning = false;

	nInputStreams = 1;
	nOutputStreams = 1;
		
	ioProcList = CFArrayCreateMutable(plugin->GetAllocator(), 0, NULL);
	pthread_mutex_init(&ioProcListMutex, NULL);
	
	deviceName = CFStringCreateWithCString(plugin->GetAllocator(), "PulseAudio", kCFStringEncodingASCII);
	deviceManufacturer = CFStringCreateWithCString(plugin->GetAllocator(), "pulseaudio.org", kCFStringEncodingASCII);
	modelUID = CFStringCreateWithFormat(plugin->GetAllocator(), NULL,
					    CFSTR("%@:%d,%d"), deviceName, nInputStreams * 2, nOutputStreams * 2);	
	deviceUID = CFStringCreateWithFormat(plugin->GetAllocator(), NULL,
					     CFSTR("org.pulseaudio.HALPlugin.%@"), modelUID);

	inputStreams = pa_xnew0(PA_Stream *, nInputStreams);
	outputStreams = pa_xnew0(PA_Stream *, nOutputStreams);

	deviceBackend = new PA_DeviceBackend(this);
	deviceBackend->Initialize();
	
	deviceControl = new PA_DeviceControl(this);
	deviceControl->Initialize();

	memset(&streamDescription, 0, sizeof(streamDescription));
	memset(&physicalFormat, 0, sizeof(physicalFormat));
	
	streamDescription.mSampleRate = sampleRate;
	streamDescription.mFormatID = kAudioFormatLinearPCM;
	streamDescription.mFormatFlags = kAudioFormatFlagsCanonical;
	streamDescription.mBitsPerChannel = 32;
	streamDescription.mChannelsPerFrame = 2;
	streamDescription.mFramesPerPacket = 1;
	streamDescription.mBytesPerFrame = 8;
	streamDescription.mBytesPerPacket = 8;

	physicalFormat.mSampleRateRange.mMinimum = sampleRate;
	physicalFormat.mSampleRateRange.mMaximum = sampleRate;
	memcpy(&physicalFormat.mFormat, &streamDescription, sizeof(streamDescription));

	AudioDeviceID newID;
	OSStatus ret = AudioObjectCreate(plugin->GetInterface(),
					 kAudioObjectSystemObject,
					 kAudioDeviceClassID,
					 &newID);
	
	if (ret == 0) {
		SetObjectID(newID);
		DebugLog("New device has ID %d", (int) newID);
	}
	
	for (i = 0; i < nInputStreams; i++) {
		inputStreams[i] = new PA_Stream(plugin, this, true, 1);
		inputStreams[i]->Initialize();
	}
	
	for (i = 0; i < nOutputStreams; i++) {
		outputStreams[0] = new PA_Stream(plugin, this, false, 1);
		outputStreams[0]->Initialize();
	}
	
	SetupAsyncCommands();
}

void
PA_Device::Teardown()
{
	TraceCall();
	
	if (deviceBackend) {
		deviceBackend->Teardown();
		delete deviceBackend;
		deviceBackend = NULL;
	}
	
	if (deviceControl) {
		deviceControl->Teardown();
		delete deviceControl;
		deviceControl = NULL;
	}

	if (deviceUID) {
		CFRelease(deviceUID);
		deviceUID = NULL;
	}
	
	if (modelUID) {
		CFRelease(modelUID);
		modelUID = NULL;
	}
	
	if (deviceName) {
		CFRelease(deviceName);
		deviceName = NULL;
	}
	
	if (deviceManufacturer) {
		CFRelease(deviceManufacturer);
		deviceManufacturer = NULL;
	}

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		pa_xfree(io);
	}
	
	if (ioProcList) {
		pthread_mutex_lock(&ioProcListMutex);
		CFRelease(ioProcList);
		ioProcList = NULL;
		pthread_mutex_unlock(&ioProcListMutex);
	}

	pthread_mutex_destroy(&ioProcListMutex);
	
	for (UInt32 i = 0; i < nInputStreams; i++)
		if (inputStreams[i]) {
			inputStreams[i]->Teardown();
			delete inputStreams[i];
			inputStreams[i] = NULL;
		}

	for (UInt32 i = 0; i < nOutputStreams; i++)
		if (outputStreams[i]) {
			outputStreams[i]->Teardown();
			delete outputStreams[i];
			outputStreams[i] = NULL;
		}

	pa_xfree(inputStreams);
	inputStreams = NULL;
	
	pa_xfree(outputStreams);
	outputStreams = NULL;
}

void
PA_Device::SetBufferSize(UInt32 size)
{
	bufferFrameSize = size;
}

CFAllocatorRef
PA_Device::GetAllocator()
{
	return plugin->GetAllocator();
}

void
PA_Device::AnnounceDeviceChange()
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = kAudioDevicePropertyDeviceHasChanged;
	addr.mElement = 0;
	addr.mScope = 0;

	AudioObjectPropertiesChanged(plugin->GetInterface(),
				     GetObjectID(),
				     1, &addr);
}

#pragma mark ### IOProcTracker / list management ###

IOProcTracker *
PA_Device::FindIOProc(AudioDeviceIOProc inProc)
{
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc)
			return io;
	}
	
	return NULL;
}

IOProcTracker *
PA_Device::FindIOProcByID(AudioDeviceIOProcID inProcID)
{
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inProcID)
			return io;
	}
	
	return NULL;
}

CFArrayRef
PA_Device::CopyIOProcList()
{
	pthread_mutex_lock(&ioProcListMutex);
	CFArrayRef list = CFArrayCreateCopy(GetAllocator(), ioProcList);
	pthread_mutex_unlock(&ioProcListMutex);

	return list;
}

void
PA_Device::EnableAllIOProcs(Boolean enabled)
{
	pthread_mutex_lock(&ioProcListMutex);
	
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		io->enabled = !!enabled;
	}
	
	pthread_mutex_unlock(&ioProcListMutex);
}

UInt32
PA_Device::CountEnabledIOProcs()
{
	UInt32 count = 0;
	
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->enabled)
			count++;
	}
	
	return count;
}

#pragma mark ### Plugin interface ###

PA_Stream *
PA_Device::GetStreamById(AudioObjectID inObjectID)
{
	for (UInt32 i = 0; i < nInputStreams; i++)
		if (inputStreams[i]->GetObjectID() == inObjectID)
			return inputStreams[i];

	for (UInt32 i = 0; i < nOutputStreams; i++)
		if (outputStreams[i]->GetObjectID() == inObjectID)
			return outputStreams[i];
	
	return NULL;
}

PA_Object *
PA_Device::FindObjectByID(AudioObjectID searchID)
{
	if (GetObjectID() == searchID)
		return this;

	PA_Object *o = NULL;

	for (UInt32 i = 0; i < nInputStreams; i++) {
		o = inputStreams[i]->FindObjectByID(searchID);

		if (o)
			return o;
	}

	for (UInt32 i = 0; i < nOutputStreams; i++) {
		o = outputStreams[i]->FindObjectByID(searchID);

		if (o)
			return o;
	}

	return NULL;
}

OSStatus
PA_Device::CreateIOProcID(AudioDeviceIOProc inProc,
			  void *inClientData,
			  AudioDeviceIOProcID *outIOProcID)
{
	DebugLog("inProc %p outIOProcID %p", inProc, outIOProcID);

	pthread_mutex_lock(&ioProcListMutex);

	if (FindIOProc(inProc)) {
		DebugLog("IOProc has already been added");
		pthread_mutex_unlock(&ioProcListMutex);
		return kAudioHardwareIllegalOperationError;
	}
	
	IOProcTracker *io = pa_xnew0(IOProcTracker, 1);
	io->proc = inProc;
	io->clientData = inClientData;
	
	if (outIOProcID)
		*outIOProcID = (AudioDeviceIOProcID) io;

	CFArrayAppendValue(ioProcList, io);
	pthread_mutex_unlock(&ioProcListMutex);
	
	DebugIOProc("New IOProc tracker @%p", io);
	
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::DestroyIOProcID(AudioDeviceIOProcID inIOProcID)
{
	pthread_mutex_lock(&ioProcListMutex);

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inIOProcID) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			pthread_mutex_unlock(&ioProcListMutex);
			DebugIOProc("Destroyed IOProc tracker @%p", io);
			return kAudioHardwareNoError;
		}
	}
	
	pthread_mutex_unlock(&ioProcListMutex);
	DebugLog("IOProc has not been added");

	return kAudioHardwareIllegalOperationError;
}

OSStatus
PA_Device::AddIOProc(AudioDeviceIOProc inProc, 
		     void *inClientData)
{
	return CreateIOProcID(inProc, inClientData, NULL);
}

OSStatus
PA_Device::RemoveIOProc(AudioDeviceIOProc inProc)
{
	pthread_mutex_lock(&ioProcListMutex);
	
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			pthread_mutex_unlock(&ioProcListMutex);
			DebugIOProc("Removed IOProc tracker @%p", io);
			return kAudioHardwareNoError;
		}
	}
	
	pthread_mutex_unlock(&ioProcListMutex);
	DebugLog("IOProc has not been added");
	return kAudioHardwareIllegalOperationError;
}

OSStatus
PA_Device::Start(AudioDeviceIOProcID inProcID)
{
	return StartAtTime(inProcID, NULL, 0);
}

OSStatus
PA_Device::StartAtTime(AudioDeviceIOProcID inProcID,
		       AudioTimeStamp *ioRequestedStartTime,
		       UInt32 inFlags)
{
	pthread_mutex_lock(&ioProcListMutex);
	IOProcTracker *io = FindIOProcByID(inProcID);
	
	if (!io)
		io = FindIOProc(inProcID);
	
	if (io) {
		io->enabled = true;
		io->startTimeFlags = inFlags;

		if (ioRequestedStartTime)
			memcpy(&io->startTime, ioRequestedStartTime, sizeof(*ioRequestedStartTime));

		DebugIOProc("Starting IOProc @%p", io);
	}

	pthread_mutex_unlock(&ioProcListMutex);

	if (inProcID && !io) {
		DebugLog("IOProc has not been added");
	}

	if (!isRunning) {
		isRunning = true;
		DebugLog("Starting hardware");
		deviceBackend->Connect();
		deviceControl->AnnounceDevice();
		AnnounceDeviceChange();
	}
	
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::Stop(AudioDeviceIOProc inProcID)
{
	unsigned long arg = (unsigned long) inProcID;

	// This plugin method is likely to be called from the IOProc thread, which
	// the pulseaudio mainloop API doesn't like. Hence, we have to trick around
	// and send a message to the thread the plugin was initialized from. Luckily,
	// CFMessagePorts make that very easy.

	CFDataRef data = CFDataCreate(GetAllocator(), (UInt8 *) &arg, sizeof(arg));
	SInt32 ret = SendAsyncMessage(ASYC_MSG_DEVICE_STOP, data);
	CFRelease(data);

	return ret;
}

#if 0
static OSStatus
oneShotIOProc(AudioDeviceID            /* inDevice */,
	      const AudioTimeStamp *   /* inNow */,
	      const AudioBufferList *  /* inInputData */,
	      const AudioTimeStamp *   /* inInputTime */,
	      AudioBufferList *        /* outOutputData */,
	      const AudioTimeStamp *   /* inOutputTime */,
	      void *                  inClientData)
{
	IOProcTracker *io = (IOProcTracker *) inClientData;
	
	io->enabled = false;
	MPSignalSemaphore(io->semaphore);
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::Read(const AudioTimeStamp *inStartTime,
		AudioBufferList *outData)
{
	// add a temporary IOProc listener and wait for it to be served

	ioProcListMutex->Lock();
	IOProcTracker *io = pa_xnew0(IOProcTracker, 1);
	io->proc = oneShotIOProc;
	io->clientData = io;
	io->bufferList = outData;
	MPCreateSemaphore(1, 0, &io->semaphore);
	memcpy(&io->startTime, inStartTime, sizeof(io->startTime));

	CFArrayAppendValue(ioProcList, io);
	ioProcListMutex->Unlock();

	MPWaitOnSemaphore(io->semaphore, kDurationForever);
	
	DestroyIOProcID((AudioDeviceIOProcID) io);

	return kAudioHardwareNoError;
}
#endif

OSStatus
PA_Device::Read(const AudioTimeStamp *, AudioBufferList *)
{
	return kAudioHardwareUnsupportedOperationError;
}

OSStatus
PA_Device::GetCurrentTime(AudioTimeStamp *outTime)
{
	TraceCall();
	memset(outTime, 0, sizeof(*outTime));

	outTime->mRateScalar = 1.0;
	outTime->mHostTime = CAHostTimeBase::GetCurrentTime();
	outTime->mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;

	return kAudioHardwareNoError;
}

OSStatus
PA_Device::TranslateTime(const AudioTimeStamp *inTime,
			 AudioTimeStamp *outTime)
{
	memcpy(outTime, inTime, sizeof(*outTime));
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetNearestStartTime(AudioTimeStamp *ioRequestedStartTime,
			       UInt32 /* inFlags */)
{
	memset(ioRequestedStartTime, 0, sizeof(*ioRequestedStartTime));
	return kAudioHardwareNoError;
}

#pragma mark ### properties ###

Boolean
PA_Device::HasProperty(const AudioObjectPropertyAddress *inAddress)
{	
	switch (inAddress->mSelector) {
			//case kAudioDevicePropertyIcon:
                case kAudioDevicePropertyActualSampleRate:
                case kAudioDevicePropertyBufferFrameSize:
                case kAudioDevicePropertyBufferFrameSizeRange:
                case kAudioDevicePropertyBufferSize:
                case kAudioDevicePropertyBufferSizeRange:
                case kAudioDevicePropertyClockDomain:
                //case kAudioDevicePropertyConfigurationApplication:
                case kAudioDevicePropertyDeviceCanBeDefaultDevice:
                case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
                case kAudioDevicePropertyDeviceHasChanged:
                case kAudioDevicePropertyDeviceIsAlive:
                case kAudioDevicePropertyDeviceIsRunning:
                case kAudioDevicePropertyDeviceIsRunningSomewhere:
                case kAudioDevicePropertyDeviceManufacturer:
                case kAudioDevicePropertyDeviceName:
                case kAudioDevicePropertyDeviceUID:
                //case kAudioDevicePropertyIOProcStreamUsage:
		case kAudioDevicePropertyIsHidden:
                case kAudioDevicePropertyLatency:
                case kAudioDevicePropertyModelUID:
		case kAudioDevicePropertyNominalSampleRate:
		//case kAudioDevicePropertyRelatedDevices:
                case kAudioDevicePropertySafetyOffset:
                case kAudioDevicePropertyStreamConfiguration:
                case kAudioDevicePropertyStreams:
                case kAudioDevicePropertyTransportType:
                case kAudioObjectPropertyManufacturer:
                case kAudioObjectPropertyName:
		case kAudioDevicePropertyAvailableNominalSampleRates:
		
		// stream properties
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
			return true;
	}

	return super::HasProperty(inAddress);
}

OSStatus
PA_Device::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	switch (inAddress->mSelector) {
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertyBufferSize:
		case kAudioDevicePropertyNominalSampleRate:
			
		// stream properties
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioDevicePropertyStreamFormat:
			
			*outIsSettable = true;
			return kAudioHardwareNoError;
	}
	
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_Device::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 inQualifierDataSize,
			       const void *inQualifierData,
			       UInt32 *outDataSize)
{
	Boolean isInput = inAddress->mScope == kAudioDevicePropertyScopeInput;

	switch (inAddress->mSelector) {
		case kAudioObjectPropertyName:
		case kAudioObjectPropertyManufacturer:
		case kAudioDevicePropertyConfigurationApplication:
		case kAudioDevicePropertyDeviceUID:
		case kAudioDevicePropertyModelUID:
			*outDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyRelatedDevices:
			*outDataSize = 0; //sizeof(AudioObjectID);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyIsHidden:
		case kAudioDevicePropertyTransportType:
		case kAudioDevicePropertyClockDomain:
		case kAudioDevicePropertyDeviceIsAlive:
		case kAudioDevicePropertyDeviceHasChanged:
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
		case kAudioDevicePropertyLatency:
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertySafetyOffset:
		case kAudioDevicePropertyBufferSize:
			*outDataSize = sizeof(UInt32);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyBufferFrameSizeRange:
		case kAudioDevicePropertyBufferSizeRange:
			*outDataSize = sizeof(AudioValueRange);
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyStreams:
			*outDataSize = sizeof(AudioStreamID) * (isInput ? nInputStreams : nOutputStreams);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyStreamConfiguration:
			*outDataSize = sizeof(AudioBufferList);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyIOProcStreamUsage:
			//theAnswer = SizeOf32(void*) + SizeOf32(UInt32) + (GetNumberStreams(isInput) * SizeOf32(UInt32));
			*outDataSize = 0;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyNominalSampleRate:
		case kAudioDevicePropertyActualSampleRate:
			*outDataSize = sizeof(Float64);
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyDeviceName:
			*outDataSize = CFStringGetLength(deviceName) + 1;
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyDeviceManufacturer:
			*outDataSize = CFStringGetLength(deviceManufacturer) + 1;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyAvailableNominalSampleRates:
			*outDataSize = sizeof(AudioValueRange);
			return kAudioHardwareNoError;			

		//case kAudioDevicePropertyIcon:
		//	return sizeof(CFURLRef);

		// Stream properties
		case kAudioStreamPropertyAvailableVirtualFormats:
		case kAudioStreamPropertyAvailablePhysicalFormats:
			*outDataSize = sizeof(AudioStreamRangedDescription);
			return kAudioHardwareNoError;

		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
			*outDataSize = sizeof(AudioStreamBasicDescription);
			return kAudioHardwareNoError;
	}
	
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_Device::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 *ioDataSize,
			   void *outData)
{
	Boolean isInput = inAddress->mScope == kAudioDevicePropertyScopeInput;
	char tmp[100];

	switch (inAddress->mSelector) {
		case kAudioObjectPropertyName:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceName);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyModelUID:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), modelUID);
			*ioDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyDeviceUID:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceUID);
			*ioDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;

		case kAudioObjectPropertyManufacturer:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceManufacturer);
			*ioDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyDeviceName:
			*ioDataSize = MIN(*ioDataSize, sizeof(tmp));
			memset(outData, 0, *ioDataSize);
			CFStringGetCString(deviceName, tmp, sizeof(tmp), kCFStringEncodingASCII);
			memcpy(outData, tmp, *ioDataSize);
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyDeviceManufacturer:
			*ioDataSize = MIN(*ioDataSize, sizeof(tmp));
			memset(outData, 0, *ioDataSize);
			CFStringGetCString(deviceManufacturer, tmp, sizeof(tmp), kCFStringEncodingASCII);
			memcpy(outData, tmp, *ioDataSize);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyTransportType:
			*static_cast<UInt32*>(outData) = 'virt';
			return kAudioHardwareNoError;

		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceIsAlive:
			/* "always true" */
			*static_cast<UInt32*>(outData) = 1;
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyIsHidden:
			/* "always false" */
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
			*static_cast<UInt32*>(outData) = isRunning;
			return kAudioHardwareNoError;			
			
		case kAudioDevicePropertyBufferFrameSize:
			*static_cast<UInt32*>(outData) = GetIOBufferFrameSize();
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferSize:
			*static_cast<UInt32*>(outData) = GetIOBufferFrameSize() * deviceBackend->GetFrameSize();
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferFrameSizeRange:
			// FIXME
			static_cast<AudioValueRange*>(outData)->mMinimum = 64;
			static_cast<AudioValueRange*>(outData)->mMaximum = 8192;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyAvailableNominalSampleRates:
			static_cast<AudioValueRange*>(outData)->mMinimum = GetSampleRate();
			static_cast<AudioValueRange*>(outData)->mMaximum = GetSampleRate();
			return kAudioHardwareNoError;

		case kAudioDevicePropertyActualSampleRate:
		case kAudioDevicePropertyNominalSampleRate:
			*static_cast<Float64*>(outData) = GetSampleRate();
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyClockDomain:
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;

		case kAudioDevicePropertySafetyOffset:
		case kAudioDevicePropertyLatency:
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyStreams: {
			AudioStreamID *list = static_cast<AudioStreamID*>(outData);
			
			if (isInput) {
				UInt32 n = MIN(nInputStreams, *ioDataSize / sizeof(UInt32));
				*ioDataSize = n * sizeof(UInt32);
				for (UInt32 i = 0; i < n; i++)
					list[i] = inputStreams[i]->GetObjectID();
			} else {
				UInt32 n = MIN(nOutputStreams, *ioDataSize / sizeof(UInt32));
				*ioDataSize = n * sizeof(UInt32);
				for (UInt32 i = 0; i < n; i++)
					list[i] = outputStreams[i]->GetObjectID();
			}
			
			return kAudioHardwareNoError;
		}

		case kAudioDevicePropertyStreamConfiguration: {
			AudioBufferList *list = static_cast<AudioBufferList*>(outData);
			memset(list, 0, sizeof(*list));
			list->mNumberBuffers = 1;
			list->mBuffers[0].mNumberChannels = (isInput ? nInputStreams : nOutputStreams) * 2;
			list->mBuffers[0].mDataByteSize = GetIOBufferFrameSize() * deviceBackend->GetFrameSize();
			*ioDataSize = sizeof(*list);
			return kAudioHardwareNoError;
		}

		case kAudioStreamPropertyAvailablePhysicalFormats:
		case kAudioStreamPropertyAvailableVirtualFormats:
			*ioDataSize = MIN(*ioDataSize, sizeof(physicalFormat));
			memcpy(outData, &physicalFormat, *ioDataSize);
			return kAudioHardwareNoError;

		case kAudioStreamPropertyPhysicalFormats:
		case kAudioStreamPropertyPhysicalFormat:
		case kAudioDevicePropertyStreamFormats:
		case kAudioDevicePropertyStreamFormat:
			*ioDataSize = MIN(*ioDataSize, sizeof(streamDescription));
			memcpy(outData, &streamDescription, *ioDataSize);
			return kAudioHardwareNoError;
	}

	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Device::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
	switch (inAddress->mSelector) {
		case kAudioDevicePropertyDeviceIsRunning:
			EnableAllIOProcs(*(UInt32 *) inData);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferFrameSize:
			SetBufferSize(*(UInt32 *) inData);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferSize:
			SetBufferSize((*(UInt32 *) inData) * 8);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyNominalSampleRate:
			sampleRate = *static_cast<const Float64*>(inData);
			DebugLog("SETTING sample rate %f", sampleRate);
			//FIXME - tell the backend
			return kAudioHardwareNoError;

		case kAudioStreamPropertyPhysicalFormat:
			return kAudioHardwareNoError;

		case kAudioDevicePropertyStreamFormat:
			inDataSize = MIN(inDataSize, sizeof(streamDescription));
			memcpy(&streamDescription, inData, inDataSize);
			return kAudioHardwareNoError;
	}
	
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### properties (legacy interface) ###

OSStatus
PA_Device::GetPropertyInfo(UInt32 inChannel,
			   Boolean isInput,
			   AudioDevicePropertyID inPropertyID,
			   UInt32 *outSize,
			   Boolean *outWritable)
{
	AudioObjectPropertyAddress addr;
	OSStatus ret = kAudioHardwareNoError;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;

	if (!HasProperty(&addr)) {
		DebugLog(" WHO!?");
		return kAudioHardwareUnknownPropertyError;
	}
	
	if (outWritable)
		ret = IsPropertySettable(&addr, outWritable);
	
	if (ret != kAudioHardwareNoError)
		return ret;
	
	if (outSize)
		ret = GetPropertyDataSize(&addr, 0, NULL, outSize);
		
	return ret;
}

OSStatus
PA_Device::GetProperty(UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 *ioPropertyDataSize,
		       void *outPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;

	return GetPropertyData(&addr, 0, NULL, ioPropertyDataSize, outPropertyData);
}

OSStatus
PA_Device::SetProperty(const AudioTimeStamp * /* inWhen */,
		       UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 inPropertyDataSize,
		       const void *inPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;
	
	return SetPropertyData(&addr, 0, NULL, inPropertyDataSize, inPropertyData);
}

#pragma mark ### async command ###

OSStatus
PA_Device::DoStop(AudioDeviceIOProcID inProcID)
{
	pthread_mutex_lock(&ioProcListMutex);
	IOProcTracker *io = FindIOProcByID(inProcID);

	if (!io)
		io = FindIOProc(inProcID);

	if (io)
		io->enabled = false;
		
	UInt32 count = CountEnabledIOProcs();
	pthread_mutex_unlock(&ioProcListMutex);

	if (inProcID && !io) {
		DebugLog("IOProc has not been added");
		return kAudioHardwareIllegalOperationError;
	}

	DebugIOProc("Stopping IOProc @%p", io);

	if (isRunning && count == 0) {
		isRunning = false;
		DebugLog("Stopping hardware");
		deviceControl->SignOffDevice();
		deviceBackend->Disconnect();
		AnnounceDeviceChange();	
	}
	
	return kAudioHardwareNoError;
}

static CFDataRef
staticMessagePortDispatch(CFMessagePortRef /* local */,
			  SInt32 msgid,
			  CFDataRef data,
			  void *info)
{
	PA_Device *dev = static_cast<PA_Device *> (info);
	
	DebugLog("msgid = %d", (int) msgid);
	
	switch (msgid) {
		case ASYC_MSG_DEVICE_STOP: {
			AudioDeviceIOProcID ioid;
			CFDataGetBytes(data, CFRangeMake(0, sizeof(ioid)), (Byte *) &ioid);
			OSStatus ret = dev->DoStop(ioid);
			return CFDataCreate(dev->GetAllocator(), (Byte *) &ret, sizeof(ret));
		}

		default:
			DebugLog("Unhandled message %d", (int) msgid);
	}
	
	return NULL;
}

OSStatus
PA_Device::SendAsyncMessage(SInt32 command, CFDataRef data)
{
	CFDataRef returnData = NULL;
	CFMessagePortSendRequest(remotePort, command, data,
				 10, 10, kCFRunLoopDefaultMode, &returnData);
	
	if (returnData) {
		OSStatus ret;
		CFDataGetBytes(returnData, CFRangeMake(0, sizeof(ret)), (Byte *) &ret);
		CFRelease(returnData);
		return ret;
	}

	return 0;
}

void
PA_Device::SetupAsyncCommands()
{
	CFMessagePortContext context;
	CFRunLoopSourceRef source;
	CFStringRef portNameRef = NULL;

	// create a random string for out port name to avoid getting messages like
	//    bootstrap_register(): failed 1100 (0x44c) 'Permission denied'
	srandom(clock());
	portNameRef = CFStringCreateWithFormat(GetAllocator(), NULL, CFSTR("%08x%08x"),
					       random(), random());
	memset(&context, 0, sizeof(context));
	context.info = this;
	
	localPort = CFMessagePortCreateLocal(GetAllocator(), portNameRef,
					     staticMessagePortDispatch, &context, NULL);
	source = CFMessagePortCreateRunLoopSource(GetAllocator(), localPort, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	
	remotePort = CFMessagePortCreateRemote(GetAllocator(), portNameRef);
	
	CFRelease(portNameRef);
}
