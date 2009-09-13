//
//  PAEngine.h
//

#ifndef PAENGINE_H
#define PAENGINE_H

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/IOLib.h>

#define PAEngine org_pulseaudio_ioaudioengine
#define MAX_STREAMS		64

class PAEngine : public IOAudioEngine
{
	OSDeclareDefaultStructors(PAEngine)

public:
	virtual bool				init(UInt32 in, UInt32 out);
	virtual void				free();
	bool						initHardware(IOService *inProvider);
	IOBufferMemoryDescriptor	*audioInBuf, *audioOutBuf;

private:
	IOAudioStream				*createNewAudioStream(IOAudioStreamDirection direction, void *sampleBuffer);


// CoreAudio interface
public:
	virtual OSString			*getGlobalUniqueID();
	virtual IOReturn			performAudioEngineStart();
	virtual IOReturn			performAudioEngineStop();
	virtual UInt32				getCurrentSampleFrame();
	virtual IOReturn			clipOutputSamples(const void *inMixBuffer, void *outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat *inFormat, IOAudioStream *inStream);
	virtual IOReturn			convertInputSamples(const void *inSourceBuffer, void *outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat* inFormat, IOAudioStream* inStream);
	virtual IOReturn			performFormatChange(IOAudioStream *inStream, const IOAudioStreamFormat *inNewFormat, const IOAudioSampleRate *inNewSampleRate);
	void						TimerFired(OSObject* inTarget, IOTimerEventSource* inSender);

protected:
	UInt32						channelsIn, channelsOut, nStreams;
	UInt32						currentFrame;

	IOAudioSampleRate			sampleRate;
    IOAudioStream				*audioStream[MAX_STREAMS];
	IOTimerEventSource			*timerEventSource;
};

#endif // PAENGINE_H