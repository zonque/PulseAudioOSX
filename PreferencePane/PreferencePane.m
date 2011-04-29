/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PreferencePane.h"

@implementation PreferencePane

- (void) didSelect
{
}

- (void) didUnselect
{
}

- (void) mainViewDidLoad
{
	helperConnection = [[PAHelperConnection alloc] init];
	helperConnection.delegate = self;
	if (![helperConnection connect]) {
	//	[NSApp terminate: nil];
	}
	
	growl.delegate = self;
	
	NSDictionary *preferences = [helperConnection.serverProxy getPreferences];

	[self PAHelperConnection: helperConnection
	      preferencesChanged: preferences];
	 
}

#pragma mark ### GUI ###
- (IBAction) setStatusBarEnabled: (id) sender
{
	NSButton *button = sender;
	BOOL enabled = ([button state] == NSOnState);

	[helperConnection.serverProxy setPreferences: [NSNumber numberWithBool: enabled]
					      forKey: @"statusBarEnabled"];
}

- (IBAction) setPulseAudioEnabled: (id) sender
{
	[loginItemController toggleLoginItem: sender];
}

#pragma mark ### PAHelperConnectionDelegate ###

- (void) PAHelperConnectionDied: (PAHelperConnection *) connection
{
	NSLog(@"%s()", __func__);
}

- (void) PAHelperConnection: (PAHelperConnection *) connection
	audioClientsChanged: (NSArray *) array
{
	[audioClients audioClientsChanged: array];
}

- (void) PAHelperConnection: (PAHelperConnection *) connection
	 preferencesChanged: (NSDictionary *) preferences
{
	BOOL statusBarEnabled = [[preferences objectForKey: @"statusBarEnabled"] boolValue];
	[statusBarEnabledButton setState: statusBarEnabled];
	
	[growl preferencesChanged: preferences];
}

#pragma mark ### GrowlDelegate ###

- (void) setPreferences: (id) value
		 forKey: (NSString *) key
{
	[helperConnection.serverProxy setPreferences: value
					      forKey: key];
}

@end
