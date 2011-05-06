/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import "PAElementInfo.h"

@interface PASinkInputInfo : PAElementInfo
{
        UInt32 volume;
        UInt32 bufferUsec;
        UInt32 sinkUsec;

        NSString *resampleMethod;
        NSString *driver;

        NSArray *channelNames;
        NSDictionary *properties;

        BOOL muted;
        BOOL volumeWriteable;
}

@property (nonatomic, readwrite) UInt32 volume;
@property (nonatomic, readonly) UInt32 bufferUsec;
@property (nonatomic, readonly) UInt32 sinkUsec;

@property (nonatomic, readonly) NSString *resampleMethod;
@property (nonatomic, readonly) NSString *driver;

@property (nonatomic, readonly) NSArray *channelNames;
@property (nonatomic, readonly) NSDictionary *properties;

@property (nonatomic, readwrite) BOOL muted;
@property (nonatomic, readonly) BOOL volumeWriteable;

@end

