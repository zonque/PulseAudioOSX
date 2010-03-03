/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PAEngine.h"
#include "PAUserClientTypes.h"
#include "PALog.h"

#include <libkern/sysctl.h>

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioControl.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/audio/IOAudioStream.h>

#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOTimerEventSource.h>

#define SAMPLERATES				{ 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 }
#define NUM_SAMPLE_FRAMES       (1024*16)
#define CHANNELS_PER_STREAM     2
#define BYTES_PER_SAMPLE        sizeof(float)
#define AUDIO_BUFFER_SIZE       (NUM_SAMPLE_FRAMES * BYTES_PER_SAMPLE * CHANNELS_PER_STREAM)

#define super IOAudioEngine

OSDefineMetaClassAndStructors(PAEngine, IOAudioEngine)

#pragma mark ########## IOAudioEngine ##########

bool PAEngine::init(struct PAVirtualDevice *info)
{
    if (!super::init(NULL))
        return false;

	channelsIn = info->channelsIn;
	channelsOut = info->channelsOut;
    nStreams = max(channelsIn, channelsOut) / CHANNELS_PER_STREAM;

	return true;
}

void PAEngine::free()
{
	debugFunctionEnter();

    if (audioOutBuf) {
        audioOutBuf->complete();
        audioOutBuf->release();
        audioOutBuf = NULL;
    }
    
    if (audioInBuf) {
        audioInBuf->complete();
        audioInBuf->release();
        audioInBuf = NULL;
    }

    super::free();
}

IOAudioStream *PAEngine::createNewAudioStream(IOAudioStreamDirection direction, 
											  void *sampleBuffer)
{
	UInt32 sampleRates[] = SAMPLERATES;
    IOAudioSampleRate rate;
    IOAudioStream *audioStream = new IOAudioStream;

	debugFunctionEnter();

    if (!audioStream)
        return NULL;
    
    if (!audioStream->initWithAudioEngine(this, direction, 1)) {
        audioStream->release();
        return NULL;
    } 
    
    audioStream->setSampleBuffer(sampleBuffer, AUDIO_BUFFER_SIZE);
    rate.fraction = 0;

    IOAudioStreamFormat format;

    format.fNumChannels              = CHANNELS_PER_STREAM;
    format.fSampleFormat             = kIOAudioStreamSampleFormatLinearPCM;
    format.fNumericRepresentation    = kIOAudioStreamNumericRepresentationIEEE754Float;
    format.fBitDepth                 = 32;
    format.fBitWidth                 = 32;
    format.fAlignment                = kIOAudioStreamAlignmentHighByte;
    format.fByteOrder                = kIOAudioStreamByteOrderBigEndian;
    format.fIsMixable                = true;
    format.fDriverTag                = 0;

	for (UInt32 i = 0; i < sizeof(sampleRates) / sizeof(sampleRates[0]); i++) {
		rate.whole = sampleRates[i];
		audioStream->addAvailableFormat(&format, &rate, &rate);
	}

    audioStream->setFormat(&format);
    
    return audioStream;
}

bool PAEngine::initHardware(IOService *provider)
{
	debugFunctionEnter();

	UInt32 sampleRates[] = SAMPLERATES;

	if (!super::initHardware(provider))
		return false;

    sampleRate.whole = sampleRates[0];
    sampleRate.fraction = 0;
    setSampleRate(&sampleRate);
        
    setDescription("PulseAudio");
    setNumSampleFramesPerBuffer(NUM_SAMPLE_FRAMES);

	UInt32 audioBufferSize = AUDIO_BUFFER_SIZE * nStreams;

    audioInBuf	= IOBufferMemoryDescriptor::withCapacity(audioBufferSize, kIODirectionInOut);
    audioOutBuf	= IOBufferMemoryDescriptor::withCapacity(audioBufferSize, kIODirectionInOut);

	if (!audioInBuf || !audioOutBuf || !audioStream) {
		IOLog("%s(%p)::%s unable to allocate memory\n", getName(), this, __func__);
		return false;
	}

	audioInBuf->prepare();
	audioOutBuf->prepare();

    for (UInt32 i = 0; i < nStreams; i++) {
		IOAudioStream *stream;
	    char *streamBuf;

        if (i * CHANNELS_PER_STREAM < channelsIn) {
            streamBuf = (char *) audioInBuf->getBytesNoCopy() + (i * AUDIO_BUFFER_SIZE);
            stream = createNewAudioStream(kIOAudioStreamDirectionInput, streamBuf);
            if (!stream) {
                IOLog("%s(%p)::%s failed to create audio streams\n", getName(), this, __func__);
                return false;
            }
            
            addAudioStream(stream);
            audioStream[i * 2] = stream;
            stream->release();
        }

        if (i * CHANNELS_PER_STREAM < channelsOut) {
            streamBuf = (char *) audioOutBuf->getBytesNoCopy() + (i * AUDIO_BUFFER_SIZE);
            stream = createNewAudioStream(kIOAudioStreamDirectionOutput, streamBuf);
            if (!stream) {
                IOLog("%s(%p)::%s failed to create audio streams\n", getName(), this, __func__);
                return false;
            }
            
            addAudioStream(stream);
            audioStream[(i * 2) + 1] = stream;
            stream->release();
        }
    }

	timeStampFactory = new PATimeStampFactory(NUM_SAMPLE_FRAMES, sampleRates[0]);

	return true;
}

OSString* PAEngine::getGlobalUniqueID()
{
    return OSString::withCString("PulseAudioIOAudioEngine");
}

IOReturn PAEngine::performAudioEngineStart()
{
	debugFunctionEnter();
	setClockIsStable(true);
	currentFrame = 0;

	return kIOReturnSuccess;
}

IOReturn PAEngine::performAudioEngineStop()
{
	debugFunctionEnter();
	return kIOReturnSuccess;
}

IOReturn PAEngine::performFormatChange(IOAudioStream *inStream,
									   const IOAudioStreamFormat *inNewFormat,
									   const IOAudioSampleRate *inNewSampleRate)
{
	debugFunctionEnter();
	return kIOReturnSuccess;
}

IOReturn PAEngine::clipOutputSamples(const void *inMixBuffer, void *outTargetBuffer,
									 UInt32 inFirstFrame, UInt32 inNumberFrames,
									 const IOAudioStreamFormat *inFormat, IOAudioStream *inStream)
{
	memcpy((float *) outTargetBuffer + inFirstFrame,
			(float *) inMixBuffer + inFirstFrame,
			inNumberFrames * sizeof(float));

	//IOLog("%s -- %d -> %d\n", __func__, inFirstFrame, inNumberFrames);
	
	return kIOReturnSuccess;
}

IOReturn PAEngine::convertInputSamples(const void *inSourceBuffer, void *outTargetBuffer,
									   UInt32 inFirstFrame, UInt32 inNumberFrames, 
									   const IOAudioStreamFormat *inFormat, IOAudioStream *inStream)
{
	memcpy((float *) outTargetBuffer + inFirstFrame,
			(float *) inSourceBuffer + inFirstFrame,
			inNumberFrames * sizeof(float));

	//currentFrame = (inFirstFrame + inNumberFrames) % NUM_SAMPLE_FRAMES;

	return kIOReturnSuccess;	
}

UInt32 PAEngine::getCurrentSampleFrame()
{
	//IOLog("%s: currentFrame %d\n", __func__, currentFrame);
	return timeStampFactory->getCurrentSampleFrame();
}
