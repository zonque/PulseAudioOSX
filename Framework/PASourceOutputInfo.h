/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@interface PASourceOutputInfo : NSObject
{
	UInt32 index;
	UInt32 bufferUsec;
	UInt32 sourceUsec;
	
	NSString *name;
	NSString *resampleMethod;
	NSString *driver;

	NSArray *channelNames;
	NSDictionary *properties;
	
	BOOL corked;
}

@property (nonatomic, assign) UInt32 index;
@property (nonatomic, assign) UInt32 bufferUsec;
@property (nonatomic, assign) UInt32 sourceUsec;

@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *resampleMethod;
@property (nonatomic, retain) NSString *driver;

@property (nonatomic, retain) NSArray *channelNames;
@property (nonatomic, retain) NSDictionary *properties;

@property (nonatomic, assign) BOOL corked;

@end
