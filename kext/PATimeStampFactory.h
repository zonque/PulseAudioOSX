/***
 This file is part of PulseAudioKext
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioKext is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PATIMESTAMPFACTORY_H
#define PATIMESTAMPFACTORY_H

class PATimeStampFactory
{
public:
	PATimeStampFactory(UInt32 initSampleFrames, UInt32 initSampleRate);
	~PATimeStampFactory();

	void	setSampleRate(UInt32 newSampleRate);
	UInt32	getCurrentSampleFrame();

private:
	UInt32	sampleRate;
	UInt32	numSampleFrames;

};

#endif /* PATIMESTAMPFACTORY_H */