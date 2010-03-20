/***
 This file is part of PulseAudioKext

 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>

 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PAVirtualDeviceUserClient.h"
#include "PAVirtualDeviceUserClientTypes.h"
#include "PAEngine.h"
#include "PAVirtualDevice.h"
#include "PALog.h"

#include "PAVirtualDeviceUserClientTypes.h"
#include "PADriverUserClientTypes.h"

#define super IOService

OSDefineMetaClassAndStructors(PAVirtualDevice, IOService)

bool
PAVirtualDevice::init(OSDictionary *dictionary)
{
	OSDictionary *newDict = NULL;

	if (!dictionary)
		newDict = dictionary = OSDictionary::withCapacity(1);
	
	dictionary->setObject("IOUserClientClass", OSString::withCString(XStr(PAVirtualDeviceUserClient)));

	if (!super::init(dictionary)) {
		if (newDict)
			newDict->release();
			
		return false;
	}

	if (newDict)
		newDict->release();

	debugFunctionEnter();
	return true;
}

bool
PAVirtualDevice::start(IOService *provider)
{
	debugFunctionEnter();

	if (!super::start(provider))
		return false;

	audioEngine = OSDynamicCast(PAEngine, provider);
	if (!audioEngine)
		return false;

	registerService();
	return true;
}

void
PAVirtualDevice::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

bool
PAVirtualDevice::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	return super::terminate(options);
}

void
PAVirtualDevice::free(void)
{
	debugFunctionEnter();
	super::free();
}

void
PAVirtualDevice::sendNotification(UInt32 notificationType, UInt32 value)
{
	OSIterator *iter = getClientIterator();
	PAVirtualDeviceUserClient *client;
	
	while ((client = OSDynamicCast(PAVirtualDeviceUserClient, iter->getNextObject())))
		client->sendNotification(notificationType, value);

	iter->release();
}

void
PAVirtualDevice::setInfo(const struct PAVirtualDeviceInfo *newInfo)
{
	memcpy(&deviceInfo, newInfo, sizeof(deviceInfo));
}

void
PAVirtualDevice::getInfo(struct PAVirtualDeviceInfo *info)
{
	memcpy(info, &deviceInfo, sizeof(*info));
}

IOReturn
PAVirtualDevice::setSamplerate(UInt rate)
{
	return audioEngine->setNewSampleRate(rate);
}

void
PAVirtualDevice::writeSamplePointer(struct samplePointerUpdateEvent *ev)
{
	audioEngine->writeSamplePointer(ev);
}

IOAudioEngineState
PAVirtualDevice::engineState()
{
	return audioEngine->state;
}
