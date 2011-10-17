/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PulseAudio/PAHelperConnection.h>

#include <sys/types.h>
#include <sys/stat.h>

#import "ConnectionServer.h"
#import "ConnectionClient.h"

@implementation ConnectionServer

@synthesize currentConnection;
@synthesize preferences;

- (void) start
{
    clientArray = [[NSMutableArray arrayWithCapacity: 0] retain];
    
	serviceSocket = [[ULINetSocket alloc] init];
	[serviceSocket setDelegate: self];
	[serviceSocket listenOnLocalSocketPath: PAOSX_HelperSocket
                     maxPendingConnections: 100];
    /* coreaudiod runs under the _coreaudiod user's permission, so we
     * have to tweak the socket permissions and allow any user to connect
     */
    chmod([PAOSX_HelperSocket cStringUsingEncoding: NSASCIIStringEncoding], S_IRUSR | S_IWUSR |
                                               S_IRGRP | S_IWGRP |
                                               S_IROTH | S_IWOTH);
	[serviceSocket scheduleOnCurrentRunLoop];
    NSLog(@"PAOSX_HelperSocket: %@", PAOSX_HelperSocket);
}

#pragma mark ### ULINetSocketDelegate ###

-(void)	netsocketDisconnected: (ULINetSocket *) inNetSocket
{
	for (ConnectionClient *c in clientArray)
		if (c.socket == inNetSocket) {
			NSLog(@"%s(): client %p died", __func__, c);
			[clientArray removeObject: c];
			[c release];
			return;
		}
}

-(void)	netsocket: (ULINetSocket *) inNetSocket
connectionTimedOut: (NSTimeInterval) inTimeout
{
	NSLog(@"%s()", __func__);
	[self netsocketDisconnected: inNetSocket];
}

-(void)	netsocket: (ULINetSocket *) inNetSocket
connectionAccepted: (ULINetSocket *) inNewNetSocket
{
	ConnectionClient *c = [[ConnectionClient alloc] initWithSocket: inNewNetSocket
                                                         forServer: self];
	c.delegate = self;
	[clientArray addObject: c];
}

#pragma mark ### ConnectionClientDelegate ###

- (void) connectionClientDied: (ConnectionClient *) client
{
	BOOL hasAudio = ([client.audioClients count] > 0);
	[clientArray removeObject: client];
	
	if (hasAudio)
		[self connectionClientChangedAudioClients: nil];
}

- (NSArray *) allAudioClients
{
    NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
	
    for (ConnectionClient *c in clientArray)
		[array addObjectsFromArray: c.audioClients];
	
	return array;
}

- (void) connectionClientChangedAudioClients: (ConnectionClient *) client
{
	NSArray *a = [self allAudioClients];
	
    for (ConnectionClient *c in clientArray)
		if (c != client)
			[c sendAudioClientsChanged: a];	
}

- (void) connectionClient: (ConnectionClient *) client
       changedPreferences: (NSDictionary *) changed;
{
	for (id key in [changed allKeys])
		[preferences setValue: [changed objectForKey: key]
                       forKey: key];
	
	for (ConnectionClient *c in clientArray)
		if (c != client)
			[c sendPreferencesChanged: preferences.preferencesDict];
}

- (void) connectionClient: (ConnectionClient *) client
     setAudioClientConfig: (NSDictionary *) config
                  forUUID: (NSString *) uuid
{
    for (ConnectionClient *c in clientArray)
		if (c != client)
			[c setConfig: config
       forDeviceWithUUID: uuid];	
}

#pragma mark ### ServerConnectionDelegate ###

- (void) ServerConnectionEstablished: (ServerConnection *) connection
{
}

- (void) ServerConnectionFailed: (ServerConnection *) connection
{
}

- (void) ServerConnectionEnded: (ServerConnection *) connection
{
}

@end
