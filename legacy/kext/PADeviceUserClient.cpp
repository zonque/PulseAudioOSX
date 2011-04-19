/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <IOKit/IOTypes.h>

#include "PAEngine.h"
#include "PADevice.h"
#include "PADeviceUserClient.h"
#include "PADeviceUserClientTypes.h"
#include "PALog.h"

#define super IOUserClient

OSDefineMetaClassAndStructors(PADeviceUserClient, IOUserClient)

#pragma mark ########## IOUserClient ##########

bool
PADeviceUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
	if (!owningTask)
		return false;
	
	clientTask = owningTask;
	return super::initWithTask(owningTask, securityID, type);
}

bool
PADeviceUserClient::start(IOService *provider)
{
	debugFunctionEnter();
	
	device = OSDynamicCast(PADevice, provider);
	if (!device)
		return false;
	
	return super::start(provider);
}

void
PADeviceUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

IOReturn
PADeviceUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return terminate(0);
}

bool
PADeviceUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	return super::terminate(options);
}

IOReturn
PADeviceUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args,
				   IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOExternalMethodDispatch genericMethodDispatch;
	
	if (!target)
		target = this;
	
	genericMethodDispatch.function = (IOExternalMethodAction) &PADeviceUserClient::genericMethodDispatchAction;
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
PADeviceUserClient::genericMethodDispatchAction(PADeviceUserClient *target,
						void *reference,
						IOExternalMethodArguments *args)
{
	IOReturn status = kIOReturnBadArgument;
	
	debugIOLog("%s(%p) -- currentDispatchSelector %d\n", __func__, target, target->currentDispatchSelector);
	//target->dumpIOExternalMethodArguments(args);
	
	if (args->asyncReferenceCount == 0) {
		switch (target->currentDispatchSelector) {
			case kPADeviceUserClientGetDeviceInfo:
				status = target->getDeviceInfo(args);
				break;
			default:
				IOLog("%s(%p): unknown selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	}

	return status;
}

#pragma mark ########## PADeviceUserClient interface ##########

IOReturn
PADeviceUserClient::getDeviceInfo(IOExternalMethodArguments *args)
{
	struct PAVirtualDeviceInfo *info = (struct PAVirtualDeviceInfo *) args->structureOutput;
	
	if (!info || args->structureOutputSize != sizeof(*info))
		return kIOReturnInvalid;
	
	device->getInfo(info);

	return kIOReturnSuccess;
}

