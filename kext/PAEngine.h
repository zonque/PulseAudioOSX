/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAENGINE_H
#define PAENGINE_H

#include "BuildNames.h"

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/IOTimerEventSource.h>

#include "PADevice.h"
#include "PAUserClientTypes.h"

#define MAX_STREAMS		64

class PAEngine : public IOAudioEngine
{
	OSDeclareDefaultStructors(PAEngine)

public:
	void						free();
	bool						initHardware(IOService *provider);
	bool						setDeviceInfo(struct PAVirtualDevice *info);
	IOBufferMemoryDescriptor	*audioInBuf, *audioOutBuf;

	OSString					*getGlobalUniqueID();
	IOReturn					performAudioEngineStart();
	IOReturn					performAudioEngineStop();
	UInt32						getCurrentSampleFrame();
	IOReturn					clipOutputSamples(const void *inMixBuffer, void *outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat *inFormat, IOAudioStream *inStream);
	IOReturn					convertInputSamples(const void *inSourceBuffer, void *outTargetBuffer, UInt32 inFirstFrame, UInt32 inNumberFrames, const IOAudioStreamFormat* inFormat, IOAudioStream* inStream);
	IOReturn					performFormatChange(IOAudioStream *inStream, const IOAudioStreamFormat *inNewFormat, const IOAudioSampleRate *inNewSampleRate);

	void						getNextTimeStamp(UInt32 inLoopCount, AbsoluteTime* outTimeStamp);
	static void					timerFired(OSObject *inTarget, IOTimerEventSource *inSender);

	UInt32						currentSampleRate;
	void						setNewSampleRate(UInt32 sampleRate);

private:
	IOAudioStream				*createNewAudioStream(IOAudioStreamDirection direction, void *sampleBuffer);
	UInt32						channelsIn, channelsOut, nStreams;
	UInt32						currentFrame, currentBlock;
	UInt32						numBlocks;

	UInt64						blockTimeoutMicroseconds;
	UInt64						ticksPerRingBuffer;
	UInt64						startTime;

    IOAudioStream				*audioStream[MAX_STREAMS];
	IOTimerEventSource			*timerEventSource;

	PADevice					*device;
	char						deviceName[DEVICENAME_MAX];
	UInt32						clockDirection;
};

#endif // PAENGINE_H