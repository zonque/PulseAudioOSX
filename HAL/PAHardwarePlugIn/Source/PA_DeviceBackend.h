#ifndef PA_DEVICE_BACKEND_H
#define PA_DEVICE_BACKEND_H

#include <IOKit/IOKitLib.h>
#include <CarbonCore/Multiprocessing.h>
#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

class PA_Device;

class PA_DeviceBackend
{
private:
	PA_Device				*device;

	char					procname[MAXCOMLEN+1];
	
	pa_stream *				PARecordStream;
	pa_stream *				PAPlaybackStream;
	
	unsigned char *				inputBuffer;
	unsigned char *				outputBuffer;
	
	bool					PAConnected;
	MPSemaphoreID				PAContextSemaphore;
	pa_buffer_attr				bufAttr;
	
	UInt64					framesPlayed;

public:
	pa_threaded_mainloop *			PAMainLoop;
	pa_context *				PAContext;

	void	ContextStateCallback();
	void	StreamStartedCallback(pa_stream *stream);
	
	void	DeviceReadCallback(pa_stream *stream, size_t nbytes);
	void	DeviceWriteCallback(pa_stream *stream, size_t nbytes);
	void	StreamOverflowCallback(pa_stream *stream);
	void	StreamUnderflowCallback(pa_stream *stream);	
	void	StreamVolumeChanged(CFStringRef name, CFDictionaryRef userInfo);
	void	StreamMuteChanged(CFStringRef name, CFDictionaryRef userInfo);
	
private:
	int	GetProcessName();
	
	
public:
	PA_DeviceBackend(PA_Device *inDevice);
	~PA_DeviceBackend();

	void Initialize();
	void Teardown();
	
	void Connect();
	void Disconnect();
	void Reconnect();
};

#endif // PA_DEVICE_BACKEND_H