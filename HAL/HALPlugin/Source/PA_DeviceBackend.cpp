/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#define CLASS_NAME "PA_DeviceBackend"

#include <pulse/pulseaudio.h>
#include <sys/sysctl.h>

#include "PA_Device.h"
#include "PA_DeviceBackend.h"

#include "CAHostTimeBase.h"

#define PA_BUFFER_SIZE	(1024 * 256)

#pragma mark ### static wrappers ###

static void staticContextStateCallback(pa_context *, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);
	be->ContextStateCallback();
}

static void staticStreamStartedCallback(pa_stream *stream, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);	
	be->StreamStartedCallback(stream);
}

static void staticDeviceWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);
	be->DeviceWriteCallback(stream, nbytes);
}

static void staticDeviceReadCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);
	be->DeviceReadCallback(stream, nbytes);
}

static void staticStreamOverflowCallback(pa_stream *stream, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);
	be->StreamOverflowCallback(stream);	
}

static void staticStreamUnderflowCallback(pa_stream *stream, void *userdata)
{
	PA_DeviceBackend *be = static_cast<PA_DeviceBackend *>(userdata);
	be->StreamUnderflowCallback(stream);	
}

#pragma mark ### PA_DeviceBackend ###

int
PA_DeviceBackend::ConstructProcessName()
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

CFStringRef
PA_DeviceBackend::GetProcessName()
{
	return CFStringCreateWithCString(device->GetAllocator(), procname, kCFStringEncodingASCII);
}

void
PA_DeviceBackend::DeviceWriteCallback(pa_stream *stream, size_t nbytes)
{
	Assert(stream == PAPlaybackStream, "bogus stream pointer in DeviceWriteCallback");
	
	Float64 usecPerFrame = 1000000.0 / 44100.0;
	
	AudioBufferList inputList, outputList;
	
	memset(&inputList, 0, sizeof(inputList));
	memset(&outputList, 0, sizeof(outputList));
	
	unsigned int fs = device->GetIOBufferFrameSize() * 8;
	unsigned char *buf, *outputBufferPos;
	
	/*
	int ret = pa_stream_begin_write(stream, (void **) &buf, &nbytes);
	if (ret < 0) {
		printf(" XXXXXXXXXXXX ret %d\n", ret);
		return;
	}
	*/
	
	//printf("buf %p, sizeof(void*) %d\n", buf, sizeof(void*));
	
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
	
	CFArrayRef ioProcList = device->LockIOProcList();
	AudioObjectID deviceID = device->GetObjectID();

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
		
		for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
			IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
			Boolean enabled = io->enabled;
			
			if ((io->startTime.mFlags & kAudioTimeStampHostTimeValid) &&
			    (io->startTime.mHostTime > now.mHostTime))
				enabled = false;

			if ((io->startTime.mFlags & kAudioTimeStampSampleTimeValid) &&
			    (io->startTime.mSampleTime > now.mSampleTime))
				enabled = false;
			
			//enabled = false;
			
			if (enabled) {
				io->proc(deviceID, &now,
					 &inputList, &inputTime,
					 &outputList, &outputTime,
					 io->clientData);
			}
		}
		
		count -= fs;
		outputBufferPos += fs;
		framesPlayed += fs/8;
	}
	
	device->UnlockIOProcList();
	
	//for (int xxx = 0; xxx < nbytes; xxx++)
	//	buf[xxx] = rand() % 0xff;
		
	pa_stream_write(stream, buf, outputBufferPos - buf, NULL, 0,
			(pa_seek_mode_t) PA_SEEK_RELATIVE);

	CFRelease(ioProcList);
}

void
PA_DeviceBackend::DeviceReadCallback(pa_stream *stream, size_t nbytes)
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
PA_DeviceBackend::StreamOverflowCallback(pa_stream *stream)
{
	printf("%s() %s\n", __func__, stream == PARecordStream ? "input" : "output");
}

void
PA_DeviceBackend::StreamUnderflowCallback(pa_stream *stream)
{
	printf("%s() %s\n", __func__, stream == PARecordStream ? "input" : "output");
}

