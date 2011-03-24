#include "PAHP_Device.h"
#include "PAHP_Control.h"
#include "PAHP_PlugIn.h"
#include "PAHP_Stream.h"

#include "HP_DeviceSettings.h"
#include "HP_HogMode.h"
#include "HP_IOCycleTelemetry.h"
#include "HP_IOProcList.h"
#include "HP_IOThread.h"

#include "CAAudioBufferList.h"
#include "CAAudioTimeStamp.h"
#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAHostTimeBase.h"
#include "CALogMacros.h"
#include "CAMutex.h"

#pragma mark ### Logging ###

#if	CoreAudio_Debug
	#define Log_ControlLife				1
	#define	Log_HardwareStartStop			1
	#define	Log_HardwareNotifications		1
	#define	Log_InterestNotification		1
#endif

#pragma mark ### PAHP_Device ###

PAHP_Device::PAHP_Device(AudioDeviceID	 inAudioDeviceID,
			 PAHP_PlugIn	*inPlugIn) :
	HP_Device(inAudioDeviceID, kAudioDeviceClassID, inPlugIn, 1, false),
	plugin(inPlugIn),
	hogMode(NULL),
	IOThread(NULL),
	anchorHostTime(0),
	controlsInitialized(false),
	controlProperty(NULL)
{
}

PAHP_Device::~PAHP_Device()
{
}

void
PAHP_Device::Initialize()
{
	HP_Device::Initialize();

	hogMode = new HP_HogMode(this);
	hogMode->Initialize();

	IOThread = new HP_IOThread(this);

	CreateStreams();

	// set the default buffer size
	mIOBufferFrameSize = 512;
	mIOBufferFrameSize = DetermineIOBufferFrameSize();

	// allocate the property object that maps device control properties onto control objects
	controlProperty = new HP_DeviceControlProperty(this);
	AddProperty(controlProperty);

	// make sure that the controls are always instantiated in the master process so that they can be saved later
	UInt32 isMaster = 0;
	UInt32 size = sizeof(UInt32);
	AudioHardwareGetProperty(kAudioHardwarePropertyProcessIsMaster, &size, &isMaster);
	if (isMaster)
		CreateControls();
	
	// PulseAudio
	
}

void
PAHP_Device::Teardown()
{
	Do_StopAllIOProcs();

	if (hogMode) {
		if (hogMode->CurrentProcessIsOwner())
			hogMode->Release();

		delete hogMode;
		hogMode = NULL;
	}

	if (controlProperty) {
		RemoveProperty(controlProperty);
		delete controlProperty;
		controlProperty = NULL;
	}

	ReleaseControls();
	ReleaseStreams();	

	delete IOThread;
	IOThread = NULL;

	HP_Device::Teardown();
}

void
PAHP_Device::Finalize()
{
	// Finalize() is called in place of Teardown() when we're being lazy about
	// cleaning up. The idea is to do as little work as possible here.

	PAHP_Stream *stream;
	UInt32 idx;

	// input
	for (idx = 0; idx != GetNumberStreams(true); idx++) {
		stream = static_cast<PAHP_Stream*>(GetStreamByIndex(true, idx));
		stream->Finalize();
	}

	// output
	for (idx = 0; idx != GetNumberStreams(false); idx++) {
		stream = static_cast<PAHP_Stream*>(GetStreamByIndex(false, idx));
		stream->Finalize();
	}

	if (hogMode->CurrentProcessIsOwner())
		hogMode->Release();
}

CFStringRef
PAHP_Device::CopyDeviceName() const
{
	CFStringRef str = CFSTR("PulseAudio");
	CFRetain(str);
	return str;
}

CFStringRef
PAHP_Device::CopyDeviceManufacturerName() const
{
	CFStringRef str = CFSTR("The PulseAudio community");
	CFRetain(str);
	return str;
}

CFStringRef
PAHP_Device::CopyDeviceUID() const
{
	CFStringRef str = CFSTR("PulseAudio");
	CFRetain(str);
	return str;
}

bool
PAHP_Device::HogModeIsOwnedBySelf() const
{
	return hogMode ? hogMode->CurrentProcessIsOwner() : false;
}

bool
PAHP_Device::HogModeIsOwnedBySelfOrIsFree() const
{
	return hogMode ? hogMode->CurrentProcessIsOwnerOrIsFree() : true;
}

void
PAHP_Device::HogModeStateChanged()
{
	HP_Device::HogModeStateChanged();

	// hold the device state lock until the changes have been completed
	// it is vital that whenever taking both locks, that the device state
	// lock be take prior to attempting to lock the IO lock.
	bool doUnlockDeviceStateGuard = GetDeviceStateMutex().Lock();

	// Synchronize with the IO thread.
	bool doUnlockIOThreadGuard = IOThread->GetIOGuard().Lock();

	RefreshAvailableStreamFormats();

	// unlock the locks so that re-entry can happen
	if(doUnlockIOThreadGuard)
		IOThread->GetIOGuard().Unlock();

	if(doUnlockDeviceStateGuard)
		GetDeviceStateMutex().Unlock();
}

