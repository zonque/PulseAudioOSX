/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>

@interface Preferences : NSObject {
        NSMutableDictionary *preferencesDict;
        BOOL growlReady;
}

@property (nonatomic, readonly) NSDictionary *preferencesDict;

- (id) valueForKey: (NSString *) key;
- (void) setValue: (id) value
           forKey: (NSString *) key;

/* Growl Notifications */
- (BOOL) isGrowlEnabled;
- (UInt64) growlNotificationFlags;
- (void) setGrowlReady: (BOOL) ready;

/* Local Server */
- (BOOL) isLocalServerEnabled;

/* Status Bar */
- (BOOL) isStatusBarEnabled;

@end
