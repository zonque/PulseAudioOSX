//
//   PAUserClient.cpp
//
//	Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
// 
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//

#include <IOKit/IOTypes.h>

#include "PAEngine.h"
#include "PADevice.h"
#include "PAUserClient.h"
#include "PALog.h"

enum {
	kPulseAudioMemoryInputSampleData = 0,
	kPulseAudioMemoryOutputSampleData
};

#define super IOUserClient

OSDefineMetaClassAndStructors(PAUserClient, IOUserClient)

#pragma mark --- IOUserClient ---

IOReturn PAUserClient::clientMemoryForType(UInt32 type, UInt32 *flags,
										   IOMemoryDescriptor **memory)
{
	IOMemoryDescriptor *buf;
	debugIOLog("%s(%p)::%s clientMemoryForType %lu\n", getName(), this, __func__, type);
	
	switch (type) {
	case kPulseAudioMemoryInputSampleData:
		buf = device->audioEngine->audioInBuf;
		break;
	case kPulseAudioMemoryOutputSampleData:
		buf = device->audioEngine->audioOutBuf;
		break;
	default:
		debugIOLog("  ... unsupported!\n");
		return kIOReturnUnsupported;
	}

	*memory = buf;

	return kIOReturnSuccess;
}

bool PAUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
	debugFunctionEnter();
	return true;
}

bool PAUserClient::start(IOService *provider)
{
	debugFunctionEnter();

	device = OSDynamicCast(PADevice, provider);
	if (!device)
		return false;

	return true;
}

void  PAUserClient::stop(IOService *provider)
{
	debugFunctionEnter();
	super::stop(provider);
}

IOReturn PAUserClient::clientClose(void)
{
	debugFunctionEnter();
	// DON'T call super::clientClose, which just returns notSupported
	return kIOReturnSuccess;
}

IOReturn PAUserClient::message(UInt32 type, IOService *provider,  void *argument)
{
	debugFunctionEnter();
	return super::message(type, provider, argument);
}

bool PAUserClient::finalize(IOOptionBits options)
{
	debugFunctionEnter();
	return super::finalize(options);
}

bool PAUserClient::terminate(IOOptionBits options)
{
	debugFunctionEnter();
	return super::terminate(options);
}