bool
PAHP_Device::HasProperty(const AudioObjectPropertyAddress &inAddress) const
{
	CAMutex::Locker stateMutex(const_cast<PAHP_Device*>(this)->GetDeviceStateMutex());

	//  create the controls if necessary
	if (IsControlRelatedProperty(inAddress.mSelector))
		const_cast<PAHP_Device*>(this)->CreateControls();

	switch (inAddress.mSelector) {
		case kAudioDevicePropertyIOCycleUsage:
			return true;

		default:
			return HP_Device::HasProperty(inAddress);
	}

	return false;
}

bool
PAHP_Device::IsPropertySettable(const AudioObjectPropertyAddress &inAddress) const
{
	CAMutex::Locker stateMutex(const_cast<PAHP_Device*>(this)->GetDeviceStateMutex());

	//  create the controls if necessary
	if (IsControlRelatedProperty(inAddress.mSelector))
		const_cast<PAHP_Device*>(this)->CreateControls();

	switch (inAddress.mSelector) {
		case kAudioDevicePropertyHogMode:
			return true;

		case kAudioDevicePropertyIOCycleUsage:
			return true;

		default:
			return HP_Device::IsPropertySettable(inAddress);
	}

	return false;
}

UInt32
PAHP_Device::GetPropertyDataSize(const AudioObjectPropertyAddress	&inAddress,
				 UInt32					 inQualifierDataSize,
				 const void				*inQualifierData) const
{
	CAMutex::Locker stateMutex(const_cast<PAHP_Device*>(this)->GetDeviceStateMutex());

	//  create the controls if necessary
	if (IsControlRelatedProperty(inAddress.mSelector))
		const_cast<PAHP_Device*>(this)->CreateControls();

	switch (inAddress.mSelector) {
		case kAudioDevicePropertyIOCycleUsage:
			return sizeof(Float32);

		default:
			return HP_Device::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData);
	}

	return 0;
}

