/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PA_VIRTUALDEVICE_USERCLIENT_H
#define PA_VIRTUALDEVICE_USERCLIENT_H

#include <IOKit/IOUserClient.h>

#include "PAVirtualDevice.h"
#include "BuildNames.h"

class PADevice;

class PAVirtualDeviceUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(PAVirtualDeviceUserClient)

private:
	PAVirtualDevice	*device;
	UInt		currentDispatchSelector;
	task_t		clientTask;

	IOMemoryDescriptor *samplePointerReadDescriptor;
	OSAsyncReference64  samplePointerReadReference;

	IOMemoryDescriptor *notificationReadDescriptor;
	OSAsyncReference64  notificationReadReference;

	/* IOMethodDispatchers */
	IOReturn	getDeviceInfo(IOExternalMethodArguments *args);
	IOReturn	setSamplerate(IOExternalMethodArguments *args);
	IOReturn	writeSamplePointer(IOExternalMethodArguments *args);
	IOReturn	readNotification(IOExternalMethodArguments *args);
	
	static IOReturn	genericMethodDispatchAction(PAVirtualDeviceUserClient *target,
						    void *reference,
						    IOExternalMethodArguments *args);
		
// IOUserClient interface
public:
	IOReturn	externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
				       IOExternalMethodDispatch *dispatch, OSObject *target, void *reference);
	IOReturn	clientMemoryForType(UInt32 type, UInt32 *flags, IOMemoryDescriptor **memory);
	IOMemoryMap	*removeMappingForDescriptor(IOMemoryDescriptor *memory);
	IOReturn	message(UInt32 type, IOService *provider,  void *argument = 0);
	IOReturn	clientClose(void);

	void		stop(IOService * provider);
	bool		start(IOService * provider);
	bool		initWithTask(task_t owningTask, void * securityID, UInt32 type);
	bool		finalize(IOOptionBits options);
	bool		terminate(IOOptionBits options);

	void		sendNotification(UInt32 notificationType, UInt32 value);
};

#endif /* PA_VIRTUALDEVICE_USERCLIENT_H */
