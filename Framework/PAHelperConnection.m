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

@synthesize delegate;
@synthesize serverProxy;

#pragma mark ### NotificationCenter callbacks ###

- (void) connectionDidDie: (NSNotification *) notification
{
	if (serverProxy)
		[serverProxy release];

	if (service)
		[service release];

	serverProxy = nil;
	service = nil;
	
	if (delegate)
		[delegate PAHelperConnectionDied: self];
}

#pragma mark ### vended selectors ###

- (void) setConfig: (NSDictionary *) config
 forDeviceWithName: (NSString *) name
{
	if (delegate)
		[delegate PAHelperConnection: self
				   setConfig: config
			   forDeviceWithName: name];
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
	NSString *name = [NSString stringWithFormat: @"%@.%d.%p",
					[self className], getpid(), self];	
	service = [NSConnection serviceConnectionWithName: name
						  rootObject: self];
	[service setDelegate: self];
	[service retain];

	// ... tell the server about it, so it can connect back
	[serverProxy registerClientWithName: name];

	return YES;
}

- (BOOL) isConnected
{
	return serverProxy != nil;
}

@end
