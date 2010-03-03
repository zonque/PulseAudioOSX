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

	deviceArray = OSArray::withCapacity(1);
	if (!deviceArray)
		return false;
	
	return true;
}

void
PADriver::free(void)
{
	debugFunctionEnter();

	if (!deviceArray)
		return;

	deviceArray->release();
	deviceArray = NULL;
	
	super::free();
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
	return deviceArray->getCount();
}

IOReturn
PADriver::addAudioDevice(const struct PAVirtualDevice *info)
{
	PADevice *device = new PADevice;

	if (!device)
		return kIOReturnNoMemory;

	if (!device->init(NULL)) {
		device->release();
		return kIOReturnError;
	}

	device->setInfo(info);
	device->attachToParent(this, gIOServicePlane);

	if (!device->start(this) ||
		!deviceArray->setObject(device)) {
		device->release();
		return kIOReturnError;
	}

	/* the OSArray holds a reference now, so we can drop ours */
	//device->release();

	return kIOReturnSuccess;
}

IOReturn
PADriver::removeAudioDevice(UInt index)
{
	PADevice *device = OSDynamicCast(PADevice, deviceArray->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;

	device->detachFromParent(this, gIOServicePlane);
	device->stop(this);
	device->release();
	deviceArray->removeObject(index);

	return kIOReturnSuccess;
}

void
PADriver::removeAllAudioDevices(void)
{
	OSCollectionIterator *iter = OSCollectionIterator::withCollection(deviceArray);
	OSObject *obj;

	while ((obj = iter->getNextObject())) {
		PADevice *device = OSDynamicCast(PADevice, obj);

		if (device) {
			device->detachFromParent(this, gIOServicePlane);
			device->stop(this);
			device->release();
		}
	}

	iter->release();
	deviceArray->flushCollection();
}

IOReturn
PADriver::getAudioEngineInfo(struct PAVirtualDevice *info, UInt index)
{
	PADevice *device = OSDynamicCast(PADevice, deviceArray->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;
	
	return device->getInfo(info);
}

IOReturn
PADriver::setSamplerate(UInt index, UInt rate)
{
	PADevice *device = OSDynamicCast(PADevice, deviceArray->getObject(index));
	
	if (!device)
		return kIOReturnInvalid;
	
	return device->setSamplerate(rate);
}
