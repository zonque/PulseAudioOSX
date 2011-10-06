/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>

#import "PulseAudio.h"
#import "PAServerConnectionAudio.h"

#import "PACardInfoInternal.h"
#import "PAClientInfoInternal.h"
#import "PAModuleInfoInternal.h"
#import "PASampleInfoInternal.h"
#import "PAServerInfoInternal.h"
#import "PASinkInfoInternal.h"
#import "PASinkInputInfoInternal.h"
#import "PASourceInfoInternal.h"
#import "PASourceOutputInfoInternal.h"

@interface PAServerConnection (internal)

+ (NSDictionary *) createDictionaryFromProplist: (pa_proplist *) plist;
+ (NSArray *) createChannelNamesArray: (const pa_channel_map *) map;

- (void) setAudioStarted;
- (BOOL) unloadModule: (PAModuleInfo *) module;

@end
