/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAUSERCLIENT_H
#define PAUSERCLIENT_H

#include <IOKit/IOUserClient.h>

#include "PADriver.h"
#include "BuildNames.h"

class PAVirtualDevice;

class PADriverUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(PADriverUserClient)

private:
	PADriver	*driver;
	UInt		currentDispatchSelector;
	task_t		clientTask;

	/* IOMethodDispatchers */
	static IOReturn	genericMethodDispatchAction(PADriverUserClient *target, void *reference, IOExternalMethodArguments *args);

	IOReturn	getNumberOfDevices(IOExternalMethodArguments *args);
	IOReturn	addDevice(IOExternalMethodArguments *args);
	IOReturn	removeDevice(IOExternalMethodArguments *args);

// IOUserClient interface
public:
	IOReturn	externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
				       IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);
	IOReturn	clientClose(void);

	void		stop(IOService * provider);
	bool		start(IOService * provider);
	bool		initWithTask(task_t owningTask, void * securityID, UInt32 type);
	bool		terminate(IOOptionBits options);
};

#endif /* PAUSERCLIENT_H */

