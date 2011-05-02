/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@class PAServerConnection;

@interface PASourceOutputInfo : NSObject
{
	PAServerConnection *server;

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

@property (nonatomic, readonly) PAServerConnection *server;

@property (nonatomic, readonly) UInt32 index;
@property (nonatomic, readonly) UInt32 bufferUsec;
@property (nonatomic, readonly) UInt32 sourceUsec;

@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) NSString *resampleMethod;
@property (nonatomic, readonly) NSString *driver;

@property (nonatomic, readonly) NSArray *channelNames;
@property (nonatomic, readonly) NSDictionary *properties;

@property (nonatomic, readonly) BOOL corked;

@end
