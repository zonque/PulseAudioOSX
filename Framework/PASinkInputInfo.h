/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@interface PASinkInputInfo : NSObject
{
	UInt32 index;
	UInt32 volume;
	UInt32 bufferUsec;
	UInt32 sinkUsec;
	
	NSString *name;
	NSString *resampleMethod;
	NSString *driver;

	NSArray *channelNames;
	NSDictionary *properties;

	BOOL muted;
	BOOL volumeWriteable;
}

@property (nonatomic, assign) UInt32 index;
@property (nonatomic, assign) UInt32 volume;
@property (nonatomic, assign) UInt32 bufferUsec;
@property (nonatomic, assign) UInt32 sinkUsec;

@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *resampleMethod;
@property (nonatomic, retain) NSString *driver;

@property (nonatomic, retain) NSArray *channelNames;
@property (nonatomic, retain) NSDictionary *properties;

@property (nonatomic, assign) BOOL muted;
@property (nonatomic, assign) BOOL volumeWriteable;

@end
