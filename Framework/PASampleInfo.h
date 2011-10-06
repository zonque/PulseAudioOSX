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

@interface PASampleInfo : PAElementInfo
{
    NSString *sampleSpec;
    NSString *channelMap;
    NSString *fileName;
    
    UInt64 duration;
    UInt64 bytes;
    
    BOOL lazy;
}

@property (nonatomic, readonly) NSString *sampleSpec;
@property (nonatomic, readonly) NSString *channelMap;
@property (nonatomic, readonly) NSString *fileName;
@property (nonatomic, readonly) UInt64 duration;
@property (nonatomic, readonly) UInt64 bytes;
@property (nonatomic, readonly) BOOL lazy;

@end
