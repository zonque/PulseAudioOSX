/***
 This file is part of PulseAudioKext

 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>

 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <IOKit/IOTypes.h>

#include "PAEngine.h"
#include "PADevice.h"
#include "PAUserClient.h"
#include "PAUserClientTypes.h"
#include "PALog.h"

#define super IOUserClient

OSDefineMetaClassAndStructors(PAUserClient, IOUserClient)

#pragma mark ########## IOUserClient ##########

IOReturn
PAUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args,
							 IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOExternalMethodDispatch genericMethodDispatch;

	if (!target)
		target = this;

	genericMethodDispatch.function = (IOExternalMethodAction) &PAUserClient::genericMethodDispatchAction;
	genericMethodDispatch.checkScalarInputCount		= args->scalarInputCount;
	genericMethodDispatch.checkStructureInputSize	= args->structureInputSize;
	genericMethodDispatch.checkScalarOutputCount	= args->scalarOutputCount;
	genericMethodDispatch.checkStructureOutputSize	= args->structureOutputSize;

	currentDispatchSelector = selector;
	dispatch = (IOExternalMethodDispatch *) &genericMethodDispatch;

	return super::externalMethod(selector, args, dispatch, target, reference);
}

IOReturn
PAUserClient::clientMemoryForType(UInt32 type, UInt32 *flags,
								  IOMemoryDescriptor **memory)
{
	IOMemoryDescriptor *buf;
	debugIOLog("%s(%p)::%s clientMemoryForType %lu\n", getName(), this, __func__, (unsigned long) type);

	UInt index = (type >> 8) & 0xff;

	switch (type & 0xff) {
	case kPAMemoryInputSampleData:
		buf = driver->getAudioMemory(index, false);
		break;
	case kPAMemoryOutputSampleData:
		buf = driver->getAudioMemory(index, true);
		break;
	default:
		debugIOLog("  ... unsupported!\n");
		return kIOReturnUnsupported;
	}

	*memory = buf;

	return kIOReturnSuccess;
}

bool
PAUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
	if (!owningTask)
		return false;

	clientTask = owningTask;
	return super::initWithTask(owningTask, securityID, type);
}

bool
PAUserClient::start(IOService *provider)
{
	debugFunctionEnter();

	driver = OSDynamicCast(PADriver, provider);
	if (!driver)
		return false;

	return super::start(provider);
}

void
PAUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

IOReturn
PAUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return terminate(0);
}

IOReturn
PAUserClient::message(UInt32 type, IOService *provider,  void *argument)
{
	debugFunctionEnter();
	return super::message(type, provider, argument);
}

bool
PAUserClient::finalize(IOOptionBits options)
{
	debugFunctionEnter();

	if (samplePointerReadDescriptor) {
		samplePointerReadDescriptor->release();
		samplePointerReadDescriptor = NULL;
	}

	return super::finalize(options);
}

bool
PAUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	driver->removeAllAudioDevices();
	return super::terminate(options);
}

#pragma mark ########## message dispatch ##########

IOReturn
PAUserClient::genericMethodDispatchAction(PAUserClient *target,
										  void *reference,
										  IOExternalMethodArguments *args)
{
	IOReturn status = kIOReturnBadArgument;

	debugIOLog("%s(%p) -- currentDispatchSelector %d\n", __func__, target, target->currentDispatchSelector);
	//target->dumpIOExternalMethodArguments(args);

	if (args->asyncReferenceCount == 0) {
		switch (target->currentDispatchSelector) {
			case kPAUserClientGetNumberOfDevices:
				status = target->getNumberOfDevices(args);
				break;
			case kPAUserClientAddDevice:
				status = target->addDevice(args);
				break;
			case kPAUserClientRemoveDevice:
				status = target->removeDevice(args);
				break;
			case kPAUserClientGetDeviceInfo:
				status = target->getDeviceInfo(args);
				break;
			case kPAUserClientSetSampleRate:
				status = target->setSamplerate(args);
				break;
			default:
				IOLog("%s(%p): unknown selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	} else {
		/* ASYNC METHODS */

		switch (target->currentDispatchSelector) {
			case kPAUserClientAsyncReadSamplePointer:
				status = target->readSamplePointer(args);
				break;
			case kPAUserClientAsyncReadSampleRateChange:
				//status = target->readSampleRateChange(args);
				break;
			default:
				IOLog("%s(%p): unknown async selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	}

	return status;
}

#pragma mark ########## PAUserClient interface ##########

IOReturn
PAUserClient::getNumberOfDevices(IOExternalMethodArguments *args)
{
	args->scalarOutput[0] = driver->numberOfDevices();
	return kIOReturnSuccess;
}

IOReturn
PAUserClient::addDevice(IOExternalMethodArguments *args)
{
	const struct PAVirtualDevice *info = (struct PAVirtualDevice *) args->structureInput;

	if (!info)
		return kIOReturnInvalid;

	return driver->addAudioDevice(info);
}

IOReturn
PAUserClient::removeDevice(IOExternalMethodArguments *args)
{
	UInt index = args->scalarInput[0];
	return driver->removeAudioDevice(index);
}

IOReturn
PAUserClient::getDeviceInfo(IOExternalMethodArguments *args)
{
	struct PAVirtualDevice *info = (struct PAVirtualDevice *) args->structureOutput;
	UInt index = args->scalarInput[0];

	if (!info || args->structureOutputSize != sizeof(*info))
		return kIOReturnInvalid;

	return driver->getAudioEngineInfo(info, index);
}

IOReturn
PAUserClient::setSamplerate(IOExternalMethodArguments *args)

{
	UInt index = args->scalarInput[0];
	UInt rate = args->scalarInput[1];

	return driver->setSamplerate(index, rate);
}

#pragma mark ########## PAUserClient interface: sample pointer feedback ##########


IOReturn
PAUserClient::readSamplePointer(IOExternalMethodArguments *args)
{
	if (args->scalarInput[0] == 0 ||
		args->scalarInput[1] != sizeof(struct samplePointerUpdateEvent))
		return kIOReturnBadArgument;
	
	if (samplePointerReadDescriptor)
		return kIOReturnBusy;
	
	samplePointerReadDescriptor =
		IOMemoryDescriptor::withAddressRange((mach_vm_address_t) args->scalarInput[0],
											 args->scalarInput[1], kIODirectionInOut, clientTask);
	if (!samplePointerReadDescriptor)
		return kIOReturnBadArgument;

	samplePointerReadDescriptor->map();

	bcopy(args->asyncReference, samplePointerReadReference, sizeof(OSAsyncReference64));
	
	return kIOReturnSuccess;
}

void
PAUserClient::reportSamplePointer(UInt32 index, UInt32 samplePointer)
{
	if (!samplePointerReadDescriptor)
		return;

	clock_sec_t secs;
	clock_nsec_t nanosecs;
	clock_get_system_nanotime (&secs, &nanosecs);

	samplePointerUpdateEvent ev;

	ev.timeStampSec = secs;
	ev.timeStampNanoSec = nanosecs;
	ev.index = index;
	ev.samplePointer = samplePointer;

	if (samplePointerReadDescriptor->prepare() != kIOReturnSuccess) {
		IOLog("%s(%p): samplePointerReadDescriptor->prepare() failed!\n", getName(), this);
		samplePointerReadDescriptor->release();
		samplePointerReadDescriptor = NULL;
		return;
	}

	samplePointerReadDescriptor->writeBytes(0, &ev, sizeof(ev));
	samplePointerReadDescriptor->complete();

	sendAsyncResult64(samplePointerReadReference, kIOReturnSuccess, NULL, 0);
}
