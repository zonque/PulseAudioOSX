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

OSDefineMetaClassAndStructors(PADriver, IOService)

#pragma mark ########## IOService ##########

bool
PADriver::init(OSDictionary* dictionary)
{
    if (!super::init(dictionary))
        return false;
	
	debugFunctionEnter();

	devices = OSArray::withCapacity(1);
	if (!devices)
		return false;
	
	return true;
}

void
PADriver::free(void)
{
	debugFunctionEnter();
	devices->release();
}

bool
PADriver::start(IOService *provider)
{
	debugFunctionEnter();

	if (!super::start(provider))
		return false;
	
	registerService();
	return true;
}

bool
PADriver::terminate(IOOptionBits options)
{
	debugFunctionEnter();	
	return super::terminate(options);
}

#pragma mark ########## proprietary methods ##########

IOReturn
PADriver::numberOfDevices(void)
{
	return devices->getCount();
}

IOReturn
PADriver::addAudioDevice(const struct PAVirtualDevice *info)
{
	IOReturn ret;
	
	PADevice *device = new PADevice;
	if (!device)
		return kIOReturnNoMemory;

	ret = device->initHardware(this, info);
	
	if (ret) {
		device->release();
		return ret;
	}

	if (!devices->setObject(device)) {
		device->release();
		return kIOReturnError;
	}

	return kIOReturnSuccess;
}

IOReturn
PADriver::removeAudioDevice(UInt index)
{
	PADevice *device = OSDynamicCast(PADevice, devices->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;

	device->release();
	devices->removeObject(index);	
	return kIOReturnSuccess;
}

IOReturn
PADriver::getAudioEngineInfo(struct PAVirtualDevice *info, UInt index)
{
	PADevice *device = OSDynamicCast(PADevice, devices->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;
	
	return device->getInfo(info);
}

IOReturn
PADriver::setSamplerate(UInt index, UInt rate)
{
	PADevice *device = OSDynamicCast(PADevice, devices->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;
	
	return device->setSamplerate(rate);
}
