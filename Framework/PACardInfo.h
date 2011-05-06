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

@interface PACardInfo : PAElementInfo
{
        NSString *driver;
        NSDictionary *properties;
}

@property (nonatomic, readonly) NSString *driver;
@property (nonatomic, readonly) NSDictionary *properties;

@end
