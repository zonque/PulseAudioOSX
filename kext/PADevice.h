/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PADEVICE_H
#define PADEVICE_H

#include <IOKit/audio/IOAudioDevice.h>

#include "BuildNames.h"
#include "PAUserClientTypes.h"

class PAEngine;
class PAUserClient;
struct PAVirtualDevice;

class PADevice : public IOAudioDevice
{
	OSDeclareDefaultStructors(PADevice)
	struct PAVirtualDevice deviceInfo;

public:
	IOReturn	initHardware(IOService *provider,
							 const struct PAVirtualDevice *info);
	PAEngine	*audioEngine;
	IOReturn	 getInfo(struct PAVirtualDevice *info);
	IOReturn	 setSamplerate(UInt rate);
};

#endif // PADEVICE_H