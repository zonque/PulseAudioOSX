/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PADRIVER_H
#define PADRIVER_H

#include "BuildNames.h"

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/IOLib.h>

//#include "PAEngine.h"

class PADevice;
class PAEngine;

class PADriver : public IOService
{
	OSDeclareDefaultStructors(PADriver)

public:
	bool			init(OSDictionary* dictionary);
	void			free(void);
	bool			start(IOService *provider);
	bool			terminate(IOOptionBits options);

	IOReturn		numberOfDevices(void);
	IOReturn		addAudioDevice(const struct PAVirtualDevice *info);
	IOReturn		removeAudioDevice(UInt index);
	void			removeAllAudioDevices(void);
	IOReturn		getAudioEngineInfo(struct PAVirtualDevice *info, UInt index);
	IOReturn		setSamplerate(UInt index, UInt rate);

	IOMemoryDescriptor *getAudioMemory(UInt index, bool output);
	void			reportSamplePointer(PADevice *device, UInt32 pointer);
	
private:
	OSArray			*deviceArray;
};

#endif /* PADRIVER_H */
