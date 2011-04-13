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

#ifndef PA_DEVICE_BACKEND_H
#define PA_DEVICE_BACKEND_H

#include <CarbonCore/Multiprocessing.h>
#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

class PA_Device;

class PA_DeviceBackend
{
private:
	PA_Device *				device;

	char					procname[MAXCOMLEN+1];
	char *					connectHost;
	
	pa_stream *				PARecordStream;
	pa_stream *				PAPlaybackStream;
	
	unsigned char *				inputBuffer;
	unsigned char *				outputBuffer;
	
	MPSemaphoreID				PAContextSemaphore;
	pa_buffer_attr				bufAttr;
	pa_sample_spec				sampleSpec;
	
	UInt64					framesPlayed;

public:
	pa_threaded_mainloop *			PAMainLoop;
	pa_context *				PAContext;

	void	ContextStateCallback();
	void	StreamStartedCallback(pa_stream *stream);
	
	void	StreamReadCallback(pa_stream *stream, size_t nbytes);
	void	StreamWriteCallback(pa_stream *stream, size_t nbytes);
	void	StreamOverflowCallback(pa_stream *stream);
	void	StreamUnderflowCallback(pa_stream *stream);	
	void	StreamEventCallback(pa_stream *stream, const char *name, pa_proplist *pl);
	void	StreamBufferAttrCallback(pa_stream *stream);

private:
	UInt32		CallIOProcs(size_t nbytes, CFArrayRef ioProcList);
	int		ConstructProcessName();
public:
	CFStringRef	GetProcessName();

public:
	PA_DeviceBackend(PA_Device *inDevice);
	~PA_DeviceBackend();

	void Initialize();
	void Teardown();
	
	void Connect();
	void Disconnect();
	void Reconnect();
	void SetHostName(CFStringRef inHost);

	UInt32 GetConnectionStatus();
	UInt32 GetFrameSize();
	
	void ChangeStreamVolume(UInt32 index, Float32 ival);
	void ChangeStreamMute(UInt32 index, Boolean mute);

	Boolean isRunning();
};

#endif // PA_DEVICE_BACKEND_H
