/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#define PATHHACK "/Users/daniel/src/pa/PulseAudioOSX/PulseAudioHelper/"

#define REMOTE_OBJECT @"PulseAudioPreferencePane"
#define LOCAL_OBJECT @"PulseAudioHelper"

#import "StatusBar.h"


@implementation StatusBar

- (NSMenu *) createMenu
{
	NSZone *zone = [NSMenu menuZone];
        NSMenu *m = [[NSMenu allocWithZone: zone] init];

        NSMenuItem *item;
	
        item = [m addItemWithTitle: @"xxx"
			    action: @selector(startGrowl:)
		     keyEquivalent: @""];
        [item setTarget: self];
        [item setTag: 1];

	return m;
}

- (void) showMenu
{
	statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];	
	
	NSMenu *m = [self createMenu];
	[statusItem setMenu: m]; // retains m
        [statusItem setToolTip: @"PulseAudio"];
        [statusItem setHighlightMode: YES];
	[statusItem setImage: icon];

        [m release];
}

- (void) hideMenu
{
	if (statusItem) {
		[[NSStatusBar systemStatusBar] removeStatusItem: statusItem];
		[statusItem release];
		statusItem = nil;
	}
}

#pragma mark ### NSDistributedNotificationCenter ###

- (void) setEnabled: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	BOOL enabled = [[userInfo objectForKey: @"enabled"] boolValue];
	
	if (enabled && !statusItem)
		[self showMenu];
	
	if (!enabled && statusItem)
		[self hideMenu];
}

- (void) queryEnabled: (NSNotification *) notification
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	BOOL enabled = (statusItem != nil);
	
	[userInfo setObject: [NSNumber numberWithBool: enabled]
		     forKey: @"enabled"];
	
	[notificationCenter postNotificationName: @"setStatusBarEnabled"
					  object: LOCAL_OBJECT
					userInfo: userInfo
			      deliverImmediately: YES];	
}

#pragma mark ### NSApplicationDelegate ###

- (void) applicationDidFinishLaunching: (NSNotification *) aNotification
{
	icon = [[NSImage alloc] initWithContentsOfFile: @PATHHACK"statusBar.png"];

	[self showMenu];
	
	notificationCenter = [[NSDistributedNotificationCenter defaultCenter] retain];
	
	[notificationCenter addObserver: self
			       selector: @selector(setEnabled:)
				   name: @"setStatusBarEnabled"
				 object: REMOTE_OBJECT];	

	[notificationCenter addObserver: self
			       selector: @selector(queryEnabled:)
				   name: @"queryStatusBarEnabled"
				 object: REMOTE_OBJECT];	
	
}

@end
