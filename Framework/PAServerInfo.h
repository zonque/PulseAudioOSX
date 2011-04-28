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

@property (nonatomic, retain) NSString *userName;
@property (nonatomic, retain) NSString *hostName;
@property (nonatomic, retain) NSString *serverName;
@property (nonatomic, retain) NSString *version;
@property (nonatomic, retain) NSString *sampleSpec;
@property (nonatomic, retain) NSString *channelMap;
@property (nonatomic, retain) NSString *defaultSinkName;
@property (nonatomic, retain) NSString *defaultSourceName;
@property (nonatomic, assign) UInt32 cookie;

@end
