/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PA_DRIVER_USERCLIENT_TYPES_H
#define PA_DRIVER_USERCLIENT_TYPES_H

/* synchronous functions */
enum {
	kPADriverUserClientGetNumberOfDevices	= 0,
	kPADriverUserClientAddDevice		= 1,
	kPADriverUserClientRemoveDevice		= 2,
};

#endif /* PAUSERCLIENT_TYPES_H */

