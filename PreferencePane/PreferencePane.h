/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PreferencePanes/PreferencePanes.h>
#import <PulseAudio/PAHelperConnection.h>

#import "LoginItemController.h"
#import "AudioClients.h"
#import "LocalServer.h"
#import "Growl.h"

@interface PreferencePane : NSPreferencePane <
                                                PAHelperConnectionDelegate,
                                                GrowlDelegate,
                                                LocalServerDelegate,
                                                AudioClientsDelegate>
{
        IBOutlet NSButton *statusBarEnabledButton;
        IBOutlet LoginItemController *loginItemController;

        IBOutlet AudioClients *audioClients;
        IBOutlet Growl *growl;
        IBOutlet LocalServer *localServer;

        PAHelperConnection *helperConnection;
}

- (IBAction) setPulseAudioEnabled: (id) sender;
- (IBAction) setStatusBarEnabled: (id) sender;

#pragma mark ### PAHelperConnectionDelegate ###

- (void) PAHelperConnectionDied: (PAHelperConnection *) connection;
- (void) PAHelperConnection: (PAHelperConnection *) connection
	    receivedMessage: (NSString *) name
		       dict: (NSDictionary *) msg;

#pragma mark ### GrowlDelegate/LocalServerDelegate ###

- (void) setPreferences: (id) value
                 forKey: (NSString *) key;

#pragma mark ### AudioClientsDelegate ###

- (void) setAudioDeviceConfig: (NSDictionary *) config
            forDeviceWithUUID: (NSString *) uuid;

@end
