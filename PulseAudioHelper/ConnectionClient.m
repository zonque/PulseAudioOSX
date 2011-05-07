/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <PulseAudio/PAHelperConnection.h>

#import "ConnectionClient.h"

@implementation ConnectionClient

@synthesize connection;
@synthesize audioClients;

- (id) initWithConnection: (NSConnection *) c
{
        [super init];

        connection = c;
        [connection retain];

        audioClients = [[NSMutableArray arrayWithCapacity: 0] retain];

        return self;
}

- (void) dealloc
{
        [connection release];
        if (connectionName)
                [connectionName release];

        [audioClients release];

        [super dealloc];
}

- (void) registerClientWithName: (NSString *) name
{
        connectionName = [name retain];
}

- (void) announceDevice: (NSDictionary *) device
{
        NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary: device];
        NSString *deviceName = [dict objectForKey: @"deviceName"];

        NSLog(@"%s() self = %p", __func__, self);

        [dict setObject: [NSString stringWithFormat: @"%@.%@", connectionName, deviceName]
                                             forKey: @"uuid"];
        [audioClients addObject: dict];
}

- (void) signOffDevice: (NSString *) signedOffName
{
        for (NSDictionary *client in audioClients)
                if ([[client objectForKey: @"deviceName"] isEqualToString: signedOffName]) {
                        [audioClients removeObject: client];
                        return;
                }
}

- (void) setConfig: (NSDictionary *) config
 forDeviceWithUUID: (NSString *) uuid
{
        NSLog(@"%s() self = %p", __func__, self);
        NSLog(@"client server %s() ,,, uuid %@ ... %d clients", __func__, uuid, [audioClients count]);

        for (NSDictionary *client in audioClients) {
                NSLog(@" .. client %@", client);
                if ([[client objectForKey: @"uuid"] isEqualToString: uuid]) {
                        NSString *deviceName = [client objectForKey: @"deviceName"];
                        NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];

                        NSLog(@"client server %s()", __func__);

                        [userInfo setObject: @"setAudioDeviceConfig"
                                     forKey: @"command"];
                        [userInfo setObject: config
                                     forKey: @"config"];
                        [userInfo setObject: deviceName
                                     forKey: @"deviceName"];

                        [[NSDistributedNotificationCenter defaultCenter] postNotificationName: connectionName
                                                                                       object: PAOSX_HelperName
                                                                                     userInfo: userInfo];
                }
        }
}

- (void) audioClientsChanged : (NSArray *) clients
{
        if (!connectionName)
                return;

        NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
        [userInfo setObject: @"audioClientsChanged"
                     forKey: @"command"];
        [userInfo setObject: clients
                     forKey: @"audioClients"];

        [[NSDistributedNotificationCenter defaultCenter] postNotificationName: connectionName
                                                                       object: PAOSX_HelperName
                                                                     userInfo: userInfo];
}

- (void) preferencesChanged : (NSDictionary *) preferences
{
        if (!connectionName)
                return;

        NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
        [userInfo setObject: @"preferencesChanged"
                     forKey: @"command"];
        [userInfo setObject: preferences
                     forKey: @"preferences"];

        [[NSDistributedNotificationCenter defaultCenter] postNotificationName: connectionName
                                                                       object: PAOSX_HelperName
                                                                     userInfo: userInfo];
}

@end
