/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@interface PASourceInfo : NSObject {
	NSString *name;
	NSString *description;
	NSString *sampleSpec;
	NSString *channelMap;
	NSString *driver;
	
	UInt32 latency;
	UInt32 configuredLatency;
	
	NSDictionary *properties;
}

@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *description;
@property (nonatomic, retain) NSString *sampleSpec;
@property (nonatomic, retain) NSString *channelMap;
@property (nonatomic, retain) NSString *driver;
@property (nonatomic, assign) UInt32 latency;
@property (nonatomic, assign) UInt32 configuredLatency;
@property (nonatomic, retain) NSDictionary *properties;

@end
