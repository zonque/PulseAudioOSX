/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PAStream.h"
#include "PALog.h"

#define super IOAudioStream

OSDefineMetaClassAndStructors(PAStream, IOAudioStream)

IOReturn
PAStream::addClient(IOAudioClientBuffer *clientBuffer)
{
	debugFunctionEnter();
	return super::addClient(clientBuffer);
}

void
PAStream::removeClient(IOAudioClientBuffer *clientBuffer)
{
	debugFunctionEnter();
	super::removeClient(clientBuffer);	
}
