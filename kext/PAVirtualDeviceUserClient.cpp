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
#include "PAVirtualDeviceUserClient.h"
#include "PAVirtualDeviceUserClientTypes.h"
#include "PALog.h"

#define super IOUserClient

OSDefineMetaClassAndStructors(PAVirtualDeviceUserClient, IOUserClient)

#pragma mark ########## IOUserClient ##########

bool
PAVirtualDeviceUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
	if (!owningTask)
		return false;

	clientTask = owningTask;
	return super::initWithTask(owningTask, securityID, type);
}

bool
PAVirtualDeviceUserClient::start(IOService *provider)
{
	debugFunctionEnter();
	
	device = OSDynamicCast(PAVirtualDevice, provider);
	if (!device)
		return false;
	
	return super::start(provider);
}

void
PAVirtualDeviceUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

IOReturn
PAVirtualDeviceUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return terminate(0);
}

IOReturn
PAVirtualDeviceUserClient::message(UInt32 type, IOService *provider,  void *argument)
{
	debugFunctionEnter();
	return super::message(type, provider, argument);
}

bool
PAVirtualDeviceUserClient::finalize(IOOptionBits options)
{
	debugFunctionEnter();
	
	if (samplePointerReadDescriptor) {
		samplePointerReadDescriptor->release();
		samplePointerReadDescriptor = NULL;
	}
	
	if (notificationReadDescriptor) {
		notificationReadDescriptor->release();
		notificationReadDescriptor = NULL;
	}
	
	return super::finalize(options);
}

bool
PAVirtualDeviceUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	return super::terminate(options);
}

IOReturn
PAVirtualDeviceUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *args,
					  IOExternalMethodDispatch *dispatch, OSObject *target, void *reference)
{
	IOExternalMethodDispatch genericMethodDispatch;

	if (!target)
		target = this;

	genericMethodDispatch.function = (IOExternalMethodAction) &PAVirtualDeviceUserClient::genericMethodDispatchAction;
	genericMethodDispatch.checkScalarInputCount	= args->scalarInputCount;
	genericMethodDispatch.checkStructureInputSize	= args->structureInputSize;
	genericMethodDispatch.checkScalarOutputCount	= args->scalarOutputCount;
	genericMethodDispatch.checkStructureOutputSize	= args->structureOutputSize;

	currentDispatchSelector = selector;
	dispatch = (IOExternalMethodDispatch *) &genericMethodDispatch;

	return super::externalMethod(selector, args, dispatch, target, reference);
}

IOReturn
PAVirtualDeviceUserClient::clientMemoryForType(UInt32 type, UInt32 *flags,
					       IOMemoryDescriptor **memory)
{
	IOMemoryDescriptor *buf;
	debugIOLog("%s(%p)::%s clientMemoryForType %lu\n", getName(), this, __func__, (unsigned long) type);

	switch (type) {
	case kPAMemoryInputSampleData:
		buf = device->audioInputBuf;
		break;
	case kPAMemoryOutputSampleData:
		buf = device->audioOutputBuf;
		break;
	default:
		debugIOLog("  ... unsupported!\n");
		return kIOReturnUnsupported;
	}

	if (buf)
		buf->retain();

	*memory = buf;

	return kIOReturnSuccess;
}

IOMemoryMap *
PAVirtualDeviceUserClient::removeMappingForDescriptor(IOMemoryDescriptor *memory)
{
	debugFunctionEnter();

	if (memory)
		memory->release();

	return super::removeMappingForDescriptor(memory);
}

#pragma mark ########## message dispatch ##########