void
PAHP_Device::GetPropertyData(const AudioObjectPropertyAddress	&inAddress,
			     UInt32				 inQualifierDataSize,
			     const void				*inQualifierData,
			     UInt32				&ioDataSize,
			     void				*outData) const
{
	// take and hold the state mutex
	CAMutex::Locker stateMutex(const_cast<PAHP_Device*>(this)->GetDeviceStateMutex());

	// create the controls if necessary
	if (IsControlRelatedProperty(inAddress.mSelector))
		const_cast<PAHP_Device*>(this)->CreateControls();

	switch(inAddress.mSelector) {
		case kAudioDevicePropertyHogMode:
			ThrowIf(ioDataSize != GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Device::GetPropertyData: wrong data size for kAudioDevicePropertyHogMode");
			*(static_cast<pid_t*>(outData)) = hogMode->GetOwner();
			break;

		case kAudioDevicePropertyIOCycleUsage:
			ThrowIf(ioDataSize != GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Device::GetPropertyData: wrong data size for kAudioDevicePropertyIOCycleUsage");
			*(static_cast<Float32*>(outData)) = IOThread->GetIOCycleUsage();
			break;

		default:
			HP_Device::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
			break;
	}
}

void
PAHP_Device::SetPropertyData(const AudioObjectPropertyAddress	&inAddress,
			     UInt32				 inQualifierDataSize,
			     const void				*inQualifierData,
			     UInt32				 inDataSize,
			     const void				*inData,
			     const AudioTimeStamp		*inWhen)
{
	//	take and hold the state mutex
	CAMutex::Locker stateMutex(GetDeviceStateMutex());

	//  create the controls if necessary
	if (IsControlRelatedProperty(inAddress.mSelector))
		CreateControls();

	switch (inAddress.mSelector) {
		case kAudioDevicePropertyHogMode:
			ThrowIf(inDataSize != GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Device::SetPropertyData: wrong data size for kAudioDevicePropertyHogMode");
			if(hogMode->IsFree()) {
				hogMode->Take();
			} else if(hogMode->CurrentProcessIsOwner()) {
				hogMode->Release();
			} else {
				DebugMessage("PAHP_Device::SetPropertyData: hog mode owned by another process");
				throw CAException(kAudioDevicePermissionsError);
			}

			HogModeStateChanged();
			*((pid_t*)inData) = hogMode->GetOwner();
			break;

		case kAudioDevicePropertyIOCycleUsage:
			{
				ThrowIf(inDataSize != GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData),
					CAException(kAudioHardwareBadPropertySizeError),
					"PAHP_Device::SetPropertyData: wrong data size for kAudioDevicePropertyIOCycleUsage");
				IOThread->SetIOCycleUsage(*(static_cast<const Float32*>(inData)));
				CAPropertyAddress addr(kAudioDevicePropertyIOCycleUsage, kAudioObjectPropertyScopeGlobal, 0);
				PropertiesChanged(1, &addr);
			}
			break;

		default:
			HP_Device::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData, inWhen);
			break;
	};
}

void
PAHP_Device::PropertyListenerAdded(const AudioObjectPropertyAddress &inAddress)
{
	HP_Object::PropertyListenerAdded(inAddress);

	if (inAddress.mSelector == kAudioObjectPropertySelectorWildcard ||
	    IsControlRelatedProperty(inAddress.mSelector))
		CreateControls();
}

bool
PAHP_Device::IsSafeToExecuteCommand()
{
	// it isn't safe to execute commands from the IOThread
	return IOThread ? !IOThread->IsCurrentThread() : true;
}

bool
PAHP_Device::StartCommandExecution(void **outSavedCommandState)
{
	//	lock the IOGuard since we're doing something that
	//	affects what goes on in the IOThread
	*outSavedCommandState = 0;

	if (IOThread)
		*outSavedCommandState = IOThread->GetIOGuard().Lock() ? (void*)1 : (void*)0;

	return true;
}

void
PAHP_Device::FinishCommandExecution(void *inSavedCommandState)
{
	if (IOThread && inSavedCommandState)
		IOThread->GetIOGuard().Unlock();
}

void
PAHP_Device::Do_StartIOProc(AudioDeviceIOProc inProc)
{
	ThrowIf(!HogModeIsOwnedBySelfOrIsFree(),
		CAException(kAudioDevicePermissionsError),
		"PAHP_Device::Do_StartIOProc: can't start the IOProc because hog mode is owned by another process");

	// take hog mode if there are any non-mixable output streams
	if (HasAnyNonMixableStreams(false) && !HogModeIsOwnedBySelf())
		hogMode->Take();

	HP_Device::Do_StartIOProc(inProc);
}

void
PAHP_Device::Do_StartIOProcAtTime(AudioDeviceIOProc	 inProc,
				  AudioTimeStamp	&ioStartTime,
				  UInt32		 inStartTimeFlags)
{
	ThrowIf(!HogModeIsOwnedBySelfOrIsFree(),
		CAException(kAudioDevicePermissionsError),
		"PAHP_Device::Do_StartIOProcAtTime: can't start the IOProc because hog mode is owned by another process");

	// take hog mode if there are any non-mixable output streams
	if(HasAnyNonMixableStreams(false) && !HogModeIsOwnedBySelf())
		hogMode->Take();

	HP_Device::Do_StartIOProcAtTime(inProc, ioStartTime, inStartTimeFlags);
}

CAGuard *
PAHP_Device::GetIOGuard()
{
	// this method returns the CAGuard that is to be used to synchronize with the IO cycle
	// by default, there is no CAGuard to synchronize with
	return IOThread->GetIOGuardPtr();
}

bool
PAHP_Device::CallIOProcs(const AudioTimeStamp &inCurrentTime,
			 const AudioTimeStamp &inInputTime,
			 const AudioTimeStamp &inOutputTime)
{
	// This method is called by during the IO cycle by HP_IOThread when it is time to read the
	// input data call the IOProcs and write the output data. It returns whether or not the
	// operation was successful. In this sample device, this method is broken up into
	// smaller calls for specific phases of the cycle fore easier and saner handling.
	bool hardwareIOSucceeded = true;

	StartIOCycle();

	if (HasInputStreams()) {
		// refresh the input buffers
		mIOProcList->RefreshIOProcBufferLists(true);

		// pre-process the input data
		PreProcessInputData(inInputTime);

		// read the data
		hardwareIOSucceeded = ReadInputData(inInputTime, GetIOBufferSetID());

		// post-process the data that was read
		if (hardwareIOSucceeded)
			PostProcessInputData(inInputTime);
	}

	if (hardwareIOSucceeded) {
		// get the shared input buffer list
		AudioBufferList *inputBufferList = mIOProcList->GetSharedAudioBufferList(true);

		// mark the telemetry
		mIOCycleTelemetry->IOCycleIOProcsBegin(GetIOCycleNumber());

		// iterate through the IOProcs
		for (UInt32 idx = 0; idx < mIOProcList->GetNumberIOProcs(); idx++) {
			// get the IO proc
			HP_IOProc *IOProc = mIOProcList->GetIOProcByIndex(idx);

			// call it
			IOProc->Call(inCurrentTime, inInputTime, inputBufferList, inOutputTime, NULL);

			// pre-process it before handing it to the hardware
			PreProcessOutputData(inOutputTime, *IOProc);
		}

		// mark the telemetry
		mIOCycleTelemetry->IOCycleIOProcsEnd(GetIOCycleNumber());

		// write the output data
		if (HasOutputStreams())
			hardwareIOSucceeded = WriteOutputData(inOutputTime, GetIOBufferSetID());
	}

	FinishIOCycle();

	return hardwareIOSucceeded;
}

void
PAHP_Device::StartIOEngine()
{
	// the IOGuard should already be held prior to calling this routine
	if (!IsIOEngineRunning()) {
		StartHardware();
		IOThread->Start();
	}
}

void
PAHP_Device::StartIOEngineAtTime(const AudioTimeStamp &inStartTime,
				 UInt32 inStartTimeFlags)
{
	// the IOGuard should already be held prior to calling this routine
	if (!IsIOEngineRunning()) {
		// if the engine isn't already running, then just start it
		StartHardware();
		IOThread->Start();
	} else {
		// the engine is already running, so we have to resynch the IO thread to the new start time
		AudioTimeStamp startSampleTime = inStartTime;
		startSampleTime.mFlags = kAudioTimeStampSampleTimeValid;

		// factor out the input/output-ness of the start time to get the sample time of the anchor point
		if ((inStartTimeFlags & kAudioDeviceStartTimeIsInputFlag) != 0) {
			startSampleTime.mSampleTime += GetIOBufferFrameSize();
			startSampleTime.mSampleTime += GetSafetyOffset(true);
		} else {
			startSampleTime.mSampleTime -= GetIOBufferFrameSize();
			startSampleTime.mSampleTime -= GetSafetyOffset(false);
		}

		// need an extra cycle to ensure correctness
		startSampleTime.mSampleTime -= GetIOBufferFrameSize();

		// calculate the host time of the anchor point
		AudioTimeStamp startTime;
		startTime.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
		TranslateTime(startSampleTime, startTime);

		// resynch the IO thread
		IOThread->Resynch(&startTime, true);
		mIOCycleTelemetry->Resynch(GetIOCycleNumber(), startTime);
	}
}

void
PAHP_Device::StopIOEngine()
{
	// the IOGuard should already be held prior to calling this routine
	IOThread->Stop();
	StopHardware();
}

void
PAHP_Device::StartHardware()
{
	#if Log_HardwareStartStop
		DebugMessage("PAHP_Device::StartHardware: starting the hardware");
	#endif
}

void
PAHP_Device::StopHardware()
{
	#if Log_HardwareStartStop
		DebugMessage("PAHP_Device::StopHardware: stopping the hardware");
	#endif
}

void
PAHP_Device::StartIOCycle()
{
	// this method is called at the beginning of the IO cycle to kick things off
}

void
PAHP_Device::PreProcessInputData(const AudioTimeStamp& /*inInputTime*/)
{
	// this method is called just prior to reading the input data
}

bool
PAHP_Device::ReadInputData(const AudioTimeStamp& /*inStartTime*/, UInt32 /*inBufferSetID*/)
{
	// this method is called to read the input data
	// it returns true if the read completed successfully
	return true;
}

void
PAHP_Device::PostProcessInputData(const AudioTimeStamp& /*inInputTime*/)
{
	// this method is called just after reading the input data but prior to handing it to any IOProcs

	AudioBufferList *inputBufferList = mIOProcList->GetSharedAudioBufferList(true);

	// mark the telemetry
	if ((inputBufferList != NULL) && mIOCycleTelemetry->IsCapturing() &&
	    CAAudioBufferList::HasData(*inputBufferList))
		mIOCycleTelemetry->InputDataPresent(GetIOCycleNumber());
}

void
PAHP_Device::PreProcessOutputData(const AudioTimeStamp& /*inOuputTime*/, HP_IOProc& inIOProc)
{
	// this method is called just after getting the data from the IOProc but before writing it to the hardware
	if (mIOCycleTelemetry->IsCapturing() && inIOProc.BufferListHasData(false))
		mIOCycleTelemetry->OutputDataPresent(GetIOCycleNumber());
}

bool
PAHP_Device::WriteOutputData(const AudioTimeStamp& /*inStartTime*/, UInt32 /*inBufferSetID*/)
{
	// this method is called to write the output data
	// it returns true if the write completed successfully
	return true;
}

void
PAHP_Device::FinishIOCycle()
{
	// this method is called at the end of the IO cycle
}

UInt32
PAHP_Device::GetIOCycleNumber() const
{
	return IOThread->GetIOCycleNumber();
}

void
PAHP_Device::GetCurrentTime(AudioTimeStamp& outTime)
{
	ThrowIf(!IsIOEngineRunning(),
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::GetCurrentTime: can't because the engine isn't running");

	// compute the host ticks pere frame
	Float64 actualHostTicksPerFrame = CAHostTimeBase::GetFrequency() / GetCurrentNominalSampleRate();

	// clear the output time stamp
	outTime = CAAudioTimeStamp::kZero;

	// put in the current host time
	outTime.mHostTime = CAHostTimeBase::GetTheCurrentTime();

	// calculate how many host ticks away from the anchor time stamp the current host time is
	Float64 sampleOffset = 0.0;
	if(outTime.mHostTime >= anchorHostTime) {
		sampleOffset = outTime.mHostTime - anchorHostTime;
	} else {
		// do it this way to avoid overflow problems with the unsigned numbers
		sampleOffset = anchorHostTime - outTime.mHostTime;
		sampleOffset *= -1.0;
	}

	sampleOffset /= actualHostTicksPerFrame;
	sampleOffset = floor(sampleOffset);
	outTime.mSampleTime = sampleOffset;
	outTime.mRateScalar = 1.0;
	outTime.mFlags = kAudioTimeStampSampleTimeValid |
			 kAudioTimeStampHostTimeValid	|
			 kAudioTimeStampRateScalarValid;
}

void
PAHP_Device::SafeGetCurrentTime(AudioTimeStamp& outTime)
{
	// The difference between GetCurrentTime and SafeGetCurrentTime is that GetCurrentTime should only
	// be called in situations where the device state or clock state is in a known good state, such
	// as during the IO cycle. Being in a known good state allows GetCurrentTime to bypass any
	// locks that ensure coherent cross-thread access to the device time base info.
	// SafeGetCurrentTime, then, will be called when the state is in question and all the locks should
	// be obeyed.

	// Our state here in the sample device has no such threading issues, so we pass this call on
	// to GetCurrentTime.
	GetCurrentTime(outTime);
}

void
PAHP_Device::TranslateTime(const AudioTimeStamp& inTime, AudioTimeStamp& outTime)
{
	// the input time stamp has to have at least one of the sample or host time valid
	ThrowIf((inTime.mFlags & kAudioTimeStampSampleHostTimeValid) == 0,
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_Device::TranslateTime: have to have either sample time or host time valid on the input");
	ThrowIf(!IsIOEngineRunning(),
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::TranslateTime: can't because the engine isn't running");

	// compute the host ticks pere frame
	Float64 actualHostTicksPerFrame = CAHostTimeBase::GetFrequency() / GetCurrentNominalSampleRate();

	// calculate the sample time
	Float64 offset = 0.0;
	if ((outTime.mFlags & kAudioTimeStampSampleTimeValid) != 0) {
		if ((inTime.mFlags & kAudioTimeStampSampleTimeValid) != 0) {
			// no calculations necessary
			outTime.mSampleTime = inTime.mSampleTime;
		} else if ((inTime.mFlags & kAudioTimeStampHostTimeValid) != 0) {
			// calculate how many host ticks away from the current 0 time stamp the input host time is
			if(inTime.mHostTime >= anchorHostTime) {
				offset = inTime.mHostTime - anchorHostTime;
			} else {
				// do it this way to avoid overflow problems with the unsigned numbers
				offset = anchorHostTime - inTime.mHostTime;
				offset *= -1.0;
			}

			// convert it to a number of samples
			offset /= actualHostTicksPerFrame;

			// lop off the fractional sample
			outTime.mSampleTime = floor(offset);
		} else {
			// no basis for projection, so put in a 0
			outTime.mSampleTime = 0;
		}
	}

	// calculate the host time
	if ((outTime.mFlags & kAudioTimeStampHostTimeValid) != 0) {
		if ((inTime.mFlags & kAudioTimeStampHostTimeValid) != 0) {
			// no calculations necessary
			outTime.mHostTime = inTime.mHostTime;
		} else if ((inTime.mFlags & kAudioTimeStampSampleTimeValid) != 0) {
			// calculate how many samples away from the current 0 time stamp the input sample time is
			offset = inTime.mSampleTime;

			// convert it to a number of host ticks
			offset *= actualHostTicksPerFrame;

			// lop off the fractional host tick
			offset = floor(offset);

			// put in the host time as an offset from the 0 time stamp's host time
			outTime.mHostTime = anchorHostTime + static_cast<UInt64>(offset);
		} else {
			// no basis for projection, so put in a 0
			outTime.mHostTime = 0;
		}
	}

	// calculate the rate scalar
	if (outTime.mFlags & kAudioTimeStampRateScalarValid)
		// the sample device has perfect timing
		outTime.mRateScalar = 1.0;
}

void
PAHP_Device::GetNearestStartTime(AudioTimeStamp &ioRequestedStartTime, UInt32 inFlags)
{
	bool isConsultingHAL = (inFlags & kAudioDeviceStartTimeDontConsultHALFlag) == 0;
	bool isConsultingDevice = (inFlags & kAudioDeviceStartTimeDontConsultDeviceFlag) == 0;

	ThrowIf(!IsIOEngineRunning(),
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::GetNearestStartTime: can't because there isn't anything running yet");
	ThrowIf(!isConsultingHAL && !isConsultingDevice,
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::GetNearestStartTime: can't because the start time flags are conflicting");

	UInt32 IOBufferFrameSize = GetIOBufferFrameSize();
	bool isInput = (inFlags & kAudioDeviceStartTimeIsInputFlag) != 0;
	UInt32 safetyOffset = GetSafetyOffset(isInput);

	// fix up the requested time so we have everything we need
	AudioTimeStamp requestedStartTime;
	requestedStartTime.mFlags =	ioRequestedStartTime.mFlags |
					kAudioTimeStampSampleTimeValid |
					kAudioTimeStampHostTimeValid;
	TranslateTime(ioRequestedStartTime, requestedStartTime);

	// figure out the requested position in terms of the IO thread position
	AudioTimeStamp trueRequestedStartTime = requestedStartTime;

	//  only do this math if we are supposed to consult the HAL
	if (isConsultingHAL) {
		trueRequestedStartTime.mFlags = kAudioTimeStampSampleTimeValid;
		if (isInput) {
			trueRequestedStartTime.mSampleTime += IOBufferFrameSize;
			trueRequestedStartTime.mSampleTime += safetyOffset;
		} else {
			trueRequestedStartTime.mSampleTime -= IOBufferFrameSize;
			trueRequestedStartTime.mSampleTime -= safetyOffset;
		}

		AudioTimeStamp minimumStartSampleTime;
		AudioTimeStamp minimumStartTime;
		if (mIOProcList->IsOnlyNULLEnabled()) {
			// no IOProcs are enabled, so we can start whenever

			// the minimum starting time is the current time
			GetCurrentTime(minimumStartSampleTime);

			// plus some slop
			minimumStartSampleTime.mSampleTime += safetyOffset + (2 * IOBufferFrameSize);
			minimumStartTime.mFlags = kAudioTimeStampSampleTimeValid;

			if (trueRequestedStartTime.mSampleTime <
			    minimumStartSampleTime.mSampleTime) {
				// clamp it to the minimum
				trueRequestedStartTime = minimumStartSampleTime;
			}
		} else if(mIOProcList->IsAnythingEnabled()) {
			// an IOProc is already running, so the next start time is two buffers
			// from wherever the IO thread is currently
			IOThread->GetCurrentPosition(minimumStartSampleTime);
			minimumStartSampleTime.mSampleTime += (2 * IOBufferFrameSize);
			minimumStartTime.mFlags = kAudioTimeStampSampleTimeValid;

			if (trueRequestedStartTime.mSampleTime <
			    minimumStartSampleTime.mSampleTime) {
				//	clamp it to the minimum
				trueRequestedStartTime = minimumStartSampleTime;
			} else if (trueRequestedStartTime.mSampleTime >
				 minimumStartSampleTime.mSampleTime) {
				//	clamp it to an even IO cycle
				UInt32 nBuffers = static_cast<UInt32>(trueRequestedStartTime.mSampleTime
								      - minimumStartSampleTime.mSampleTime);
				nBuffers /= IOBufferFrameSize;
				nBuffers += 2;

				trueRequestedStartTime.mSampleTime = minimumStartSampleTime.mSampleTime
									+ (nBuffers * IOBufferFrameSize);
			}
		}

		//	bump the sample time in the right direction
		if(isInput) {
			trueRequestedStartTime.mSampleTime -= IOBufferFrameSize;
			trueRequestedStartTime.mSampleTime -= safetyOffset;
		} else {
			trueRequestedStartTime.mSampleTime += IOBufferFrameSize;
			trueRequestedStartTime.mSampleTime += safetyOffset;
		}
	}

	// convert it back if neccessary
	if (trueRequestedStartTime.mSampleTime != requestedStartTime.mSampleTime)
		TranslateTime(trueRequestedStartTime, requestedStartTime);

	//	now filter it through the hardware, unless told not to
	if (mIOProcList->IsOnlyNULLEnabled() && isConsultingDevice) {
	}

	//	assign the return value
	ioRequestedStartTime = requestedStartTime;
}

void
PAHP_Device::StartIOCycleTimingServices()
{
	// Note that the IOGuard is _not_ held during this call!

	// This method is called when an IO thread is in it's initialization phase
	// prior to it requiring any timing services. The device's timing services
	// should be initialized when this method returns.

	// in this sample driver, we base our timing on the CPU clock and assume a perfect sample rate
	anchorHostTime = CAHostTimeBase::GetCurrentTime();
}

bool
PAHP_Device::UpdateIOCycleTimingServices()
{
	// This method is called by an IO cycle when it's cycle starts.
	return true;
}

void
PAHP_Device::StopIOCycleTimingServices()
{
	// This method is called when an IO cycle has completed it's run and is tearing down.
	anchorHostTime = 0;
}

void
PAHP_Device::CreateStreams()
{
	//  common variables
	OSStatus	 ret = 0;
	AudioObjectID    newStreamID = 0;
	PAHP_Stream	*stream = NULL;

	std::vector<AudioStreamID> streamIDs;

	//  in this sample device there is only one input stream and one output stream

	//  instantiate an AudioStream
	ret = AudioObjectCreate(plugin->GetInterface(), GetObjectID(), kAudioStreamClassID, &newStreamID);

	if (ret == 0) {
		//  create the stream
		stream = new PAHP_Stream(newStreamID, plugin, this, true, 1);
		stream->Initialize();

		// add to the list of streams in this device
		AddStream(stream);

		// store the new stream ID
		streamIDs.push_back(newStreamID);
	}

	// claim a stream ID for the stream
	ret = AudioObjectCreate(plugin->GetInterface(), GetObjectID(), kAudioStreamClassID, &newStreamID);

	if (ret == 0) {
		stream = new PAHP_Stream(newStreamID, plugin, this, false, 1);
		stream->Initialize();
		AddStream(stream);
		streamIDs.push_back(newStreamID);
	}

	//  now tell the HAL about the new stream IDs
	if (streamIDs.size() != 0) {
		//	set the object state mutexes
		for (std::vector<AudioStreamID>::iterator iterator = streamIDs.begin();
		     iterator != streamIDs.end();
		     std::advance(iterator, 1)) {
			HP_Object *obj = HP_Object::GetObjectByID(*iterator);
			if(obj)
				HP_Object::SetObjectStateMutexForID(*iterator, obj->GetObjectStateMutex());
		}

		ret = AudioObjectsPublishedAndDied(plugin->GetInterface(),
						   GetObjectID(),
						   streamIDs.size(),
						   &(streamIDs.front()),
						   0, NULL);
		ThrowIfError(ret, CAException(ret),
			     "PAHP_Device::CreateStreams: couldn't tell the HAL about the streams");
	}
}

void
PAHP_Device::ReleaseStreams()
{
	// This method is only called when tearing down, so there isn't any need to inform the HAL about changes
	// since the HAL has long since released it's internal representation of these stream objects. Note that
	// if this method needs to be called outside of teardown, it would need to be modified to call
	// AudioObjectsPublishedAndDied (or AudioHardwareStreamsDied on pre-Tiger systems) to notify the HAL about
	// the state change.
	while (GetNumberStreams(true) > 0) {
		PAHP_Stream *stream = static_cast<PAHP_Stream*>(GetStreamByIndex(true, 0));
		HP_Object::SetObjectStateMutexForID(stream->GetObjectID(), NULL);
		RemoveStream(stream);
		stream->Teardown();
		delete stream;
	}

	while (GetNumberStreams(false) > 0) {
		PAHP_Stream *stream = static_cast<PAHP_Stream*>(GetStreamByIndex(false, 0));
		HP_Object::SetObjectStateMutexForID(stream->GetObjectID(), NULL);
		RemoveStream(stream);
		stream->Teardown();
		delete stream;
	}
}

void
PAHP_Device::RefreshAvailableStreamFormats()
{
	UInt32 index;
	PAHP_Stream *stream;

	for (index = 0; index < GetNumberStreams(true); index++) {
		stream = static_cast<PAHP_Stream*>(GetStreamByIndex(true, index));
		stream->RefreshAvailablePhysicalFormats();
	}

	for(index = 0; index < GetNumberStreams(false); index++) {
		stream = static_cast<PAHP_Stream*>(GetStreamByIndex(false, index));
		stream->RefreshAvailablePhysicalFormats();
	}
}

void
PAHP_Device::CreateControls()
{
	OSStatus ret = 0;
	UInt32 nChannels = 0, index = 0;
	HP_Control* control = NULL;

	if (controlsInitialized)
		return;

	controlsInitialized = true;

	AudioObjectID controlID;
	std::vector<AudioObjectID> controlIDs;

	//  iterate through the input channels
	nChannels = GetTotalNumberChannels(true);
	for (index = 0; index <= nChannels; index++) {
		// make an input volume control

		ret = AudioObjectCreate(plugin->GetInterface(), GetObjectID(), kAudioVolumeControlClassID, &controlID);
		if (ret == 0) {
			control = new PAHP_LevelControl(controlID,
							kAudioVolumeControlClassID,
							kAudioDevicePropertyScopeInput,
							index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}

		// make an input mute control

		ret = AudioObjectCreate(plugin->GetInterface(),
					GetObjectID(),
					kAudioMuteControlClassID,
					&controlID);
		if (ret == 0) {
			control = new PAHP_BooleanControl(controlID,
							  kAudioMuteControlClassID,
							  kAudioDevicePropertyScopeInput,
							  index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}

		// make an input data source control
		ret = AudioObjectCreate(plugin->GetInterface(),
					GetObjectID(),
					kAudioDataSourceControlClassID,
					&controlID);
		if (ret == 0) {
			control = new PAHP_SelectorControl(controlID,
							   kAudioDataSourceControlClassID,
							   kAudioDevicePropertyScopeInput,
							   index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}			
	}

	// iterate through the output channels
	nChannels = GetTotalNumberChannels(false);
	for (index = 0; index <= nChannels; index++) {
		// make an output volume control

		ret = AudioObjectCreate(plugin->GetInterface(),
					GetObjectID(),
					kAudioVolumeControlClassID,
					&controlID);
		if (ret == 0) {
			control = new PAHP_LevelControl(controlID,
							kAudioVolumeControlClassID,
							kAudioDevicePropertyScopeOutput,
							index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}

		// make an output mute control

		ret = AudioObjectCreate(plugin->GetInterface(),
					GetObjectID(),
					kAudioMuteControlClassID,
					&controlID);
		if (ret == 0) {
			control = new PAHP_BooleanControl(controlID,
							  kAudioMuteControlClassID,
							  kAudioDevicePropertyScopeOutput,
							  index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}

		// make an output data source control

		ret = AudioObjectCreate(plugin->GetInterface(),
					GetObjectID(),
					kAudioDataSourceControlClassID,
					&controlID);
		if (ret == 0) {
			control = new PAHP_SelectorControl(controlID,
							   kAudioDataSourceControlClassID,
							   kAudioDevicePropertyScopeOutput,
							   index, plugin, this);
			control->Initialize();
			AddControl(control);
			controlIDs.push_back(controlID);
		}			
	}

	if (controlIDs.size() == 0)
		return;
	
	// tell the HAL about the new controls

	for(std::vector<AudioObjectID>::iterator iterator = controlIDs.begin();
	    iterator != controlIDs.end();
	    std::advance(iterator, 1)) {
		HP_Object *obj = HP_Object::GetObjectByID(*iterator);
		if(obj)
			HP_Object::SetObjectStateMutexForID(*iterator, obj->GetObjectStateMutex());
	}

	ret = AudioObjectsPublishedAndDied(plugin->GetInterface(),
					   GetObjectID(),
					   controlIDs.size(),
					   &(controlIDs.front()),
					   0, NULL);
	ThrowIfError(ret, CAException(ret),
		     "PAHP_Device::CreateControls: couldn't tell the HAL about the controls");
}

void
PAHP_Device::ReleaseControls()
{
	// This method is only called when tearing down, so there isn't any need to inform the HAL about changes
	// since the HAL has long since released it's internal representation of these control objects. Note that
	// if this method needs to be called outside of teardown, it would need to be modified to call
	// AudioObjectsPublishedAndDied (or nothing on pre-Tiger systems, since controls weren't first
	// class objects yet) to notify the HAL about the state change.
	if (!controlsInitialized)
		return;

	controlsInitialized = false;
	ControlList::iterator iterator = mControlList.begin();

	while (iterator != mControlList.end()) {
		HP_Control *control = *iterator;
		HP_Object::SetObjectStateMutexForID(control->GetObjectID(), NULL);
		control->Teardown();
		delete control;
		std::advance(iterator, 1);
	}

	mControlList.clear();
}

bool
PAHP_Device::IsControlRelatedProperty(AudioObjectPropertySelector inSelector)
{
	// This function determines whether or not a given property selector might be implemented by a
	// control object. Note that this list only covers standard control properties and would need
	// to be augmented by any custom properties the device may have.
	switch (inSelector) {
		//  AudioObject Properties
		case kAudioObjectPropertyOwnedObjects:

		// AudioSystem Properties
		case kAudioHardwarePropertyBootChimeVolumeScalar:
		case kAudioHardwarePropertyBootChimeVolumeDecibels:
		case kAudioHardwarePropertyBootChimeVolumeRangeDecibels:
		case kAudioHardwarePropertyBootChimeVolumeScalarToDecibels:
		case kAudioHardwarePropertyBootChimeVolumeDecibelsToScalar:

		//  AudioDevice Properties
		case kAudioDevicePropertyJackIsConnected:
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeDecibels:
		case kAudioDevicePropertyVolumeRangeDecibels:
		case kAudioDevicePropertyVolumeScalarToDecibels:
		case kAudioDevicePropertyVolumeDecibelsToScalar:
		case kAudioDevicePropertyStereoPan:
		case kAudioDevicePropertyStereoPanChannels:
		case kAudioDevicePropertyMute:
		case kAudioDevicePropertySolo:
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
		case kAudioDevicePropertyDataSourceNameForIDCFString:
		case kAudioDevicePropertyClockSource:
		case kAudioDevicePropertyClockSources:
		case kAudioDevicePropertyClockSourceNameForIDCFString:
		case kAudioDevicePropertyClockSourceKindForID:
		case kAudioDevicePropertyPlayThru:
		case kAudioDevicePropertyPlayThruSolo:
		case kAudioDevicePropertyPlayThruVolumeScalar:
		case kAudioDevicePropertyPlayThruVolumeDecibels:
		case kAudioDevicePropertyPlayThruVolumeRangeDecibels:
		case kAudioDevicePropertyPlayThruVolumeScalarToDecibels:
		case kAudioDevicePropertyPlayThruVolumeDecibelsToScalar:
		case kAudioDevicePropertyPlayThruStereoPan:
		case kAudioDevicePropertyPlayThruStereoPanChannels:
		case kAudioDevicePropertyPlayThruDestination:
		case kAudioDevicePropertyPlayThruDestinations:
		case kAudioDevicePropertyPlayThruDestinationNameForIDCFString:
		case kAudioDevicePropertyChannelNominalLineLevel:
		case kAudioDevicePropertyChannelNominalLineLevels:
		case kAudioDevicePropertyChannelNominalLineLevelNameForIDCFString:
		case kAudioDevicePropertyDriverShouldOwniSub:
		case kAudioDevicePropertySubVolumeScalar:
		case kAudioDevicePropertySubVolumeDecibels:
		case kAudioDevicePropertySubVolumeRangeDecibels:
		case kAudioDevicePropertySubVolumeScalarToDecibels:
		case kAudioDevicePropertySubVolumeDecibelsToScalar:
		case kAudioDevicePropertySubMute:
		case kAudioDevicePropertyDataSourceNameForID:
		case kAudioDevicePropertyClockSourceNameForID:
		case kAudioDevicePropertyPlayThruDestinationNameForID:
		case kAudioDevicePropertyChannelNominalLineLevelNameForID:
			return true;
	};

	return false;
}
