#include <sys/sysctl.h>

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

#include <IOKit/audio/IOAudioTypes.h>
#include <CarbonCore/DriverServices.h>
#include <CarbonCore/Math64.h>

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

#pragma mark ### Logging ###

#if	CoreAudio_Debug
	#define Log_ControlLife				1
	#define	Log_HardwareStartStop			1
	#define	Log_HardwareNotifications		1
	#define	Log_InterestNotification		1
#endif

#define PA_BUFFER_SIZE	(1024 * 256)

#pragma mark ### static wrappers ###

static void staticContextStateCallback(pa_context *, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);
	dev->ContextStateCallback();
}

static void staticStreamStartedCallback(pa_stream *stream, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);	
	dev->StreamStartedCallback(stream);
}

static void staticDeviceWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);
	dev->DeviceWriteCallback(stream, nbytes);
}

static void staticDeviceReadCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);
	dev->DeviceReadCallback(stream, nbytes);
}

static void staticStreamOverflowCallback(pa_stream *stream, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);
	dev->StreamOverflowCallback(stream);	
}

static void staticStreamUnderflowCallback(pa_stream *stream, void *userdata)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(userdata);
	dev->StreamUnderflowCallback(stream);	
}

static void staticScanDevices(CFNotificationCenterRef /* center */,
			      void *observer,
			      CFStringRef /* name */,
			      const void * /* object */,
			      CFDictionaryRef /* userInfo */)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(observer);
	if (dev->PAContext)
		dev->AnnounceDevice();
}

static void staticSetConfig(CFNotificationCenterRef /* center */,
			    void *observer,
			    CFStringRef /* name */,
			    const void * /* object */,
			    CFDictionaryRef userInfo)
{
	CFNumberRef n = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("pid"));
	pid_t pid;
	
	CFNumberGetValue(n, kCFNumberIntType, &pid);
	if (pid == getpid()) {
		PAHP_Device *dev = static_cast<PAHP_Device *>(observer);
		dev->SetConfig(userInfo);
	}
}

static void staticStreamVolumeChanged(CFNotificationCenterRef /* center */,
				      void *observer,
				      CFStringRef name,
				      const void * /* object */,
				      CFDictionaryRef userInfo)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(observer);
	dev->StreamVolumeChanged(name, userInfo);
}

static void staticStreamMuteChanged(CFNotificationCenterRef /* center */,
				    void *observer,
				    CFStringRef name,
				    const void * /* object */,
				    CFDictionaryRef userInfo)
{
	PAHP_Device *dev = static_cast<PAHP_Device *>(observer);
	dev->StreamMuteChanged(name, userInfo);
}

static void staticDebugTimerCallback (CFRunLoopTimerRef /* timer */,
				      void *info)
{
	PAHP_Device *dev = static_cast<PAHP_Device *> (info);
	dev->debugTimer();
}

#pragma mark ### PulseAudio related ###

void
PAHP_Device::debugTimer()
{
	printf("== DIFF %lld   actualSampleRate %f rate %f underrunCount %llu\n",
	       DiffAudioPositions(&playbackBufferWritePos, &playbackBufferReadPos),
	       actualSampleRate, rateScalar, underrunCount);
}

int
PAHP_Device::GetProcessName()
{
        int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
        unsigned int nnames = sizeof(name) / sizeof(name[0]) - 1;
        struct kinfo_proc *result;
	unsigned int i;
        size_t length = 0;
        int err;
	
	snprintf(procname, sizeof(procname), "UNKNOWN");

        err = sysctl(name, nnames, NULL, &length, NULL, 0);
        if (err < 0)
                return err;
	
        result = (kinfo_proc *) malloc(length);
        if (!result)
                return -ENOMEM;
	
        err = sysctl(name, nnames, result, &length, NULL, 0);
	if (err < 0) {
		free(result);
		return err;
	}

        for (i = 0; i < length / sizeof(*result); i++)
                if (result[i].kp_proc.p_pid == getpid())
                        strncpy(procname, result[i].kp_proc.p_comm, sizeof(procname));
	
        free(result);
	
        return 0;
}