void
PA_DeviceBackend::ContextStateCallback()
{
	switch (pa_context_get_state(PAContext)) {
		case PA_CONTEXT_READY:
		{
			printf("%s(): Connection ready.\n", __func__);
			
			char tmp[sizeof(procname) + 10];
			
			snprintf(tmp, sizeof(tmp), "%s playback", procname);
			PAPlaybackStream = pa_stream_new(PAContext, tmp, &sampleSpec, NULL);
			pa_stream_set_write_callback(PAPlaybackStream, staticDeviceWriteCallback, this);
			pa_stream_set_overflow_callback(PAPlaybackStream, staticStreamOverflowCallback, this);
			pa_stream_set_underflow_callback(PAPlaybackStream, staticStreamUnderflowCallback, this);
			pa_stream_set_started_callback(PAPlaybackStream, staticStreamStartedCallback, this);
			pa_stream_connect_playback(PAPlaybackStream, NULL, &bufAttr,
						   (pa_stream_flags_t)  (PA_STREAM_INTERPOLATE_TIMING |
									 PA_STREAM_AUTO_TIMING_UPDATE),
						   NULL, NULL);
			
			/*
			snprintf(tmp, sizeof(tmp), "%s record", procname);
			PARecordStream = pa_stream_new(PAContext, tmp, &sampleSpec, NULL);
			pa_stream_set_read_callback(PAPlaybackStream, staticDeviceReadCallback, this);
			pa_stream_set_overflow_callback(PARecordStream, staticStreamOverflowCallback, this);
			pa_stream_set_underflow_callback(PARecordStream, staticStreamUnderflowCallback, this);
			pa_stream_connect_record(PARecordStream, NULL, &bufAttr, (pa_stream_flags_t) 0);
			*/
			
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
PA_DeviceBackend::StreamStartedCallback(pa_stream * /* stream */)
{
}

void
PA_DeviceBackend::ChangeStreamVolume(UInt32 index, Float32 value)
{
	int ival = value * 0x10000;

	pa_cvolume volume;
	volume.channels = 2;
	volume.values[0] = ival;
	volume.values[1] = ival;
	 
	//printf("%s() %f -> %05x!\n", __func__, value, ival);
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_sink_volume_by_index(PAContext, index, &volume, NULL, NULL);
	pa_threaded_mainloop_unlock(PAMainLoop);
}

void
PA_DeviceBackend::ChangeStreamMute(UInt32 index, Boolean mute)
{
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_sink_mute_by_index(PAContext, index, mute, NULL, NULL);
	pa_threaded_mainloop_unlock(PAMainLoop);
}

Boolean
PA_DeviceBackend::isRunning()
{
	return true;
}

#pragma mark ### Construct/Desconstruct

PA_DeviceBackend::PA_DeviceBackend(PA_Device *inDevice) :
	device(inDevice)
{
}

PA_DeviceBackend::~PA_DeviceBackend()
{
}

void
PA_DeviceBackend::Initialize()
{
	framesPlayed = 0;
	inputBuffer = (unsigned char *) pa_xmalloc0(PA_BUFFER_SIZE);
	outputBuffer = (unsigned char *) pa_xmalloc0(PA_BUFFER_SIZE);
	connectHost = NULL;
	
	sampleSpec.format = PA_SAMPLE_FLOAT32;
	sampleSpec.rate = device->GetSampleRate();
	sampleSpec.channels = 2;
	
	bufAttr.tlength = device->GetIOBufferFrameSize() * pa_frame_size(&sampleSpec);
	bufAttr.maxlength = -1;
	bufAttr.minreq = -1;
	bufAttr.prebuf = -1;
	
	ConstructProcessName();
}

void
PA_DeviceBackend::Teardown()
{
	pa_xfree(inputBuffer);
	pa_xfree(outputBuffer);
	pa_xfree(connectHost);
	inputBuffer = NULL;
	outputBuffer = NULL;
	connectHost = NULL;
}

#pragma mark ### Connect/Disconnect ###

void
PA_DeviceBackend::Connect()
{
	if (PAContext)
		return;
	
	PAMainLoop = pa_threaded_mainloop_new();
	Assert(PAMainLoop, "pa_threaded_mainloop_new() failed");
	pa_mainloop_api *api = pa_threaded_mainloop_get_api(PAMainLoop);
	Assert(api, "pa_threaded_mainloop_get_api() failed");
	
	PAContext = pa_context_new(api, procname);
	Assert(PAContext, "pa_context_new() failed");
	
	pa_context_set_state_callback(PAContext, staticContextStateCallback, this);
	pa_context_connect(PAContext, connectHost,
			   (pa_context_flags_t) (PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN),
			   NULL);
	
	int ret = pa_threaded_mainloop_start(PAMainLoop);
	Assert(ret >= 0, "pa_threaded_mainloop_start() failed\n");
	
	printf("%s(): WAITING\n", __func__);
	MPWaitOnSemaphore(PAContextSemaphore, kDurationForever);
	printf("%s(): DONE\n", __func__);	
}

void
PA_DeviceBackend::SetHostName(CFStringRef inHost)
{
	if (connectHost)
		pa_xfree(connectHost);
	
	CFIndex size = CFStringGetLength(inHost);
	connectHost = (char *) pa_xmalloc0(size);
	if (!connectHost)
		return;
	
	CFStringGetCString(inHost, connectHost, size, kCFStringEncodingASCII);
}

void
PA_DeviceBackend::Disconnect()
{
	
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
PA_DeviceBackend::Reconnect()
{
	Disconnect();
	Connect();
}

UInt32
PA_DeviceBackend::GetConnectionStatus()
{
	UInt32 status;

	pa_threaded_mainloop_lock(PAMainLoop);
	status = pa_context_get_state(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);

	return status;
}

UInt32
PA_DeviceBackend::GetFrameSize()
{
	return pa_frame_size(&sampleSpec);
}
