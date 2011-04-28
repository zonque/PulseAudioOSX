/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ConnectionServer.h"
#import "ObjectNames.h"

@implementation ConnectionServer

@synthesize currentConnection;

#pragma mark ### PAHelperConnection protocol ###

- (void) audioClientsChanged
{
	for (id<PAHelperConnection> o in clientObjects)
		[o audioClientsChanged: audioClients];
}

- (void) registerClientWithName: (NSString *) name
{
	id proxy = [NSConnection rootProxyForConnectionWithRegisteredName: name
								     host: nil];

	NSLog(@"%s() %@", __func__, name);

	if (proxy)
		[clientObjects addObject: proxy];
}

- (void) announceDevice: (NSDictionary *) device
{
	NSMutableDictionary *entry = [NSMutableDictionary dictionaryWithDictionary: device];

	[entry setObject: currentConnection
		  forKey: @"connection"];
	
	[audioClients addObject: entry];
	
//	NSLog(@" %s(): %@", __func__, device);
}

- (void) signOffDevice: (NSString *) signedOffName
{	
	NSLog(@" %s(): %@", __func__, signedOffName);

	for (NSDictionary *entry in audioClients) {
		NSString *name = [entry objectForKey: @"deviceName"];
		NSConnection *connection = [entry objectForKey: @"connection"];

		if ([name isEqualToString: signedOffName] &&
		    connection == currentConnection) {
			[audioClients removeObject: entry];
			NSLog(@"removing ...");
			return;
		}
	}
}

#pragma mark ### NSConnection callbacks ###

- (void) connectionDied: (NSNotification *) notification
{
	NSConnection *connection = [notification object];
	NSLog(@"%s() %@", __func__, [connection statistics]);
	
	for (NSDistantObject *o in clientObjects)
		if ([o connectionForProxy] == connection) {
			[clientObjects removeObject: o];
			return;
		}
	
	for (NSDictionary *entry in audioClients) {
		NSConnection *c = [entry objectForKey: @"connection"];
		
		if (c == connection) {
			[audioClients removeObject: entry];
			return;
		}
	}
}

- (void) connectionInitialized: (NSNotification *) notification
{
	NSLog(@"%s()", __func__);
	//NSConnection *connection = [notification object];
	//[connection crea
}

- (id) init
{
	[super init];

	clientObjects = [[NSMutableArray arrayWithCapacity: 0] retain];
	audioClients = [[NSMutableArray arrayWithCapacity: 0] retain];
	
	return self;
}

- (void) start
{
	NSConnection *connection = [NSConnection serviceConnectionWithName: @PAOSX_HelperName
								rootObject: self];	
	[connection setDelegate: self];	
	[connection addRunLoop: [NSRunLoop currentRunLoop]];
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
	
	NSLog(@"%s() conn %p", __func__, conn);
	return NO;
}

@end
