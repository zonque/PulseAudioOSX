/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@interface PAServerInfo : NSObject
{
	NSString *userName;
	NSString *hostName;
	NSString *serverName;
	NSString *version;
	NSString *sampleSpec;
	NSString *channelMap;
	NSString *defaultSinkName;
	NSString *defaultSourceName;
	
	UInt32 cookie;
}

@property (assign) NSString *userName;
@property (assign) NSString *hostName;
@property (assign) NSString *serverName;
@property (assign) NSString *version;
@property (assign) NSString *sampleSpec;
@property (assign) NSString *channelMap;
@property (assign) NSString *defaultSinkName;
@property (assign) NSString *defaultSourceName;
@property (assign) UInt32 cookie;

@end
