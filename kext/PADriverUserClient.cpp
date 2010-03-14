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
#include "PADriverUserClient.h"
#include "PADriverUserClientTypes.h"
#include "PALog.h"

#define super IOUserClient

OSDefineMetaClassAndStructors(PADriverUserClient, IOUserClient)

#pragma mark ########## IOUserClient ##########

bool
PADriverUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
	if (!owningTask)
		return false;

	clientTask = owningTask;
	return super::initWithTask(owningTask, securityID, type);
}

bool
PADriverUserClient::start(IOService *provider)
{
	debugFunctionEnter();

	driver = OSDynamicCast(PADriver, provider);
	if (!driver)
		return false;

	return super::start(provider);
}

void
PADriverUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

IOReturn
PADriverUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return terminate(0);
}

bool
PADriverUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	driver->removeAllAudioDevices();
	return super::terminate(options);
}

IOReturn
PADriverUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args,
				   IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOExternalMethodDispatch genericMethodDispatch;
	
	if (!target)
		target = this;
	
	genericMethodDispatch.function = (IOExternalMethodAction) &PADriverUserClient::genericMethodDispatchAction;
	genericMethodDispatch.checkScalarInputCount	= args->scalarInputCount;
	genericMethodDispatch.checkStructureInputSize	= args->structureInputSize;
	genericMethodDispatch.checkScalarOutputCount	= args->scalarOutputCount;
	genericMethodDispatch.checkStructureOutputSize	= args->structureOutputSize;
	
	currentDispatchSelector = selector;
	dispatch = (IOExternalMethodDispatch *) &genericMethodDispatch;
	
	return super::externalMethod(selector, args, dispatch, target, reference);
}

#pragma mark ########## message dispatch ##########

IOReturn
PADriverUserClient::genericMethodDispatchAction(PADriverUserClient *target,
						void *reference,
						IOExternalMethodArguments *args)
{
	IOReturn status = kIOReturnBadArgument;

	debugIOLog("%s(%p) -- currentDispatchSelector %d\n", __func__, target, target->currentDispatchSelector);
	//target->dumpIOExternalMethodArguments(args);

	if (args->asyncReferenceCount == 0) {
		switch (target->currentDispatchSelector) {
			case kPADriverUserClientGetNumberOfDevices:
				status = target->getNumberOfDevices(args);
				break;
			case kPADriverUserClientAddDevice:
				status = target->addDevice(args);
				break;
			case kPADriverUserClientRemoveDevice:
				status = target->removeDevice(args);
				break;
			default:
				IOLog("%s(%p): unknown selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	} else {
		/* ASYNC METHODS */
		IOLog("%s(%p): unknown async selector %d!\n", __func__, target, target->currentDispatchSelector);
		status = kIOReturnInvalid;
	}

	return status;
}

#pragma mark ########## PADriverUserClient interface ##########

IOReturn
PADriverUserClient::getNumberOfDevices(IOExternalMethodArguments *args)
{
	args->scalarOutput[0] = driver->numberOfDevices();
	return kIOReturnSuccess;
}

IOReturn
PADriverUserClient::addDevice(IOExternalMethodArguments *args)
{
	struct PAVirtualDeviceInfo info;
	
	if (!args->structureInput || args->structureInputSize != sizeof(info))
		return kIOReturnInvalid;

	memcpy(&info, args->structureInput, sizeof(info));
	IOReturn ret = driver->addAudioDevice(&info);

	if (args->structureOutput &&
	    args->structureOutputSize == sizeof(info))
		memcpy(args->structureOutput, &info, sizeof(info));

	return ret;
}

IOReturn
PADriverUserClient::removeDevice(IOExternalMethodArguments *args)
{
	UInt index = args->scalarInput[0];
	return driver->removeAudioDevice(index);
}
