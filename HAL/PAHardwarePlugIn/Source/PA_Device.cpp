#define CLASS_NAME "PA_Device"

//#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>

#include "PA_Device.h"
#include "PA_Stream.h"
#include "PA_DeviceBackend.h"
#include "PA_DeviceControl.h"
#include "CAAudioBufferList.h"

//#include <pulse/pulseaudio.h>

#define super PA_Object
#define TraceCall(x) printf("PA_Device::%s() :%d\n", __func__, __LINE__);

PA_Device::PA_Device(PA_Plugin *inPlugin) : plugin(inPlugin)
{
}

PA_Device::~PA_Device()
{
}

IOProcTracker *
PA_Device::FindIOProc(AudioDeviceIOProc inProc)
{
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc)
			return io;
	}

	return NULL;
}

IOProcTracker *
PA_Device::FindIOProcByID(AudioDeviceIOProcID inProcID)
{
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inProcID)
			return io;
	}
	
	return NULL;
}

OSStatus
PA_Device::RegisterObjects()
{
	AudioObjectID oid = GetObjectID();
	std::vector<AudioStreamID> streamIDs;

	AudioObjectsPublishedAndDied(plugin->GetInterface(),
				     kAudioObjectSystemObject,
				     1, &oid, 0, NULL);
	
	for (UInt32 i = 0; i < nInputStreams; i++)
		if (inputStreams[i])
			streamIDs.push_back(inputStreams[i]->GetObjectID());

	for (UInt32 i = 0; i < nOutputStreams; i++)
		if (outputStreams[i])
			streamIDs.push_back(outputStreams[i]->GetObjectID());

	// now tell the HAL about the new stream IDs
	if (streamIDs.size() != 0)
		return AudioObjectsPublishedAndDied(plugin->GetInterface(),
						    GetObjectID(),
						    streamIDs.size(),
						    &(streamIDs.front()),
						    0, NULL);	
	return kAudioHardwareNoError;
}

void
PA_Device::Initialize()
{
	UInt32 i;

	TraceCall();
	ioProcList = CFArrayCreateMutable(NULL, 0, NULL);
	ioProcListMutex = new CAMutex("ioProcListMutex");
	
	deviceUID = CFStringCreateWithCString(NULL, "PulseAudio", kCFStringEncodingASCII);
	deviceName = CFStringCreateWithCString(NULL, "PulseAudio", kCFStringEncodingASCII);
	deviceManufacturer = CFStringCreateWithCString(NULL, "pulseaudio.org", kCFStringEncodingASCII);

	nInputStreams = 1;
	nOutputStreams = 1;
	
	inputStreams = pa_xnew0(PA_Stream *, nInputStreams);
	outputStreams = pa_xnew0(PA_Stream *, nOutputStreams);

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

	deviceBackend = new PA_DeviceBackend(this);
	deviceBackend->Initialize();
	
	deviceControl = new PA_DeviceControl(this);
	deviceControl->Initialize();

	sampleRate = 48000.0f;
	
	streamDescription.mSampleRate = sampleRate;
	streamDescription.mFormatID = kAudioFormatLinearPCM;
	streamDescription.mFormatFlags = kAudioFormatFlagIsFloat;
	streamDescription.mBitsPerChannel = 16;
	streamDescription.mChannelsPerFrame = 2;
	streamDescription.mBytesPerPacket = 2 * GetIOBufferFrameSize() * GetFrameSize();
	streamDescription.mFramesPerPacket = GetIOBufferFrameSize();
	streamDescription.mBytesPerFrame = GetFrameSize();

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
	
	RegisterObjects();
}