void
PAHP_Device::AnnounceDevice()
{
	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef number;
	CFStringRef str;

	UInt32 n = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);

	str = CFStringCreateWithCString(NULL, procname, kCFStringEncodingASCII);
	CFDictionarySetValue(userInfo, CFSTR("procname"), str);
	CFRelease(str);

	str = CopyDeviceName();
	CFDictionarySetValue(userInfo, CFSTR("audioDevice"), str);
	CFRelease(str);
	
	n = GetIOBufferFrameSize();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("IOBufferFrameSize"), number);
	CFRelease(number);
	
	CFDictionarySetValue(userInfo, CFSTR("serverName"), CFSTR("localhost"));

	pa_threaded_mainloop_lock(PAMainLoop);
	n = pa_context_get_state(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("connectionStatus"), number);
	CFRelease(number);
	
	
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("announceDevice"),
					     CFSTR("PAHP_Device"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

void
PAHP_Device::UnannounceDevice()
{
	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef number;
	CFStringRef str;
	
	UInt32 pid = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &pid);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("unannounceDevice"),
					     CFSTR("PAHP_Device"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

void
PAHP_Device::SetConfig(CFDictionaryRef config)
{
	CFStringRef server = (CFStringRef) CFDictionaryGetValue(config, CFSTR("serverName"));
	char buf[128];
	
	CFStringGetCString(server, buf, sizeof(buf), kCFStringEncodingASCII);	
	printf("REQUEST to connect to %s\n", buf);
}

void
PAHP_Device::IncAudioPosition(AudioPosition *a, UInt32 bytes)
{
	a->mutex->Lock();
	a->bytePos += bytes;
	a->cycleCount += a->bytePos / PA_BUFFER_SIZE;
	a->bytePos %= PA_BUFFER_SIZE;
	a->mutex->Unlock();
}

// returns > 0  if   A > B
// returns < 0  if   A < B
// returns   0  if  A == B
SInt64
PAHP_Device::DiffAudioPositions(const AudioPosition *a,
				const AudioPosition *b) const
{
	SInt64 diff;

	a->mutex->Lock();
	b->mutex->Lock();
	
	if (!b)
		diff = (a->cycleCount * PA_BUFFER_SIZE) + a->bytePos;
	else
		diff = ((SInt64) ((SInt64) a->cycleCount - (SInt64) b->cycleCount) * PA_BUFFER_SIZE) +
			(SInt64) ((SInt64) a->bytePos - (SInt64) b->bytePos);

	a->mutex->Unlock();
	b->mutex->Unlock();

	return diff;
}

void
PAHP_Device::ZeroAudioPosition(AudioPosition *p)
{
	p->bytePos = p->cycleCount = 0;
}

Float64
PAHP_Device::UpdateKalman(Float64 value)
{
	k1.x_next = k1.x;

	k1.P_next = k1.P + k1.Q;
	k1.K = k1.P_next / (k1.P_next + k1.R);
	k1.x = k1.x_next + k1.K * (value - k1.x_next);
	k1.P = (1 - k1.K) * k1.P_next;
	
	return k1.x;
}

void
PAHP_Device::InitKalman(Float64 startValue)
{
        k1.x = startValue;
        k1.P = 1.0;
	k1.P_next = 0.0;
        k1.Q = 1.0 / 100.0;
        k1.R = 0.1 * 0.1;
	k1.x_next = 0.0;
	k1.K = 0.0;
}

void
PAHP_Device::UpdateTiming()
{
	//
	// This is where we do our timing adoptions.
	// The CoreAudio part of this driver expects us to report the approximate sample rate
	// of the PulseAudio part by updating 'actualSampleRate' and 'rateScalar' (which is
	// nothing more than a factor to translate between the nominal and the actual sample
	// frame rate.
	//

	const pa_sample_spec *spec = pa_stream_get_sample_spec(PAPlaybackStream);
	size_t frameSize = pa_frame_size(spec);
	
	// First, let's check if PulseAudio would provide us any stream timing information

#if 0
	const pa_timing_info *timingInfo = pa_stream_get_timing_info(PAPlaybackStream);

	if (timingInfo) {
		if (lastTimingInfo.timestamp.tv_sec != 0) {

			// We got timing information, and we've seen one before.
			// So we can now look at the delta and see how many frames we consumed over time.

			UInt64 timeA = (timingInfo->timestamp.tv_sec * USEC_PER_SEC) +
					timingInfo->timestamp.tv_usec;
			UInt64 timeB = (lastTimingInfo.timestamp.tv_sec * USEC_PER_SEC) +
					lastTimingInfo.timestamp.tv_usec;
			UInt64 timePassed = timeA - timeB;
			UInt64 framesReceived = (timingInfo->write_index -
						 lastTimingInfo.write_index) / frameSize;

			if (timePassed && framesReceived) {
				// The calculated framerate from PulseAudio goes to pulseRate ...
				pulseRate = ((Float64) framesReceived * USEC_PER_SEC) / ((Float64) timePassed);
				printf("NEW PULSE RATE %f\n", pulseRate);
				memcpy(&lastTimingInfo, timingInfo, sizeof(lastTimingInfo));
			}
		} else {
			// Never seen a timing information before, just copy ...
			memcpy(&lastTimingInfo, timingInfo, sizeof(lastTimingInfo));
		}
	}

#endif

	// Taking into account the number of counted underrun bytes and the current diff between the two
	// playbackBuffer pointers, we can calculate how many sample frames we have to compensate.
	// Positive numbers of 'frames' means that we need to slow down the CoreAudio IOProc,
	// Negative numbers mean we're getting too less data.
	SInt64 frames;
	
	frames = DiffAudioPositions(&playbackBufferWritePos, &playbackBufferReadPos) - underrunCount;
	frames /= frameSize;
	underrunCount = 0;

	UInt64 target = 10000;
	SInt64 diff = (SInt64) frames - target; // frames = 90000 -> diff = -10000
	Float64 foo = diff / 10.0;
	
	//printf(" diff %lld ... -> %f\n", diff, foo);
	
	actualSampleRate -= foo;
	
	
	/*
	
	// weight in the information we calculated from PulseAudio
	Float64 newVal = actualSampleRate; // + ((pulseRate - actualSampleRate) / 10.0);

	newVal -= driftCompensation;
	//printf("pulserate %16f newVal %16f  frames %lld\n", pulseRate, newVal, frames);
	*/
	
	//actualSampleRate = newVal; //UpdateKalman(newVal);	
	rateScalar = actualSampleRate / 44100.0; //GetCurrentNominalSampleRate();
	
	driftCompensation /= 4.0;
}

Float64
PAHP_Device::GetCurrentActualSampleRate() const
{
	return actualSampleRate;
}

void
PAHP_Device::DeviceWriteCallback(pa_stream *stream, size_t nbytes)
{
	Assert(stream == PAPlaybackStream, "bogus stream pointer in DeviceWriteCallback");

	if (!ioproc)
		return;
	
	Float64 usecPerFrame = 1000000.0 / 44100.0;
	
	AudioBufferList inputList, outputList;

	memset(&inputList, 0, sizeof(inputList));
	memset(&outputList, 0, sizeof(outputList));

	unsigned int fs = GetIOBufferFrameSize() * 8;
	unsigned char *buf, *outputBufferPos;
	
	//pa_stream_begin_write(stream, (void **) &buf, &nbytes);
	buf = outputBuffer;
	outputBufferPos = buf;
	size_t count = nbytes;
	
	memset(buf, 0, nbytes);
	
	AudioTimeStamp now, inputTime, outputTime;
	memset(&now, 0, sizeof(AudioTimeStamp));
	memset(&inputTime, 0, sizeof(AudioTimeStamp));
	memset(&outputTime, 0, sizeof(AudioTimeStamp));

	now.mRateScalar = inputTime.mRateScalar = outputTime.mRateScalar = 1.0;
	now.mHostTime =  inputTime.mHostTime = outputTime.mHostTime = CAHostTimeBase::GetCurrentTime();
	now.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;

	inputTime.mFlags = now.mFlags | kAudioTimeStampSampleTimeValid;
	outputTime.mFlags = now.mFlags | kAudioTimeStampSampleTimeValid;
	
	while (count >= fs) {
		outputList.mBuffers[0].mNumberChannels = 2;
		outputList.mBuffers[0].mDataByteSize = fs;
		outputList.mBuffers[0].mData = outputBufferPos;
		outputList.mNumberBuffers = 1;
		
		inputList.mBuffers[0].mNumberChannels = 2;
		inputList.mBuffers[0].mDataByteSize = fs;
		inputList.mBuffers[0].mData = inputBuffer;
		inputList.mNumberBuffers = 1;

		inputTime.mSampleTime = (framesPlayed * usecPerFrame) - 10000;
		outputTime.mSampleTime = (framesPlayed * usecPerFrame) + 10000;

		//printf(" >>> %d bytes @%p\n", fs, buf);
		ioproc(deviceID, &now,
		       &inputList, &inputTime,
		       &outputList, &outputTime,
		       ioprocClientData);
		//printf(" <<< %d bytes @%p\n", (int) outputList.mBuffers[0].mDataByteSize, outputList.mBuffers[0].mData);
		
		count -= fs;
		outputBufferPos += fs;
		framesPlayed += fs/8;
	}

	pa_stream_write(stream, buf, outputBufferPos - buf, NULL, 0, (pa_seek_mode_t) 0);

#if 0
	SInt64 gap = DiffAudioPositions(&playbackBufferWritePos, &playbackBufferReadPos);

	if ((gap < 0) || ((UInt64) gap < nbytes)) {
		char silence[8192];
		size_t rest = nbytes;

		memset(silence, 0, sizeof(silence));

		//printf("streaming silence for %d bytes\n", (int) nbytes);

		while (rest) {
			int n = MIN(sizeof(silence), rest);
			pa_stream_write(stream, silence, n, NULL, 0, (pa_seek_mode_t) 0);
			rest -= n;			
		}

		underrunCount += nbytes;
		UpdateTiming();

		return;
	}

	if (playbackBufferReadPos.bytePos + nbytes >= PA_BUFFER_SIZE) {
		/* wrap case */
		UInt32 slice = PA_BUFFER_SIZE - playbackBufferReadPos.bytePos;
		pa_stream_write(stream, outputBuffer + playbackBufferReadPos.bytePos, slice, NULL, 0, (pa_seek_mode_t) 0);
		pa_stream_write(stream, outputBuffer, nbytes - slice, NULL, 0, (pa_seek_mode_t) 0);
	} else
		pa_stream_write(stream, outputBuffer + playbackBufferReadPos.bytePos, nbytes, NULL, 0, (pa_seek_mode_t) 0);

	IncAudioPosition(&playbackBufferReadPos, nbytes);
	UpdateTiming();
#endif
}

void
PAHP_Device::DeviceReadCallback(pa_stream *stream, size_t nbytes)
{
	Assert(stream == PARecordStream, "bogus stream pointer in DeviceReadCallback");

	pa_stream_drop(stream);

#if 0
	const unsigned char *buf;

	pa_stream_peek(stream, (const void **) &buf, &nbytes);

	if (recordBufferReadPos.bytePos + nbytes >= PA_BUFFER_SIZE) {
		/* wrap case */
		
		UInt32 lengthA = PA_BUFFER_SIZE - recordBufferReadPos.bytePos;
		UInt32 lengthB = nbytes - lengthA;

		memcpy(inputBuffer + recordBufferReadPos.bytePos, buf, lengthA);
		memcpy(inputBuffer, buf + lengthA, lengthB);
	} else
		memcpy(inputBuffer, buf, nbytes);

	pa_stream_drop(stream);
	IncAudioPosition(&recordBufferReadPos, nbytes);
#endif
}

void
PAHP_Device::StreamOverflowCallback(pa_stream *stream)
{
	printf("%s() %s\n", __func__, stream == PARecordStream ? "input" : "output");
}

void
PAHP_Device::StreamUnderflowCallback(pa_stream *stream)
{
	printf("%s() %s\n", __func__, stream == PARecordStream ? "input" : "output");
}

void
PAHP_Device::ContextStateCallback()
{
	switch (pa_context_get_state(PAContext)) {
		case PA_CONTEXT_READY:
			{
				printf("%s(): Connection ready.\n", __func__);
				pa_sample_spec ss;
				
				ss.format = PA_SAMPLE_FLOAT32;
				ss.rate = 44100;
				ss.channels = 2;
				
				buf_attr.tlength = 8192;
				buf_attr.maxlength = -1;
				buf_attr.minreq = -1;
				buf_attr.prebuf = -1;
				
				char tmp[sizeof(procname) + 10];
				
				snprintf(tmp, sizeof(tmp), "%s playback", procname);
				PAPlaybackStream = pa_stream_new(PAContext, tmp, &ss, NULL);
				pa_stream_set_write_callback(PAPlaybackStream, staticDeviceWriteCallback, this);
				pa_stream_set_overflow_callback(PAPlaybackStream, staticStreamOverflowCallback, this);
				pa_stream_set_underflow_callback(PAPlaybackStream, staticStreamUnderflowCallback, this);
				pa_stream_set_started_callback(PAPlaybackStream, staticStreamStartedCallback, this);
				pa_stream_connect_playback(PAPlaybackStream, NULL, &buf_attr,
							   (pa_stream_flags_t)  (PA_STREAM_INTERPOLATE_TIMING |
										 PA_STREAM_AUTO_TIMING_UPDATE),
							   NULL, NULL);

				snprintf(tmp, sizeof(tmp), "%s record", procname);
				PARecordStream = pa_stream_new(PAContext, tmp, &ss, NULL);
				pa_stream_set_read_callback(PAPlaybackStream, staticDeviceReadCallback, this);
				pa_stream_set_overflow_callback(PARecordStream, staticStreamOverflowCallback, this);
				pa_stream_set_underflow_callback(PARecordStream, staticStreamUnderflowCallback, this);
				pa_stream_connect_record(PARecordStream, NULL, &buf_attr, (pa_stream_flags_t) 0);

				PAConnected = true;
				MPSignalSemaphore(PAContextSemaphore);
			}
			break;
		case PA_CONTEXT_TERMINATED:
			printf("%s(): Connection terminated.\n", __func__);
			PAConnected = false;
			MPSignalSemaphore(PAContextSemaphore);
			break;
		case PA_CONTEXT_FAILED:
			printf("%s(): Connection failed.\n", __func__);
			PAConnected = false;
			MPSignalSemaphore(PAContextSemaphore);
			break;
		default:
			PAConnected = false;
			break;
	}
}

void
PAHP_Device::StreamStartedCallback(pa_stream *stream)
{
	if (stream == PAPlaybackStream) {
		playbackBufferReadPos.cycleCount = playbackBufferWritePos.cycleCount;
		playbackBufferReadPos.bytePos = playbackBufferReadPos.bytePos;
	} else {
		recordBufferReadPos.cycleCount = recordBufferWritePos.cycleCount;
		recordBufferReadPos.bytePos = recordBufferWritePos.bytePos;
	}
}

void
PAHP_Device::StreamVolumeChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	Float32 value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberFloatType, &value);
	
	if (!PAContext)
		return;
	
	int ival = value * 0x10000;
	
	pa_cvolume volume;
	volume.channels = 2;
	volume.values[0] = ival;
	volume.values[1] = ival;
	
	//printf("%s() %f -> %05x!\n", __func__, value, ival);
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_sink_volume_by_index(PAContext, 0, &volume, NULL, NULL);
	pa_threaded_mainloop_unlock(PAMainLoop);
}

