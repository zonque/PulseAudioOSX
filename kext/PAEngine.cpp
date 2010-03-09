/***
 This file is part of PulseAudioKext

 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>

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
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/IOWorkLoop.h>

#define SAMPLERATES				{ 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 }
#define NUM_SAMPLE_FRAMES       (1024*16*4)
#define CHANNELS_PER_STREAM     2
#define BYTES_PER_SAMPLE        sizeof(float)
#define AUDIO_BUFFER_SIZE       (NUM_SAMPLE_FRAMES * BYTES_PER_SAMPLE * CHANNELS_PER_STREAM)

#define super IOAudioEngine

OSDefineMetaClassAndStructors(PAEngine, IOAudioEngine)

#pragma mark ########## IOAudioEngine ##########

void
PAEngine::free()
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

	if (timerEventSource) {
		IOWorkLoop *workLoop = getWorkLoop();
		if(workLoop)
			workLoop->removeEventSource(timerEventSource);

		timerEventSource->release();
		timerEventSource = NULL;
	}

    super::free();
}

IOAudioStream *
PAEngine::createNewAudioStream(IOAudioStreamDirection direction,
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

bool
PAEngine::initHardware(IOService *provider)
{
	UInt32 sampleRates[] = SAMPLERATES;
	IOAudioSampleRate sampleRate;

	debugFunctionEnter();

	device = OSDynamicCast(PADevice, provider);
	if (!device)
		return false;

	if (!super::initHardware(provider))
		return false;

	sampleRate.whole = sampleRates[0];
    sampleRate.fraction = 0;
    setSampleRate(&sampleRate);
	setNewSampleRate(sampleRate.whole);

    setDescription(info->name);
    setNumSampleFramesPerBuffer(NUM_SAMPLE_FRAMES);

	info->audioBufferSize = AUDIO_BUFFER_SIZE * nStreams;

    audioInBuf	= IOBufferMemoryDescriptor::withCapacity(info->audioBufferSize, kIODirectionInOut);
    audioOutBuf	= IOBufferMemoryDescriptor::withCapacity(info->audioBufferSize, kIODirectionInOut);
	
	if (!audioInBuf || !audioOutBuf) {
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

	IOWorkLoop *workLoop = getWorkLoop();
	if (!workLoop)
		return false;

	timerEventSource = IOTimerEventSource::timerEventSource(this, timerFired);

	if (!timerEventSource) {
		IOLog("%s(%p)::%s failed to create timerEventSource\n", getName(), this, __func__);
		return false;
	}

	workLoop->addEventSource(timerEventSource);

	return true;
}

bool
PAEngine::setDeviceInfo(struct PAVirtualDevice *newInfo)
{
	debugFunctionEnter();

	info = newInfo;

	if (!info->blockSize ||
		(NUM_SAMPLE_FRAMES % info->blockSize) != 0)
		return false;
	
	channelsIn = info->channelsIn;
	channelsOut = info->channelsOut;
	nStreams = max(channelsIn, channelsOut) / CHANNELS_PER_STREAM;

	if (nStreams == 0)
			return false;

	return true;
}

OSString *
PAEngine::getGlobalUniqueID()
{
    char tmp[128];
	snprintf(tmp, sizeof(tmp), "%s (%s)", getName(), info->name);
    return OSString::withCString(tmp);
}

IOReturn
PAEngine::performAudioEngineStart()
{
	debugFunctionEnter();
	currentFrame = 0;
	currentBlock = 0;
	startTime = 0;

	// if we're clocked from kernel side, start our timer.
	// otherwise, we rely on the userspace to push the block counter forward.
	if (info->clockDirection == kPADeviceClockFromKernel) {
		timerEventSource->setTimeoutUS(blockTimeoutMicroseconds);
		setClockIsStable(true);
	}

	device->driver->sendNotification(device, kPAUserClientNotificationEngineStarted, 0);
	
	return kIOReturnSuccess;
}

IOReturn
PAEngine::performAudioEngineStop()
{
	debugFunctionEnter();

	if (info->clockDirection == kPADeviceClockFromKernel)
		timerEventSource->cancelTimeout();

	device->driver->sendNotification(device, kPAUserClientNotificationEngineStopped, 0);

	return kIOReturnSuccess;
}

void
PAEngine::setNewSampleRate(UInt32 sampleRate)
{
	currentSampleRate = sampleRate;
	
	//	get the host time base info
	mach_timebase_info timeBaseInfo;
	clock_timebase_info(&timeBaseInfo);
	
	//	compute the unscaled host ticks per ring buffer
	//	computed in steps to make sure we don't overflow
	//	note that we really don't care about fractional host ticks here
	ticksPerRingBuffer = 1000000000ULL * NUM_SAMPLE_FRAMES;
	ticksPerRingBuffer /= currentSampleRate;
	ticksPerRingBuffer *= timeBaseInfo.denom;
	ticksPerRingBuffer /= timeBaseInfo.numer;
	
	//	scale by the rate scalar
	//	note that the rate scalar is stored as a 4.28 fixed point number which allows for a range of 0 to 8.
	//	note also that we don't care about fractions here.
	//ticksPerRingBuffer *= mRateScalar;
	//ticksPerRingBuffer >>= 28;
	
	blockTimeoutMicroseconds = 1000000ULL * info->blockSize / currentSampleRate;
	numBlocks = NUM_SAMPLE_FRAMES / info->blockSize;

	device->driver->sendNotification(device, kPAUserClientNotificationSampleRateChanged, sampleRate);
}

IOReturn
PAEngine::performFormatChange(IOAudioStream *stream,
							  const IOAudioStreamFormat *newFormat,
							  const IOAudioSampleRate *newSampleRate)
{
	debugFunctionEnter();

	if (newSampleRate)
		setNewSampleRate(newSampleRate->whole);

	return kIOReturnSuccess;
}

UInt32
PAEngine::getCurrentSampleFrame()
{
	//	this keeps the the erase head one full block behind
	UInt32 frame = currentBlock * info->blockSize;

	if (currentBlock != 0)
		frame--;

	return frame;
}

#pragma mark ########## Timestamp factory ##########

void
PAEngine::timerFired(OSObject *target, IOTimerEventSource *timerSource)
{
	PAEngine *engine = OSDynamicCast(PAEngine, target);
	if (!engine)
		return;

	engine->currentBlock++;

	//	check to see if we have wrapped around
	if (engine->currentBlock > engine->numBlocks) {
		engine->currentBlock = 0;

		AbsoluteTime timeStamp;
		engine->getNextTimeStamp(engine->status->fCurrentLoopCount, &timeStamp);
		engine->takeTimeStamp(true, &timeStamp);
	}

//	IOLog(" ---- currentBlock %d\n", engine->currentBlock);
	
	timerSource->setTimeoutUS(engine->blockTimeoutMicroseconds);
	engine->device->driver->reportSamplePointer(engine->device,
												engine->currentBlock *
													engine->info->blockSize);
}

void
PAEngine::getNextTimeStamp(UInt32 inLoopCount, AbsoluteTime *outTimeStamp)
{
	UInt64 timeStamp = startTime;

	//	make sure we have a start time
	if(!startTime) {
		clock_get_uptime(&startTime);
		timeStamp = startTime;
	} else {
		//	add in the number of loops through the ring buffer
		timeStamp += inLoopCount * ticksPerRingBuffer;

		//	compute the amount of jitter
		//UInt64 jitter = (UInt64) random() * (UInt64) maxJitter;
		//jitter >>= 32;
		//timeStamp += jitter;
	}

	//	return the time stamp as an AbsoluteTime to make life easy on the driver
	*outTimeStamp = *((AbsoluteTime*) &timeStamp);
}
