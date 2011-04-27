/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PASampleInfo.h"


@implementation PASampleInfo

@synthesize name;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize fileName;
@synthesize duration;
@synthesize bytes;
@synthesize lazy;

@end
