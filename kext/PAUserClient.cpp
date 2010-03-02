/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
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

OSDefineMetaClassAndStructors(PAUserClient, super)

#pragma mark --- IOUserClient ---

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
			default:
				IOLog("%s(%p): unknown async selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	}
	
	return status;	
}

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
	debugIOLog("%s(%p)::%s clientMemoryForType %lu\n", getName(), this, __func__, type);
	
	switch (type) {
	case kPAMemoryInputSampleData:
		//buf = driver->audioEngine->audioInBuf;
		break;
	case kPAMemoryOutputSampleData:
		//buf = driver->audioEngine->audioOutBuf;
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
	debugFunctionEnter();
	return true;
}

bool
PAUserClient::start(IOService *provider)
{
	debugFunctionEnter();

	driver = OSDynamicCast(PADriver, provider);
	if (!driver)
		return false;

	return driver->registerUserClient(this);
}

void
PAUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	driver->deregisterUserClient(this);
	super::stop(provider);
}

IOReturn
PAUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return kIOReturnSuccess;
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
	return super::finalize(options);
}

bool
PAUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	return super::terminate(options);
}

#pragma mark --- PAUserClient interface ---

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