IOReturn
PAVirtualDeviceUserClient::genericMethodDispatchAction(PAVirtualDeviceUserClient *target,
					  void *reference,
					  IOExternalMethodArguments *args)
{
	IOReturn status = kIOReturnBadArgument;

	//debugIOLog("%s(%p) -- currentDispatchSelector %d\n", __func__, target, target->currentDispatchSelector);
	//target->dumpIOExternalMethodArguments(args);

	if (args->asyncReferenceCount == 0) {
		switch (target->currentDispatchSelector) {
			case kPAVirtualDeviceUserClientGetDeviceInfo:
				status = target->getDeviceInfo(args);
				break;
			case kPAVirtualDeviceUserClientSetSampleRate:
				status = target->setSamplerate(args);
				break;
			case kPAVirtualDeviceUserClientWriteSamplePointer:
				status = target->writeSamplePointer(args);
				break;
			default:
				IOLog("%s(%p): unknown selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	} else {
		/* ASYNC METHODS */

		switch (target->currentDispatchSelector) {
			case kPAVirtualDeviceUserClientAsyncReadNotification:
				status = target->readNotification(args);
				break;
			default:
				IOLog("%s(%p): unknown async selector %d!\n", __func__, target, target->currentDispatchSelector);
				status = kIOReturnInvalid;
		} // switch
	}

	return status;
}

#pragma mark ########## PAVirtualDeviceUserClient interface ##########

IOReturn
PAVirtualDeviceUserClient::getDeviceInfo(IOExternalMethodArguments *args)
{
	struct PAVirtualDeviceInfo *info = (struct PAVirtualDeviceInfo *) args->structureOutput;

	if (!info || args->structureOutputSize != sizeof(*info))
		return kIOReturnInvalid;

	device->getInfo(info);

	return kIOReturnSuccess;
}

IOReturn
PAVirtualDeviceUserClient::setSamplerate(IOExternalMethodArguments *args)

{
	UInt rate = args->scalarInput[0];
	return device->setSamplerate(rate);
}

IOReturn
PAVirtualDeviceUserClient::writeSamplePointer(IOExternalMethodArguments *args)
{
	struct samplePointerUpdateEvent *ev = (struct samplePointerUpdateEvent *) args->structureInput;
	
	if (!ev || args->structureInputSize != sizeof(*ev))
		return kIOReturnInvalid;

	device->writeSamplePointer(ev);
	
	return kIOReturnSuccess;
}

#pragma mark ########## PAVirtualDeviceUserClient interface: notification feedback ##########

IOReturn
PAVirtualDeviceUserClient::readNotification(IOExternalMethodArguments *args)
{
	if (args->scalarInput[0] == 0 ||
	    args->scalarInput[1] != sizeof(struct notificationBlock))
		return kIOReturnBadArgument;
	
	if (notificationReadDescriptor)
		return kIOReturnBusy;
	
	notificationReadDescriptor = IOMemoryDescriptor::withAddressRange((mach_vm_address_t) args->scalarInput[0],
									  args->scalarInput[1], kIODirectionInOut, clientTask);
	if (!notificationReadDescriptor)
		return kIOReturnBadArgument;
	
	notificationReadDescriptor->map();
	
	bcopy(args->asyncReference, notificationReadReference, sizeof(OSAsyncReference64));

	if (device->engineState() == kIOAudioEngineRunning)
		sendNotification(kPAVirtualDeviceUserClientNotificationEngineStarted, 0);	

	return kIOReturnSuccess;
}

void
PAVirtualDeviceUserClient::sendNotification(UInt32 notificationType, UInt32 value)
{
	if (!notificationReadDescriptor)
		return;
	
	clock_sec_t secs;
	clock_nsec_t nanosecs;
	clock_get_system_nanotime (&secs, &nanosecs);
	
	notificationBlock no;
	
	no.timeStampSec = secs;
	no.timeStampNanoSec = nanosecs;
	no.notificationType = notificationType;
	no.value = value;
	
	if (notificationReadDescriptor->prepare() != kIOReturnSuccess) {
		IOLog("%s(%p): notificationReadDescriptor->prepare() failed!\n", getName(), this);
		notificationReadDescriptor->release();
		notificationReadDescriptor = NULL;
		return;
	}
	
	notificationReadDescriptor->writeBytes(0, &no, sizeof(no));
	notificationReadDescriptor->complete();
	
	sendAsyncResult64(notificationReadReference, kIOReturnSuccess, NULL, 0);
}
