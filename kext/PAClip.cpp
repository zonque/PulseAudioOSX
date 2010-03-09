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
PAEngine::clipOutputSamples(const void *inMixBuffer, void *outTargetBuffer,
			    UInt32 inFirstFrame, UInt32 inNumberFrames,
			    const IOAudioStreamFormat *inFormat, IOAudioStream *inStream)
{
	memcpy((float *) outTargetBuffer + inFirstFrame,
		(float *) inMixBuffer + inFirstFrame,
		inNumberFrames * sizeof(float));

	return kIOReturnSuccess;
}

IOReturn
PAEngine::convertInputSamples(const void *inSourceBuffer, void *outTargetBuffer,
			      UInt32 inFirstFrame, UInt32 inNumberFrames,
			      const IOAudioStreamFormat *inFormat, IOAudioStream *inStream)
{
	memcpy((float *) outTargetBuffer + inFirstFrame,
		(float *) inSourceBuffer + inFirstFrame,
		inNumberFrames * sizeof(float));

	return kIOReturnSuccess;
}

