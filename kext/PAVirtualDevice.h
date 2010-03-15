/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PA_VIRTUALDEVICE_H
#define PA_VIRTUALDEVICE_H

#include "BuildNames.h"

#include <IOKit/audio/IOAudioDevice.h>

#include "PADriver.h"
#include "PAUserClientCommonTypes.h"
#include "PAVirtualDeviceUserClientTypes.h"

class PAEngine;
class PAUserClient;

class PAVirtualDevice : public IOService
{
	OSDeclareDefaultStructors(PAVirtualDevice)
	struct PAVirtualDeviceInfo deviceInfo;
	PAEngine		*audioEngine;

public:
	bool			init(OSDictionary* dictionary);
	bool			start(IOService *provider);
	void			stop(IOService *provider);

	bool			terminate(IOOptionBits options);

	void			free(void);

	void			reportSamplePointer(UInt32 pointer);
	void			sendNotification(UInt32 notificationType, UInt32 value);

	void			setInfo(const struct PAVirtualDeviceInfo *info);
	void			getInfo(struct PAVirtualDeviceInfo *info);
	IOReturn		setSamplerate(UInt rate);

	IOMemoryDescriptor	*audioInputBuf;
	IOMemoryDescriptor	*audioOutputBuf;
	void			*refCon;
};

#endif /* PA_VIRTUALDEVICE_H */