void
PA_Device::Teardown()
{
	TraceCall();
	
	deviceBackend->Teardown();
	delete deviceBackend;
	deviceBackend = NULL;

	deviceControl->Teardown();
	delete deviceControl;
	deviceControl = NULL;
	
	CFRelease(deviceUID);
	CFRelease(deviceName);
	CFRelease(deviceManufacturer);
	
	deviceUID = NULL;
	deviceName = NULL;
	deviceManufacturer = NULL;
	
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		pa_xfree(io);
	}
	
	CFRelease(ioProcList);
	ioProcList = NULL;
	
	delete ioProcListMutex;
	ioProcListMutex = NULL;

	for (UInt32 i = 0; i < nInputStreams; i++)
		delete inputStreams[i];

	for (UInt32 i = 0; i < nOutputStreams; i++)
		delete outputStreams[i];

	pa_xfree(inputStreams);
	pa_xfree(outputStreams);
}

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
	ioProcListMutex->Lock();

	if (FindIOProc(inProc)) {
		DebugLog("IOProc has already been added");
		ioProcListMutex->Unlock();
		return kAudioHardwareIllegalOperationError;
	}
	
	IOProcTracker *io = pa_xnew0(IOProcTracker, 1);
	io->proc = inProc;
	io->clientData = inClientData;
	
	if (outIOProcID)
		*outIOProcID = (AudioDeviceIOProcID) io;

	CFArrayAppendValue(ioProcList, io);
	ioProcListMutex->Unlock();
	
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::DestroyIOProcID(AudioDeviceIOProcID inIOProcID)
{
	ioProcListMutex->Lock();

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inIOProcID) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			ioProcListMutex->Unlock();
			return kAudioHardwareNoError;
		}
	}
	
	ioProcListMutex->Unlock();
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
	ioProcListMutex->Lock();
	
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			ioProcListMutex->Unlock();
			return kAudioHardwareNoError;
		}
	}
	
	ioProcListMutex->Unlock();
	DebugLog("IOProc has not been added");
	return kAudioHardwareIllegalOperationError;
}

OSStatus
PA_Device::Start(AudioDeviceIOProc inProc)
{
	return StartAtTime(inProc, NULL, 0);
}

OSStatus
PA_Device::StartAtTime(AudioDeviceIOProc inProc,
		       AudioTimeStamp *ioRequestedStartTime,
		       UInt32 inFlags)
{
	ioProcListMutex->Lock();
	IOProcTracker *io = FindIOProc(inProc);
	
	if (io) {
		io->enabled = true;
		io->startTimeFlags = inFlags;

		if (ioRequestedStartTime)
			memcpy(&io->startTime, ioRequestedStartTime, sizeof(*ioRequestedStartTime));
	}

	ioProcListMutex->Unlock();
	
	if (io)
		return kAudioHardwareNoError;
	else {
		DebugLog("IOProc has not been added");
		return kAudioHardwareIllegalOperationError;
	}
}

OSStatus
PA_Device::Stop(AudioDeviceIOProc inProc)
{
	ioProcListMutex->Lock();
	IOProcTracker *io = FindIOProc(inProc);
	
	if (io)
		io->enabled = false;
	
	ioProcListMutex->Unlock();
	
	if (io)
		return kAudioHardwareNoError;
	else {
		DebugLog("IOProc has not been added");
		return kAudioHardwareIllegalOperationError;
	}
}

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

OSStatus
PA_Device::GetCurrentTime(AudioTimeStamp *outTime)
{
	TraceCall();
	memset(outTime, 0, sizeof(*outTime));
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
			       UInt32 inFlags)
{
	memset(ioRequestedStartTime, 0, sizeof(*ioRequestedStartTime));
	return kAudioHardwareNoError;
}

void
PA_Device::EnableAllIOProcs(Boolean enabled)
{
	ioProcListMutex->Lock();

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		io->enabled = enabled;
	}

	ioProcListMutex->Unlock();
}

void
PA_Device::SetBufferSize(UInt32 size)
{
	bufferFrameSize = size;
}

