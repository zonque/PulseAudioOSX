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
	
        if (![helperConnection connectWithRetry: NO]) {
		//FIXME
                [NSApp terminate: nil];
        }

        growl.delegate = self;
        localServer.delegate = self;
        audioClients.delegate = self;

	/*
        NSArray *currentAudioDevices = [helperConnection.serverProxy currentAudioDevices];
        
        CFShow(currentAudioDevices);
        
        [audioClients audioClientsChanged: currentAudioDevices];
        
        NSDictionary *preferences = [helperConnection.serverProxy getPreferences];
	 */	
}

#pragma mark ### GUI ###
- (IBAction) setStatusBarEnabled: (id) sender
{
        NSButton *button = sender;
        BOOL enabled = ([button state] == NSOnState);

	NSDictionary *p = [NSDictionary dictionaryWithObject: [NSNumber numberWithBool: enabled]
						      forKey: @"statusBarEnabled"];
	[helperConnection sendMessage: PAOSX_MessageSetPreferences
				 dict: p];
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

/*
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
*/

- (void) PAHelperConnection: (PAHelperConnection *) connection
	    receivedMessage: (NSString *) name
		       dict: (NSDictionary *) msg
{
	NSLog(@"%s() %@ -> %@", __func__, name, msg);
	
	if ([name isEqualToString: PAOSX_MessageAudioClientsUpdate])
		[audioClients audioClientsChanged: [msg objectForKey: @"clients"]];

	if ([name isEqualToString: PAOSX_MessageSetPreferences]) {
		BOOL statusBarEnabled = [[msg objectForKey: @"statusBarEnabled"] boolValue];
		[statusBarEnabledButton setState: statusBarEnabled];
		[growl preferencesChanged: msg];
	}
}

#pragma mark ### GrowlDelegate ###

- (void) setPreferences: (id) value
                 forKey: (NSString *) key
{
	[helperConnection sendMessage: PAOSX_MessageSetPreferences
				 dict: [NSDictionary dictionaryWithObject: value
								   forKey: key]];
}

#pragma mark ### AudioClientsDelegate ###

- (void) setAudioDeviceConfig: (NSDictionary *) config
            forDeviceWithUUID: (NSString *) uuid
{
        NSLog(@"%s() uuid %@ config %@", __func__, uuid, config);
	NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[dict setObject: config
		 forKey: @"config"];
	[dict setObject: uuid
		 forKey: @"uuid"];
	
	[helperConnection sendMessage: PAOSX_MessageSetAudioClientConfig
				 dict: dict];
}

@end
