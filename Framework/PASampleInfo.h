/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@interface PASampleInfo : NSObject
{
	NSString *name;
	NSString *sampleSpec;
	NSString *channelMap;
	NSString *fileName;
	
	UInt64 duration;
	UInt64 bytes;
	
	BOOL lazy;
}

@property (assign) NSString *name;
@property (assign) NSString *sampleSpec;
@property (assign) NSString *channelMap;
@property (assign) NSString *fileName;
@property (assign) UInt64 duration;
@property (assign) UInt64 bytes;
@property (assign) BOOL lazy;


@end