void
PAHP_Device::StreamMuteChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	bool value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberIntType, &value);
	
	if (!PAContext)
		return;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_sink_mute_by_index(PAContext, 0, value, NULL, NULL);
	pa_threaded_mainloop_unlock(PAMainLoop);
}

#pragma mark ### Construct / Deconstruct ###

PAHP_Device::PAHP_Device(AudioDeviceID	 inAudioDeviceID,
			 PAHP_PlugIn	*inPlugIn) :
	HP_Device(inAudioDeviceID, kAudioDeviceClassID, inPlugIn, 1, false),
	plugin(inPlugIn),
	hogMode(NULL),
	IOThread(NULL),
	anchorHostTime(0),
	controlsInitialized(false),
	controlProperty(NULL),
	PAContext(NULL),
	deviceID(inAudioDeviceID)
{
}

PAHP_Device::~PAHP_Device()
{
}

void
PAHP_Device::Initialize()
{
	HP_Device::Initialize();

	recordBufferReadPos.mutex = new CAMutex("recordBufferReadPos");
	recordBufferWritePos.mutex = new CAMutex("recordBufferWritePos");
	playbackBufferReadPos.mutex = new CAMutex("playbackBufferReadPos");
	playbackBufferWritePos.mutex = new CAMutex("playbackBufferWritePos");

	PAMainLoop = NULL;
	
	int ret = GetProcessName();
	if (ret < 0)
		printf("Unable to get process name!? ret = %d\n", ret);

	ret = MPCreateSemaphore(UINT_MAX, 0, &PAContextSemaphore);
	if (ret != 0)
		printf("MPCreateSemaphore() failed\n");

	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticScanDevices,
					CFSTR("scanDevices"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);

	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticSetConfig,
					CFSTR("setConfiguration"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);

	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticStreamVolumeChanged,
					CFSTR("updateStreamVolume"),
					CFSTR("PAHP_LevelControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);

	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticStreamMuteChanged,
					CFSTR("updateMuteVolume"),
					CFSTR("PAHP_BooleanControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);	

	CFRunLoopTimerContext context;
	memset(&context, 0, sizeof(context));
	context.info = this;

	timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 0, 1, 0, 0, staticDebugTimerCallback, &context);
	CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes);
	
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
	
	inputBuffer = (unsigned char *) pa_xmalloc0(PA_BUFFER_SIZE);
	outputBuffer = (unsigned char *) pa_xmalloc0(PA_BUFFER_SIZE);
	
	hogMode = new HP_HogMode(this);
	hogMode->Initialize();

	IOThread = new HP_IOThread(this);
}

