/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>

#define LOCAL_OBJECT @"PulseAudioHelper"
#define REMOTE_OBJECT_PREFPANE @"PulseAudioPreferencePane"
#define REMOTE_OBJECT_HALPLUGIN @"PAHP_Device"

@interface Preferences : NSObject {
	NSDistributedNotificationCenter *notificationCenter;
	NSMutableDictionary *preferencesDict;
	
	BOOL growlReady;
}

- (NSDistributedNotificationCenter *) notificationCenter;

/* Growl Notifications */
- (BOOL) isGrowlEnabled;
- (UInt64) growlNotificationFlags;
- (void) setGrowlReady: (BOOL) ready;

/* Local Server */
- (BOOL) isLocalServerEnabled;

/* Status Bar */
- (BOOL) isStatusBarEnabled;

@end
