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
#import "ConnectionServer.h"
#import "Preferences.h"

@implementation ConnectionClient

@synthesize audioClients;
@synthesize delegate;

- (ULINetSocket *) socket
{
    return connection.socket;
}

- (id) initWithSocket: (ULINetSocket *) _socket
            forServer: (ConnectionServer *) _server;

{
    [super init];
    
    lock = [[NSLock alloc] init];
    server = [_server retain];
    audioClients = [[NSMutableArray arrayWithCapacity: 0] retain];
	
    connection = [[PAHelperConnection alloc] initWithSocket: _socket];
    [connection setDelegate: self];
    [connection scheduleOnCurrentRunLoop];
    [connection retain];
    
    NSLog(@"%s(): new client for socket %p", __func__, _socket);
    
    return self;
}

- (void) dealloc
{
    [connection release];
    [audioClients release];
    [server release];
    [lock release];
    
    [super dealloc];
}

- (void) setConfig: (NSDictionary *) config
 forDeviceWithUUID: (NSString *) uuid
{
    NSLog(@"%s(%p) uuid %@", __func__, self, uuid);
    
    [lock lock];
    
    for (NSDictionary *client in audioClients) {
        NSLog(@" client uuid: %@", [client objectForKey: @"uuid"]);
        if ([[client objectForKey: @"uuid"] isEqualToString: uuid]) {
			NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary: client];
			[dict addEntriesFromDictionary: config];
			NSLog(@" dict = %@", dict);
			[connection sendMessage: PAOSX_MessageSetAudioClientConfig
                               dict: dict];
		}
    }
    
    [lock unlock];
}

- (void) sendAudioClientsChanged: (NSArray *) clients
{
    [lock lock];
	NSLog(@"%s(%p): clients %@", __func__, self, clients);
	[connection sendMessage: PAOSX_MessageAudioClientsUpdate
                       dict: [NSDictionary dictionaryWithObject: clients
                                                         forKey: @"clients"]];
    [lock unlock];
}

- (void) sendPreferencesChanged: (NSDictionary *) preferences
{
    [lock lock];
	[connection sendMessage: PAOSX_MessageSetPreferences
                       dict: preferences];	
    [lock unlock];
}

- (void) PAHelperConnectionEstablished: (PAHelperConnection *) c
{
    NSDictionary *p = server.preferences.preferencesDict;
    [connection sendMessage: PAOSX_MessageSetPreferences
                       dict: p];
	
    [connection sendMessage: PAOSX_MessageAudioClientsUpdate
                       dict: [NSDictionary dictionaryWithObject: [delegate allAudioClients]
                                                         forKey: @"clients"]];
}

- (void) PAHelperConnectionDied: (PAHelperConnection *) c
{
    NSLog(@"%s(): connection %p", __func__, c);
    [delegate connectionClientDied: self];
}

- (void) PAHelperConnection: (PAHelperConnection *) c
            receivedMessage: (NSString *) name
                       dict: (NSDictionary *) dict
{
    NSLog(@"%s(): %@ -> %@", __func__, name, dict);
	
    if ([name isEqualToString: PAOSX_MessageAudioClientStarted]) {
        NSString *deviceName = [dict objectForKey: @"deviceName"];
        BOOL found = NO;
		
        for (NSDictionary *client in audioClients)
            if ([[client objectForKey: @"deviceName"] isEqualToString: deviceName])
                found = YES;
        
        if (!found) {
            NSMutableDictionary *client = [NSMutableDictionary dictionaryWithDictionary: dict];
            NSString *uuid = [NSString stringWithFormat: @"%p.%@", self, deviceName];
            
            [client setObject: uuid
                       forKey: @"uuid"];
			
            [audioClients addObject: client];
            NSLog(@" adding new audioclient. count %d", [audioClients count]);
            [delegate connectionClientChangedAudioClients: self];
        }
    }
    
    if ([name isEqualToString: PAOSX_MessageAudioClientStopped]) {
        NSString *deviceName = [dict objectForKey: @"deviceName"];
		
        for (NSDictionary *client in audioClients)
            if ([[client objectForKey: @"deviceName"] isEqualToString: deviceName]) {
                [audioClients removeObject: client];
                [delegate connectionClientChangedAudioClients: self];
                return;
            }
    }
	
    if ([name isEqualToString: PAOSX_MessageSetPreferences])
        [delegate connectionClient: self
                changedPreferences: dict];
	
    if ([name isEqualToString: PAOSX_MessageSetAudioClientConfig]) {
        NSString *uuid = [dict objectForKey: @"uuid"];
        NSDictionary *config = [dict objectForKey: @"config"];
        [delegate connectionClient: self
              setAudioClientConfig: config
                           forUUID: uuid];
	}
}

@end
