/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <libkern/OSBase.h>
#include <libkern/OSTypes.h>
#include <libkern/libkern.h>
#include <kern/clock.h>

#include "PATimeStampFactory.h"

PATimeStampFactory::PATimeStampFactory(UInt32 initSampleFrames, UInt32 initSampleRate)
	:	sampleRate(initSampleRate), numSampleFrames(initSampleFrames)
{
}

PATimeStampFactory::~PATimeStampFactory()
{

}

void PATimeStampFactory::setSampleRate(UInt32 newSampleRate)
{
	if (newSampleRate == sampleRate)
		return;

	sampleRate = newSampleRate;

	// reset factory
}

UInt32 PATimeStampFactory::getCurrentSampleFrame()
{
	return 0;
}
