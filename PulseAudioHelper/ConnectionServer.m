/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PulseAudio/PAHelperConnection.h>

#import "ConnectionServer.h"
#import "ConnectionClient.h"

@implementation ConnectionServer

@synthesize currentConnection;

@synthesize preferences;

- (ConnectionClient *) findClientByConnection: (NSConnection *) connection
{
        for (ConnectionClient *c in clientArray)
                if (c.connection == connection)
                        return c;

        return nil;
}

- (void) audioClientsChanged
{
        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];

        for (ConnectionClient *c in clientArray)
                [array addObjectsFromArray: c.audioClients];

        for (ConnectionClient *c in clientArray)
                [c audioClientsChanged: array];
}

- (void) preferencesChanged
{
        for (ConnectionClient *c in clientArray)
                if (c.connection != currentConnection)
                        [c preferencesChanged: preferences.preferencesDict];
}

#pragma mark ### PAHelperConnection protocol ###

- (void) registerClientWithName: (NSString *) name
{
        ConnectionClient *c = [self findClientByConnection: currentConnection];

        if (c)
                [c registerClientWithName: name];
}

- (void) announceDevice: (NSDictionary *) device
{
        ConnectionClient *c = [self findClientByConnection: currentConnection];

        if (c) {
                [c announceDevice: device];
                [self audioClientsChanged];
        }
}

- (void) signOffDevice: (NSString *) signedOffName
{
        ConnectionClient *c = [self findClientByConnection: currentConnection];

        if (c) {
                [c signOffDevice: signedOffName];
                [self audioClientsChanged];
        }
}

- (void) setConfig: (NSDictionary *) config
 forDeviceWithUUID: (NSString *) uuid
{
        ConnectionClient *c = [self findClientByConnection: currentConnection];

        if (c)
                [c setConfig: config
           forDeviceWithUUID: uuid];
}

- (NSDictionary *) getPreferences
{
        return preferences.preferencesDict;
}

- (void) setPreferences: (id) object
                 forKey: (NSString *) key
{
        [preferences setValue: object
                       forKey: key];
        [self preferencesChanged];
}

#pragma mark ### NSConnection callbacks ###

- (void) connectionDied: (NSNotification *) notification
{
        NSConnection *connection = [notification object];
        ConnectionClient *c = [self findClientByConnection: connection];

        if (c) {
                BOOL hadAudioClients = ([c.audioClients count] > 0);
                [c release];
                [clientArray removeObject: c];

                if (hadAudioClients)
                        [self audioClientsChanged];
        }
}

- (void) connectionInitialized: (NSNotification *) notification
{
        NSConnection *connection = [notification object];
        ConnectionClient *c = [[ConnectionClient alloc] initWithConnection: connection];
        [clientArray addObject: c];
}

- (void) start
{
        clientArray = [[NSMutableArray arrayWithCapacity: 0] retain];

        serviceConnection = [NSConnection serviceConnectionWithName: PAOSX_HelperName
                                                         rootObject: self];
        [serviceConnection setDelegate: self];
        [serviceConnection addRunLoop: [NSRunLoop currentRunLoop]];
        [serviceConnection retain];
}

#pragma mark ### NSConnectionDelegate ###

- (BOOL) connection: (NSConnection *) parentConnection
shouldMakeNewConnection: (NSConnection *) newConnnection
{

        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(connectionInitialized:)
                                                     name: NSConnectionDidInitializeNotification
                                                   object: newConnnection];
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(connectionDied:)
                                                     name: NSConnectionDidDieNotification
                                                   object: newConnnection];

        return YES;
}

- (BOOL) connection: (NSConnection *) conn
      handleRequest: (NSDistantObjectRequest *) doReq
{
        ConnectionServer *server = [conn rootObject];
        server.currentConnection = conn;
        return NO;
}

@end
