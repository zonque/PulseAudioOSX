/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAHelperConnection.h"

@implementation PAHelperConnection

@synthesize name;
@synthesize delegate;
@synthesize serverProxy;

#pragma mark ### NotificationCenter callbacks ###

- (void) connectionDidDie: (NSNotification *) notification
{
        NSLog(@"%s()", __func__);

        if (serverProxy)
                [serverProxy release];

        serverProxy = nil;

        if (delegate)
                [delegate PAHelperConnectionDied: self];
}

- (void) serverMessage: (NSNotification *) notification
{
        NSDictionary *userInfo = [notification userInfo];
        NSString *command = [userInfo objectForKey: @"command"];

        if (!delegate)
                return;

        if ([command isEqualToString: @"audioClientsChanged"]) {
                NSArray *audioClients = [userInfo objectForKey: @"audioClients"];

                if ([delegate respondsToSelector: @selector(PAHelperConnection:audioClientsChanged:)])
                        [delegate PAHelperConnection: self
                                 audioClientsChanged: audioClients];
        }

        if ([command isEqualToString: @"preferencesChanged"]) {
                NSDictionary *preferences = [userInfo objectForKey: @"preferences"];

                if ([delegate respondsToSelector: @selector(PAHelperConnection:preferencesChanged:)])
                        [delegate PAHelperConnection: self
                                  preferencesChanged: preferences];
        }

        if ([command isEqualToString: @"setAudioDeviceConfig"]) {
                NSDictionary *config = [userInfo objectForKey: @"config"];
                NSString *deviceName = [userInfo objectForKey: @"deviceName"];

                if ([delegate respondsToSelector: @selector(PAHelperConnection:setConfig:forDeviceWithName:)])
                        [delegate PAHelperConnection: self
                                           setConfig: config
                                   forDeviceWithName: deviceName];
        }
}

#pragma mark ### vended selectors ###

- (void) setConfig: (NSDictionary *) config
 forDeviceWithName: (NSString *) _name
{
        if (delegate &&
            [delegate respondsToSelector: @selector(PAHelperConnection:setConfig:forDeviceWithName:)])
                [delegate PAHelperConnection: self
                                   setConfig: config
                           forDeviceWithName: _name];
}

#pragma mark PAHelperConnection ###

- (BOOL) connect
{
        if (serverProxy)
                return YES;

        serverProxy = [NSConnection rootProxyForConnectionWithRegisteredName: PAOSX_HelperName
                                                                        host: nil];
        if (!serverProxy)
                return NO;

        // connect to the server
        NSConnection *connection = [serverProxy connectionForProxy];
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(connectionDidDie:)
                                                     name: NSConnectionDidDieNotification
                                                   object: connection];

        [serverProxy setProtocolForProxy: @protocol(PAHelperConnection)];
        [serverProxy retain];

        // make up a new name ...
        name = [NSString stringWithFormat: @"%@.%p", [[NSProcessInfo processInfo] globallyUniqueString], self];

        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
                                                            selector: @selector(serverMessage:)
                                                                name: name
                                                              object: PAOSX_HelperName];

        // ... tell the server about it, so it can connect back
        [serverProxy registerClientWithName: name];

        return YES;
}

- (BOOL) isConnected
{
        return serverProxy != nil;
}

@end
