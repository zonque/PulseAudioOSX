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

OSDefineMetaClassAndStructors(PADevice, super)

IOReturn
PADevice::initHardware(IOService *provider,
					   const struct PAVirtualDevice *info)
{
	debugFunctionEnter();

	if (!IOAudioDevice::initHardware(provider))
		return false;

	setDeviceName(info->name);
	setDeviceShortName(info->name);
	setManufacturerName("pulseaudio.org");
	setDeviceModelName("virtual audio device");
	setDeviceTransportType('virt');
	setDeviceCanBeDefault(true);

	audioEngine = new PAEngine;

	if (!audioEngine)
		return false;

	memcpy(&deviceInfo, info, sizeof(deviceInfo));

	if (!audioEngine->init(&deviceInfo)) {
		audioEngine->release();
		return false;
	}
	
	activateAudioEngine(audioEngine);

	// the core is holding a reference now, so we can drop ours
	audioEngine->release();

	return true;
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
