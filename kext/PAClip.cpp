/***
 This file is part of PulseAudioKext
 
 Copyright (c) 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include "PAEngine.h"

#include <IOKit/IOLib.h>
#include <IOKit/audio/IOAudioStream.h>

IOReturn
PAEngine::clipOutputSamples(const void *mixBuffer, void *targetBuffer,
			    UInt32 firstSampleFrame, UInt32 numberFrames,
			    const IOAudioStreamFormat *streamFormat, IOAudioStream *stream)
{
	UInt32 sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);

	memcpy((float *) targetBuffer + sampleIndex,
		(float *) mixBuffer + sampleIndex,
		numberFrames * streamFormat->fNumChannels * sizeof(float));

	return kIOReturnSuccess;
}

IOReturn
PAEngine::convertInputSamples(const void *sourceBuffer, void *targetBuffer,
			      UInt32 firstSampleFrame, UInt32 numberFrames,
			      const IOAudioStreamFormat *streamFormat, IOAudioStream *stream)
{
	UInt32 sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);

	memcpy(targetBuffer, (float *) sourceBuffer + sampleIndex,
		numberFrames * streamFormat->fNumChannels * sizeof(float));

	return kIOReturnSuccess;
}