void
PAHP_Device::Teardown()
{
	CFRunLoopTimerInvalidate(timer);
	
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
	
	if (inputBuffer) {
		pa_xfree(inputBuffer);
		inputBuffer = NULL;
	}
	
	if (outputBuffer) {
		pa_xfree(outputBuffer);
		outputBuffer = NULL;
	}
	
	delete recordBufferReadPos.mutex;
	delete recordBufferWritePos.mutex;
	delete playbackBufferReadPos.mutex;
	delete playbackBufferWritePos.mutex;

	MPDeleteSemaphore(PAContextSemaphore);
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

#pragma mark ### Name strings ###

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
	CFStringRef str = CFSTR("pulseaudio.org");
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

#pragma mark ### Hog Mode ###

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
	if (doUnlockIOThreadGuard)
		IOThread->GetIOGuard().Unlock();

	if (doUnlockDeviceStateGuard)
		GetDeviceStateMutex().Unlock();
}

#pragma mark ### Properties ###

UInt32
PAHP_Device::GetTransportType() const
{
	return kIOAudioDeviceTransportTypeNetwork;
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

		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
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

		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
			return sizeof(UInt32);

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

		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
			*(static_cast<UInt32*>(outData)) = 1;
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
	// take and hold the state mutex
	CAMutex::Locker stateMutex(GetDeviceStateMutex());

	// create the controls if necessary
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
	}
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

