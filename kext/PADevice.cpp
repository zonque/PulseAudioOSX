//
//   PADevice.cpp
//
//	Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
// 
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//

#include "PAEngine.h"
#include "PADevice.h"
#include "PALog.h"

OSDefineMetaClassAndStructors(PADevice, IOAudioDevice)

bool PADevice::initHardware(IOService* inProvider)
{
	debugFunctionEnter();

	if(!IOAudioDevice::initHardware(inProvider))
		return false;

	// provide some settings and strings
	setDeviceName("PulseAudio");
	setDeviceShortName("PulseAudio");
	setManufacturerName("pulseaudio.org");
	setDeviceModelName("virtual audio device");
	setDeviceTransportType('virt');
	setDeviceCanBeDefault(true);

	audioEngine = new PAEngine;

	if (!audioEngine)
		return false;
	
	if (!audioEngine->init(2, 2)) {
		audioEngine->release();
		return false;
	}
	
	activateAudioEngine(audioEngine);

	// the core is holding a reference now, so we can drop ours
	audioEngine->release();

	return true;
}
