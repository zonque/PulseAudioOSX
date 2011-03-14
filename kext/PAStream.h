/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PASTREAM_H
#define PASTREAM_H

#include "BuildNames.h"

#include <IOKit/IOLib.h>
#include <IOKit/audio/IOAudioStream.h>

class PAStream : public IOAudioStream
{
	OSDeclareDefaultStructors(PAStream)
	
public:
	IOReturn addClient(IOAudioClientBuffer *clientBuffer);
	void removeClient(IOAudioClientBuffer *clientBuffer);
};

#endif /* PASTREAM_H */