#pragma mark ### IOProc ###

void
PAHP_Device::AddIOProc(AudioDeviceIOProc inProc, void* inClientData)
{
	printf("%s() .. inProc %p\n", __func__, inProc);
	ioproc = inProc;
	ioprocClientData = inClientData;
	
	HP_Device::AddIOProc(inProc, inClientData);
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
#if 0
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
			//IOProc->Call(inCurrentTime, inInputTime, inputBufferList, inOutputTime, NULL);

			// pre-process it before handing it to the hardware
			PreProcessOutputData(inOutputTime, *IOProc);
		}

		// mark the telemetry
		mIOCycleTelemetry->IOCycleIOProcsEnd(GetIOCycleNumber());

		// write the output data
		if (HasOutputStreams())
			hardwareIOSucceeded = WriteOutputData(inOutputTime, GetIOBufferSetID());
	}
#endif

	FinishIOCycle();

	return hardwareIOSucceeded;
}

void
PAHP_Device::StartIOEngine()
{
	HP_Stream *stream = GetStreamByIndex(true, 0);	
	actualSampleRate = 44100.0; //stream->GetCurrentNominalSampleRate();
	pulseRate = actualSampleRate;
	rateScalar = 1.0;
	underrunCount = 0;
	driftCompensation = 0.0;
	InitKalman(actualSampleRate);

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

	// PulseAudio
	
	framesPlayed = 0;

	ZeroAudioPosition(&recordBufferReadPos);
	ZeroAudioPosition(&recordBufferWritePos);
	ZeroAudioPosition(&playbackBufferReadPos);
	ZeroAudioPosition(&playbackBufferWritePos);

	if (PAContext)
		return;
	
	PAMainLoop = pa_threaded_mainloop_new();
	Assert(PAMainLoop, "pa_threaded_mainloop_new() failed");
	pa_mainloop_api *api = pa_threaded_mainloop_get_api(PAMainLoop);
	Assert(api, "pa_threaded_mainloop_get_api() failed");

	PAContext = pa_context_new(api, procname);
	Assert(PAContext, "pa_context_new() failed");

	pa_context_set_state_callback(PAContext, staticContextStateCallback, this);
	pa_context_connect(PAContext, "192.168.2.5", 
			   (pa_context_flags_t) (PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN),
			   NULL);

	int ret = pa_threaded_mainloop_start(PAMainLoop);
	Assert(ret >= 0, "pa_threaded_mainloop_start() failed\n");

	printf("%s(): WAITING\n", __func__);
	MPWaitOnSemaphore(PAContextSemaphore, kDurationForever);
	printf("%s(): DONE\n", __func__);
	
	AnnounceDevice();
}

