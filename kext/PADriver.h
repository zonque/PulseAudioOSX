/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PADRIVER_H
#define PADRIVER_H

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/IOLib.h>

#include "BuildNames.h"

#define MAX_DEVICES		64

class PADevice;
class PAUserClient;

class PADriver : public IOService
{
	OSDeclareDefaultStructors(PADriver)
	
	PAUserClient *userClient;
	
public:
	bool			start(IOService *provider);

	bool			registerUserClient(PAUserClient *client);
	void			deregisterUserClient(PAUserClient *client);
	
	IOReturn		addAudioDevice(const struct PAVirtualDevice *info);
	IOReturn		removeAudioDevice(UInt index);
	IOReturn		getAudioEngineInfo(struct PAVirtualDevice *info, UInt index);
	IOReturn		setSamplerate(UInt index, UInt rate);

private:
	PADevice		*device[MAX_DEVICES];
};

#endif // PADRIVER_H