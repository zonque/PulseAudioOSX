/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PAUserClient.h"
#include "PAUserClientTypes.h"
#include "PAEngine.h"
#include "PADevice.h"
#include "PALog.h"

#define super IOAudioDevice

OSDefineMetaClassAndStructors(PADevice, IOAudioDevice)

bool
PADevice::initHardware(IOService *provider)
{
	debugFunctionEnter();

	if (!super::initHardware(provider))
		return false;

	setDeviceName(deviceInfo.name);
	setDeviceShortName(deviceInfo.name);
	setManufacturerName("pulseaudio.org");
	setDeviceModelName("virtual audio device");
	setDeviceTransportType('virt');
	setDeviceCanBeDefault(true);

	audioEngine = new PAEngine;

	if (!audioEngine)
		return false;

	if (!audioEngine->init(NULL) ||
		!audioEngine->setDeviceInfo(&deviceInfo)) {
		audioEngine->release();
		audioEngine = NULL;
		return false;
	}

	activateAudioEngine(audioEngine);

	// the core is holding a reference now, so we can drop ours
	audioEngine->release();

	return true;
}

void
PADevice::free(void)
{
	debugFunctionEnter();
	super::free();
}

void
PADevice::setInfo(const struct PAVirtualDevice *info)
{
	debugFunctionEnter();
	memcpy(&deviceInfo, info, sizeof(deviceInfo));
}

IOReturn
PADevice::getInfo(struct PAVirtualDevice *info)
{
	debugFunctionEnter();
	memcpy(info, &deviceInfo, sizeof(deviceInfo));
	return kIOReturnSuccess;
}

IOReturn
PADevice::setSamplerate(UInt rate)
{
	debugFunctionEnter();
	return kIOReturnSuccess;
}
