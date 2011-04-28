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

@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *sampleSpec;
@property (nonatomic, retain) NSString *channelMap;
@property (nonatomic, retain) NSString *fileName;
@property (nonatomic, assign) UInt64 duration;
@property (nonatomic, assign) UInt64 bytes;
@property (nonatomic, assign) BOOL lazy;


@end
