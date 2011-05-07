/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import "PAElementInfo.h"

@interface PAServerInfo : PAElementInfo
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

@property (nonatomic, readonly) NSString *userName;
@property (nonatomic, readonly) NSString *hostName;
@property (nonatomic, readonly) NSString *serverName;
@property (nonatomic, readonly) NSString *version;
@property (nonatomic, readonly) NSString *sampleSpec;
@property (nonatomic, readonly) NSString *channelMap;
@property (nonatomic, readonly) NSString *defaultSinkName;
@property (nonatomic, readonly) NSString *defaultSourceName;
@property (nonatomic, readonly) UInt32 cookie;

@end
