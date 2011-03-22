/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <IOKit/audio/IOAudioEngineUserClient.h>

#include "PAUserClientCommonTypes.h"
#include "PAEngine.h"
#include "PAStream.h"
#include "PALog.h"

#define super IOAudioStream

OSDefineMetaClassAndStructors(PAStream, IOAudioStream)

IOReturn
PAStream::addClient(IOAudioClientBuffer *clientBuffer)
{
	debugFunctionEnter();

	struct PAVirtualDeviceInfo info;
	memcpy(&info, infoTemplate, sizeof(info));
	
	IOReturn ret = super::addClient(clientBuffer);
	
	IOAudioEngineUserClient *client = clientBuffer->userClient;
	OSObject *prop = client->copyProperty("IOUserClientCreator");
	if (!prop)
		return ret;

	OSString *str = OSDynamicCast(OSString, prop);
	if (!str) {
		prop->release();
		return ret;
	}

	if (ret == kIOReturnSuccess) {
		IOAudioStreamDirection dir = clientBuffer->audioStream->direction;
		PAEngine *engine = OSDynamicCast(PAEngine, audioEngine);
		strncpy(info.name, str->getCStringNoCopy(), sizeof(info.name) - 1);

		if (dir == kIOAudioStreamDirectionInput) {
			info.channelsIn = clientBuffer->numChannels;
			if (info.audioContentType == kPADeviceAudioContentIndividual)
				engine->addVirtualDevice(&info, clientBuffer->sourceBufferDescriptor, NULL, clientBuffer);
		} else {
			info.channelsOut = clientBuffer->numChannels;
			if (info.audioContentType == kPADeviceAudioContentIndividual)
				engine->addVirtualDevice(&info, NULL, clientBuffer->sourceBufferDescriptor, clientBuffer);
		}
	}

	prop->release();

	return ret;
}

void
PAStream::removeClient(IOAudioClientBuffer *clientBuffer)
{
	debugFunctionEnter();
	
	PAEngine *engine = OSDynamicCast(PAEngine, audioEngine);
	if (engine)
		engine->removeVirtualDeviceWithRefcon(clientBuffer);

	super::removeClient(clientBuffer);	
}
