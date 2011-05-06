/***
 This file is part of PulseAudioKext

 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PADEVICE_H
#define PADEVICE_H

#include "BuildNames.h"

#include <IOKit/audio/IOAudioDevice.h>

#include "PADriver.h"
#include "PAUserClientCommonTypes.h"

class PADevice : public IOAudioDevice
{
        OSDeclareDefaultStructors(PADevice)
        struct PAVirtualDeviceInfo deviceInfo;

public:
        bool        init(OSDictionary *dictionary);
        bool        initHardware(IOService *provider);
        void        setInfo(const struct PAVirtualDeviceInfo *info);
        void        getInfo(struct PAVirtualDeviceInfo *info);
};

#endif /* PADEVICE_H */

