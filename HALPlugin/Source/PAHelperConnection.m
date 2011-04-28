/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAHelperConnection.h"
#import "PAObject.h"
#import "PADeviceAudio.h"
#import "ObjectNames.h"

@implementation PAHelperConnection

@synthesize delegate;

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

	serverProxy = [NSConnection rootProxyForConnectionWithRegisteredName: @PAOSX_HelperName
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

	// make up a new name ...
	NSString *name = [NSString stringWithFormat: @"%d.%p", getpid(), self];	
	service = [NSConnection serviceConnectionWithName: name
						  rootObject: self];
	[service setDelegate: self];

	// ... tell the server about it, so it can connect back
	[serverProxy registerClientWithName: name];

	return YES;
}

- (void) deviceStarted: (PADevice *) device
{
	if (![self isConnected])
		return;

	PADeviceAudio *audio = device.deviceAudio;
	if (!audio)
		return;
	
	NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[dict setObject: [NSNumber numberWithInt: getpid()]
		 forKey: @"pid"];
	[dict setObject: [NSNumber numberWithInt: audio.ioProcBufferSize]
		 forKey: @"ioProcBufferSize"];
	[dict setObject: [NSNumber numberWithFloat: audio.sampleRate]
		 forKey: @"sampleRate"];
	[dict setObject: device.name
		 forKey: @"deviceName"];
	
	[serverProxy announceDevice: dict];
}

- (void) deviceStopped: (PADevice *) device
{
	if (![self isConnected])
		return;

	[serverProxy signOffDevice: device.name];
}

- (BOOL) isConnected
{
	return serverProxy != nil;
}

@end