UInt32
PA_Device::GetFrameSize()
{
	// FIXME
	return 8;
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
                case kAudioDevicePropertyChannelCategoryName:
                case kAudioDevicePropertyChannelName:
                case kAudioDevicePropertyChannelNumberName:
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
                case kAudioDevicePropertyStreamFormat:
                case kAudioDevicePropertyStreams:
                case kAudioDevicePropertyTransportType:
                case kAudioObjectPropertyElementCategoryName:
                case kAudioObjectPropertyElementName:
                case kAudioObjectPropertyElementNumberName:
                case kAudioObjectPropertyManufacturer:
                case kAudioObjectPropertyName:
		case kAudioDevicePropertyAvailableNominalSampleRates:
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
		case kAudioDevicePropertyStreamFormat:
			return true;
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
		case kAudioObjectPropertyElementName:
		case kAudioObjectPropertyElementCategoryName:
		case kAudioObjectPropertyElementNumberName:
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

		case kAudioDevicePropertyAvailableNominalSampleRates:
		case kAudioDevicePropertyNominalSampleRate:
		case kAudioDevicePropertyActualSampleRate:
			*outDataSize = sizeof(Float64);
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyDeviceName:
			*outDataSize = CFStringGetLength(deviceName);
			return kAudioHardwareNoError;
	
		case kAudioDevicePropertyDeviceManufacturer:
			*outDataSize = CFStringGetLength(deviceManufacturer);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyChannelName:
		case kAudioDevicePropertyChannelCategoryName:
		case kAudioDevicePropertyChannelNumberName:
			*outDataSize = 0;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyStreamFormat:
			*outDataSize = sizeof(AudioStreamBasicDescription);
			return kAudioHardwareNoError;

		//case kAudioDevicePropertyIcon:
		//	return sizeof(CFURLRef);
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

	switch (inAddress->mSelector) {
		case kAudioObjectPropertyName:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceName);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyModelUID:
		case kAudioDevicePropertyDeviceUID:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceUID);
			*ioDataSize = sizeof(CFStringRef);
			//CFShow(*static_cast<CFStringRef*>(outData));
			return kAudioHardwareNoError;

		case kAudioObjectPropertyManufacturer:
			*static_cast<CFStringRef*>(outData) = CFStringCreateCopy(plugin->GetAllocator(), deviceManufacturer);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyDeviceName:
			CFStringGetCString(deviceName, (char *) outData, *ioDataSize, kCFStringEncodingASCII);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyDeviceManufacturer:
			CFStringGetCString(deviceManufacturer, (char *) outData, *ioDataSize, kCFStringEncodingASCII);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyTransportType:
			*static_cast<UInt32*>(outData) = 'virt';
			return kAudioHardwareNoError;

		/* "always true" */
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceIsAlive:
			*static_cast<UInt32*>(outData) = 1;
			return kAudioHardwareNoError;
			
		/* "always false" */
		case kAudioDevicePropertyIsHidden:
			*static_cast<UInt32*>(outData) = 0;
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
			*static_cast<UInt32*>(outData) = deviceBackend->isRunning();
			return kAudioHardwareNoError;			
			
		case kAudioDevicePropertyBufferFrameSize:
			*static_cast<UInt32*>(outData) = GetIOBufferFrameSize();
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferSize:
			*static_cast<UInt32*>(outData) = GetIOBufferFrameSize() * GetFrameSize();
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferFrameSizeRange:
			// FIXME
			static_cast<AudioValueRange*>(outData)->mMinimum = 64;
			static_cast<AudioValueRange*>(outData)->mMaximum = 8192;
			return kAudioHardwareNoError;

		case kAudioDevicePropertyAvailableNominalSampleRates:
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
			list->mNumberBuffers = 1;
			list->mBuffers[0].mNumberChannels = (isInput ? nInputStreams : nOutputStreams) * 2;
			list->mBuffers[0].mDataByteSize = GetIOBufferFrameSize() * GetFrameSize();
			list->mBuffers[0].mData = NULL;
			*ioDataSize = sizeof(*list);
			return kAudioHardwareNoError;
		}

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
			EnableAllIOProcs(!!(*(UInt32 *) inData));
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferFrameSize:
			SetBufferSize(*(UInt32 *) inData);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferSize:
			SetBufferSize((*(UInt32 *) inData) * 8);
			return kAudioHardwareNoError;
			
		case kAudioDevicePropertyNominalSampleRate:
			sampleRate = *static_cast<const Float64*>(inData);
			//FIXME - tell the backend
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
