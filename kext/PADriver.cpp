/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <IOKit/IOTypes.h>

#include "PAUserClient.h"
#include "PADriver.h"
#include "PADevice.h"
#include "PALog.h"

#define super IOService

OSDefineMetaClassAndStructors(PADriver, super)

bool
PADriver::start(IOService *provider)
{
	debugFunctionEnter();

	UInt i;
	
	for (i = 0; i < MAX_DEVICES; i++)
		device[i] = NULL;

	return TRUE;
}

bool
PADriver::registerUserClient(PAUserClient *client)
{
	if (userClient)
		return false;
	
	userClient = client;
	client->retain();
	
	return true;
}

void
PADriver::deregisterUserClient(PAUserClient *client)
{
	if (userClient == client) {
		userClient->release();
		userClient = NULL;
	}
}

IOReturn
PADriver::addAudioDevice(const struct PAVirtualDevice *info)
{
	UInt i;

	for (i = 0; i < MAX_DEVICES; i++)
		if (device[i] == NULL) {
			device[i] = new PADevice;
			if (!device[i])
				return kIOReturnNoMemory;

			return device[i]->initHardware(this, info);
		}

	return kIOReturnOverrun;
}

IOReturn
PADriver::removeAudioDevice(UInt index)
{
	if (index >= MAX_DEVICES || !device[index])
		return kIOReturnInvalid;

	device[index]->release();
	device[index] = NULL;
	
	return kIOReturnSuccess;
}

IOReturn
PADriver::getAudioEngineInfo(struct PAVirtualDevice *info, UInt index)
{
	if (index >= MAX_DEVICES || !device[index])
		return kIOReturnInvalid;

	return device[index]->getInfo(info);
}

IOReturn
PADriver::setSamplerate(UInt index, UInt rate)
{
	if (index >= MAX_DEVICES || !device[index])
		return kIOReturnInvalid;
	
	return device[index]->setSamplerate(rate);
}
