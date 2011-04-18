/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ServerTask.h"

#define REMOTE_OBJECT @"PulseAudioPreferencePane"

@implementation ServerTask

- (void) setPreferences: (Preferences *) newPrefs
{
	prefs = newPrefs;
}

- (void) start
{
	NSString *path = @"/Library/Frameworks/pulse.framework/Resources/bin/pulseaudio";
	NSArray *args = [NSArray arrayWithObjects:
			 @"-n",
			 @"-F", @"/Users/daniel/src/pa/pulseaudio/src/macosx.pa",
			 nil];
	task = [NSTask launchedTaskWithLaunchPath: path
					arguments: args];
}

- (void) stop
{
	if (task) {
		[task terminate];
		[task waitUntilExit];
		task = nil;
	}
}

- (void) setLocalServerEnabled: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	BOOL enabled = [[userInfo objectForKey: @"enabled"] boolValue];
	
	if (enabled && !task)
		[self start];
	
	if (!enabled && task)
		[self stop];
}

- (id) init
{
	[super init];
	
	[[prefs notificationCenter] addObserver: self
				       selector: @selector(setLocalServerEnabled:)
					   name: @"setLocalServerEnabled"
					 object: REMOTE_OBJECT];	

	if ([prefs isLocalServerEnabled])
		[self start];

	return self;
}

@end