void
PAHP_Device::StopHardware()
{
	#if Log_HardwareStartStop
		DebugMessage("PAHP_Device::StopHardware: stopping the hardware");
	#endif

	UnannounceDevice();
	
	if (!PAMainLoop)
		return;

	pa_threaded_mainloop_lock(PAMainLoop);

	if (PAPlaybackStream) {
		pa_stream_flush(PAPlaybackStream, NULL, NULL);
		pa_stream_disconnect(PAPlaybackStream);
		pa_stream_unref(PAPlaybackStream);
		PAPlaybackStream = NULL;
	}

	if (PARecordStream) {
		pa_stream_disconnect(PARecordStream);
		pa_stream_unref(PARecordStream);
		PARecordStream = NULL;
	}

	if (PAContext) {
		pa_context_disconnect(PAContext);
		pa_threaded_mainloop_unlock(PAMainLoop);

		printf("%s(): WAITING\n", __func__);
		MPWaitOnSemaphore(PAContextSemaphore, kDurationForever);
		printf("%s(): DONE\n", __func__);

		pa_threaded_mainloop_lock(PAMainLoop);
		pa_context_unref(PAContext);
		PAContext = NULL;
	}

	pa_threaded_mainloop *loop = PAMainLoop;
	PAMainLoop = NULL;

	pa_threaded_mainloop_unlock(loop);
	pa_threaded_mainloop_stop(loop);
	pa_threaded_mainloop_free(loop);	
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
PAHP_Device::ReadInputData(const AudioTimeStamp& /*inStartTime*/, UInt32 /* inBufferSetID */)
{
	// this method is called to read the input data
	// it returns true if the read completed successfully

	AudioBufferList *inputBufferList = mIOProcList->GetSharedAudioBufferList(true);

	if (!inputBufferList || !PARecordStream || !PAConnected || !inputBuffer)
		return false;

	for (UInt32 b = 0; b < inputBufferList->mNumberBuffers; b++) {
		AudioBuffer *buffer = inputBufferList->mBuffers + b;
		
		if (!buffer)
			continue;
		
		unsigned char *data = (unsigned char *) buffer->mData;
		size_t size = buffer->mDataByteSize;
		
		if ((data == NULL) || (size == 0))
			continue;

		if (recordBufferReadPos.bytePos + size > PA_BUFFER_SIZE) {
			// wrap case
			UInt32 slice = PA_BUFFER_SIZE - recordBufferReadPos.bytePos;
			memcpy(inputBuffer + recordBufferReadPos.bytePos, data, slice);
			memcpy(inputBuffer, data + slice, size - slice);
		} else
			memcpy(inputBuffer + recordBufferReadPos.bytePos, data, size);
		
		IncAudioPosition(&recordBufferReadPos, size);
	}

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
PAHP_Device::PreProcessOutputData(const AudioTimeStamp& /*inOuputTime*/, HP_IOProc &inIOProc)
{
	if (!inIOProc.BufferListHasData(false))
		return;

	// this method is called just after getting the data from the IOProc but before writing it to the hardware
	if (mIOCycleTelemetry->IsCapturing())
		mIOCycleTelemetry->OutputDataPresent(GetIOCycleNumber());

	if (!PAPlaybackStream || !PAConnected)
		return;
	
	AudioBufferList *outputBufferList = inIOProc.GetAudioBufferList(false);

	for (UInt32 b = 0; b < outputBufferList->mNumberBuffers; b++) {
		AudioBuffer *buffer = outputBufferList->mBuffers + b;
		
		if (!buffer)
			continue;

		unsigned char *data = (unsigned char *) buffer->mData;
		size_t size = buffer->mDataByteSize;

		if ((data == NULL) || (size == 0))
			continue;

		if (playbackBufferWritePos.bytePos + size > PA_BUFFER_SIZE) {
			// wrap case
			UInt32 slice = PA_BUFFER_SIZE - playbackBufferWritePos.bytePos;
			memcpy(outputBuffer + playbackBufferWritePos.bytePos, data, slice);
			memcpy(outputBuffer, data + slice, size - slice);
		} else
			memcpy(outputBuffer + playbackBufferWritePos.bytePos, data, size);

		IncAudioPosition(&playbackBufferWritePos, size);
	}
}

bool
PAHP_Device::WriteOutputData(const AudioTimeStamp& /*inStartTime*/, UInt32 /* inBufferSetID */)
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


#pragma mark ### Timing related ###


static inline UInt64
TimeValToAbsolute(const struct timeval *tv)
{
	UInt64 nanos = (tv->tv_usec * 1000ULL) +
			(tv->tv_sec * 1000000000ULL);
	return UnsignedWideToUInt64(NanosecondsToAbsolute(UInt64ToUnsignedWide(nanos)));
}

void
PAHP_Device::GetCurrentTime(AudioTimeStamp &outTime)
{
	ThrowIf(!IsIOEngineRunning(),
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::GetCurrentTime: can't because the engine isn't running");

	outTime = CAAudioTimeStamp::kZero;

#if 1
	
	// compute the host ticks pere frame
	Float64 actualHostTicksPerFrame = CAHostTimeBase::GetFrequency() / actualSampleRate;

	Float64 sampleOffset = 0.0;	

	outTime.mHostTime = CAHostTimeBase::GetTheCurrentTime();

	if(outTime.mHostTime >= anchorHostTime) {
		sampleOffset = outTime.mHostTime - anchorHostTime;
	} else {
		sampleOffset = anchorHostTime - outTime.mHostTime;
		sampleOffset *= -1.0;
	}

	sampleOffset /= actualHostTicksPerFrame;
	sampleOffset = floor(sampleOffset);
	outTime.mSampleTime = sampleOffset;
	outTime.mRateScalar = rateScalar;
	outTime.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;

	return;

#endif
	
	
#if 0
	/*
	SInt64 safety = 10000;	
	SInt64 delta = DiffAudioPositions(&playbackBufferWritePos,
					  &playbackBufferReadPos) + safety;
	
	Float64 rate;
	rate = 1.0 - (delta / 10000000.0);
	*/
	
	Float64 rate = 1.0;
	
	//printf(" -- delta %8lld  rate %f\n", delta, rate); 


	
	if (!lastTimingInfo)
		return;
	
	struct timeval now, diff;
	gettimeofday(&now, NULL);

	timersub(&now, &lastTimingInfo->timestamp, &diff);

	//outTime.mHostTime = CAHostTimeBase::GetCurrentTime();
	//outTime.mHostTime = TimeValToAbsolute(&diff);

	diff.tv_usec += lastTimingInfo->sink_usec;
	diff.tv_sec  += diff.tv_usec / 1000000;
	diff.tv_usec %= 1000000;

	// calculate how many host ticks away from the anchor time stamp the current host time is
	Float64 sampleOffset = 0.0;	
//	sampleOffset = TimeValToAbsolute(&diff);
	if(outTime.mHostTime >= mAnchorHostTime) {
		sampleOffset = outTime.mHostTime - mAnchorHostTime;
	} else {
		sampleOffset = mAnchorHostTime - outTime.mHostTime;
		sampleOffset *= -1.0;
	}
	
	
//	sampleOffset /= actualHostTicksPerFrame;
	sampleOffset = floor(sampleOffset);
	outTime.mSampleTime = sampleOffset;
	outTime.mRateScalar = 1.0;

	//const pa_timing_info *ti;
/*
	
	UInt64 delta = DiffAudioPositions(&playbackBufferReadPos,
					  &playbackBufferWritePos);
	
	SInt64 safety = 10000;
	if (delta < safety)
		outTime.mRateScalar -= pow(abs(delta), 0.1) / 2;
	else
		outTime.mRateScalar += pow(abs(delta), 0.1) / 2;
	
*/	
	
//	outTime.mRateScalar += ((Float32) delta / (Float32) safety) / 10;


	outTime.mFlags = kAudioTimeStampSampleTimeValid |
			 kAudioTimeStampHostTimeValid	|
			 kAudioTimeStampRateScalarValid;
#endif

}

void
PAHP_Device::SafeGetCurrentTime(AudioTimeStamp &outTime)
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
PAHP_Device::TranslateTime(const AudioTimeStamp &inTime, AudioTimeStamp &outTime)
{
	// the input time stamp has to have at least one of the sample or host time valid
	ThrowIf((inTime.mFlags & kAudioTimeStampSampleHostTimeValid) == 0,
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_Device::TranslateTime: have to have either sample time or host time valid on the input");
	ThrowIf(!IsIOEngineRunning(),
		CAException(kAudioHardwareNotRunningError),
		"PAHP_Device::TranslateTime: can't because the engine isn't running");
	// compute the host ticks pere frame
	Float64 actualHostTicksPerFrame = CAHostTimeBase::GetFrequency() / actualSampleRate;
	
	// calculate the sample time
	Float64 offset = 0.0;
	if (outTime.mFlags & kAudioTimeStampSampleTimeValid) {
		if (inTime.mFlags & kAudioTimeStampSampleTimeValid) {
			// no calculations necessary
			outTime.mSampleTime = inTime.mSampleTime;
		} else if (inTime.mFlags & kAudioTimeStampHostTimeValid) {
			// calculate how many host ticks away from the current 0 time stamp the input host time is
			if (inTime.mHostTime >= anchorHostTime) {
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
	if (outTime.mFlags & kAudioTimeStampHostTimeValid) {
		if (inTime.mFlags & kAudioTimeStampHostTimeValid) {
			// no calculations necessary
			outTime.mHostTime = inTime.mHostTime;
		} else if (inTime.mFlags & kAudioTimeStampSampleTimeValid) {
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
		outTime.mRateScalar = rateScalar;
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
		} else if (mIOProcList->IsAnythingEnabled()) {
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

		// bump the sample time in the right direction
		if (isInput) {
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

	// now filter it through the hardware, unless told not to
	if (mIOProcList->IsOnlyNULLEnabled() && isConsultingDevice) {
	}

	// assign the return value
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
	memset(&lastTimingInfo, 0, sizeof(lastTimingInfo));
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

#pragma mark ### Streams and Controls

void
PAHP_Device::CreateStreams()
{
	OSStatus	 ret = 0;
	AudioObjectID    newStreamID = 0;
	PAHP_Stream	*stream = NULL;

	std::vector<AudioStreamID> streamIDs;

	// instantiate an AudioStream
	ret = AudioObjectCreate(plugin->GetInterface(), GetObjectID(), kAudioStreamClassID, &newStreamID);
	if (ret == 0) {
		stream = new PAHP_Stream(newStreamID, plugin, this, true, 1);
		stream->Initialize();
		AddStream(stream);
		streamIDs.push_back(newStreamID);
	}

	ret = AudioObjectCreate(plugin->GetInterface(), GetObjectID(), kAudioStreamClassID, &newStreamID);
	if (ret == 0) {
		stream = new PAHP_Stream(newStreamID, plugin, this, false, 1);
		stream->Initialize();
		AddStream(stream);
		streamIDs.push_back(newStreamID);
	}

	// now tell the HAL about the new stream IDs
	if (streamIDs.size() != 0) {
		// set the object state mutexes
		for (std::vector<AudioStreamID>::iterator iterator = streamIDs.begin();
		     iterator != streamIDs.end();
		     std::advance(iterator, 1)) {
			HP_Object *obj = HP_Object::GetObjectByID(*iterator);
			if (obj)
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
	HP_Control *control = NULL;

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
