/***
 This file is part of PulseAudioKext

 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>

 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PALog.h"
#include "PAEngine.h"
#include "PAStream.h"
#include "PAVirtualDevice.h"
#include "PAUserClientCommonTypes.h"
#include "PAVirtualDeviceUserClientTypes.h"

#include <libkern/sysctl.h>

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/IOWorkLoop.h>

#define SAMPLERATES		{ 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 }
#define NUM_SAMPLE_FRAMES	(1024*16*4)
#define CHANNELS_PER_STREAM	2
#define BYTES_PER_SAMPLE	sizeof(float)
#define AUDIO_BUFFER_SIZE	(NUM_SAMPLE_FRAMES * BYTES_PER_SAMPLE * CHANNELS_PER_STREAM)

#define ARRAY_SIZE(a)		(sizeof(a) / sizeof(a[0]))

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

	if (virtualDeviceArray) {
		virtualDeviceArray->flushCollection();
		virtualDeviceArray->release();
		virtualDeviceArray = NULL;
	}

	super::free();
}

IOAudioStream *
PAEngine::createNewAudioStream(IOAudioStreamDirection direction,
							   void *sampleBuffer)
{
	UInt32 sampleRates[] = SAMPLERATES;
	IOAudioSampleRate rate;
	IOAudioStream *audioStream = new PAStream;

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

	format.fNumChannels		= CHANNELS_PER_STREAM;
	format.fSampleFormat		= kIOAudioStreamSampleFormatLinearPCM;
	format.fNumericRepresentation	= kIOAudioStreamNumericRepresentationIEEE754Float;
	format.fBitDepth		= 32;
	format.fBitWidth		= 32;
	format.fAlignment		= kIOAudioStreamAlignmentHighByte;
	format.fByteOrder		= kIOAudioStreamByteOrderBigEndian;
	format.fIsMixable		= true;
	format.fDriverTag		= 0;

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

	virtualDeviceArray = OSArray::withCapacity(1);
	if (!virtualDeviceArray) {
		IOLog("%s(%p)::%s unable to allocate memory\n", getName(), this, __func__);
		return false;
	}
	
	sampleRate.whole = sampleRates[0];
	sampleRate.fraction = 0;
	setSampleRate(&sampleRate);
	setNewSampleRate(sampleRate.whole);

	setDescription(info->name);
	setNumSampleFramesPerBuffer(NUM_SAMPLE_FRAMES);

	info->audioBufferSize = AUDIO_BUFFER_SIZE * nStreams;

	audioInBuf  = IOBufferMemoryDescriptor::withCapacity(info->audioBufferSize, kIODirectionInOut);
	audioOutBuf = IOBufferMemoryDescriptor::withCapacity(info->audioBufferSize, kIODirectionInOut);
	
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

	/* FIXME */
	if (addVirtualDevice(info, audioInBuf, audioOutBuf, this) != kIOReturnSuccess)
		return false;

	return true;
}

bool
PAEngine::setDeviceInfo(struct PAVirtualDeviceInfo *newInfo)
{
	debugFunctionEnter();

	info = newInfo;

	if (!info->blockSize ||
		(NUM_SAMPLE_FRAMES % info->blockSize) != 0) {
		IOLog("%s(%p):: bogus blockSize %d\n", getName(), this, (int) info->blockSize);
		return false;
	}

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
	samplePointer = 0;

	sendNotification(kPAVirtualDeviceUserClientNotificationEngineStarted, 0);

	return kIOReturnSuccess;
}

IOReturn
PAEngine::performAudioEngineStop()
{
	debugFunctionEnter();

	sendNotification(kPAVirtualDeviceUserClientNotificationEngineStopped, 0);

	return kIOReturnSuccess;
}

IOReturn
PAEngine::setNewSampleRate(UInt32 sampleRate)
{
	UInt i, validRates[] = SAMPLERATES;
	
	for (i = 0; i < ARRAY_SIZE(validRates); i++)
		if (validRates[i] == sampleRate)
			break;
	
	if (i == ARRAY_SIZE(validRates)) {
		IOLog("%s(%p): Invalid sample rate %d\n", getName(), this, (int) sampleRate);
		return kIOReturnInvalid;
	}
	
	currentSampleRate = sampleRate;

	sendNotification(kPAVirtualDeviceUserClientNotificationSampleRateChanged, sampleRate);

	return kIOReturnSuccess;
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
	return samplePointer;
}

void
PAEngine::writeSamplePointer(struct samplePointerUpdateEvent *ev)
{
	samplePointer = ev->samplePointer;
	if (samplePointer >= NUM_SAMPLE_FRAMES) {
		samplePointer %= NUM_SAMPLE_FRAMES;
		takeTimeStamp();
	}
}

#pragma mark ########## virtual device handling ##########

void
PAEngine::sendNotification(UInt32 notificationType, UInt32 value)
{
	OSCollectionIterator *iter = OSCollectionIterator::withCollection(virtualDeviceArray);
	PAVirtualDevice *dev;
	
	while ((dev = OSDynamicCast(PAVirtualDevice, iter->getNextObject())))
		dev->sendNotification(notificationType, value);
	
	iter->release();
}

IOReturn
PAEngine::addVirtualDevice(struct PAVirtualDeviceInfo *info,
			   IOMemoryDescriptor *inBuf,
			   IOMemoryDescriptor *outBuf,
			   void *refCon)
{
	PAVirtualDevice *dev = new PAVirtualDevice;

	if (!dev)
		return kIOReturnNoMemory;

	if (!dev->init(NULL) ||
	    !virtualDeviceArray->setObject(dev)) {
		dev->release();
		return kIOReturnError;
	}

	/* the OSArray holds a reference now */
	dev->release();

	dev->attachToParent(this, gIOServicePlane);
	dev->setInfo(info);

	if (!dev->start(this)) {
		/* FIXME - leaking 'device' */
		return kIOReturnError;
	}

	dev->audioInputBuf = inBuf;
	dev->audioInputBuf = outBuf;
	dev->refCon = refCon;
	
	return kIOReturnSuccess;
}

void
PAEngine::removeVirtualDeviceWithRefcon(void *refCon)
{
	OSCollectionIterator *iter = OSCollectionIterator::withCollection(virtualDeviceArray);
	PAVirtualDevice *dev;

	while ((dev = OSDynamicCast(PAVirtualDevice, iter->getNextObject()))) {
		UInt index = virtualDeviceArray->getNextIndexOfObject((OSMetaClassBase *) dev, 0);

		if (dev->refCon == refCon) {
			if (state == kIOAudioEngineRunning)
				dev->sendNotification(kPAVirtualDeviceUserClientNotificationEngineStopped, 0);
			dev->detachFromParent(this, gIOServicePlane);
			dev->stop(this);
			dev->terminate(0);
			virtualDeviceArray->removeObject(index);
		}
	}

	iter->release();
}
