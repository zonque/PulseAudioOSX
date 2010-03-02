/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAENGINE_H
#define PAENGINE_H

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/IOLib.h>

#include "PATimeStampFactory.h"

#include "BuildNames.h"

#define MAX_STREAMS		64

class PAEngine : public IOAudioEngine
{
	OSDeclareDefaultStructors(PAEngine)

public:
	virtual bool				init(struct PAVirtualDevice *info);
	virtual void				free();
	bool						initHardware(IOService *inProvider);
	IOBufferMemoryDescriptor	*audioInBuf, *audioOutBuf;

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

private:
	IOAudioStream				*createNewAudioStream(IOAudioStreamDirection direction, void *sampleBuffer);
	UInt32						channelsIn, channelsOut, nStreams;
	UInt32						currentFrame;

	IOAudioSampleRate			sampleRate;
    IOAudioStream				*audioStream[MAX_STREAMS];
	IOTimerEventSource			*timerEventSource;

	PATimeStampFactory			*timeStampFactory;
};

#endif // PAENGINE_H