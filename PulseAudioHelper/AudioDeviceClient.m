/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "AudioDeviceClient.h"

@implementation AudioDeviceClient

- (void) connectionDied: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSValue *cookie = [userInfo objectForKey: kSocketServerCookieKey];
	NSMutableArray *newClients = [NSMutableArray arrayWithArray: audioClients];
	BOOL modified = NO;
	
	NSLog(@"%s()\n", __func__);
	
	for (NSDictionary *d in audioClients) {
		NSValue *v = [userInfo objectForKey: kSocketServerCookieKey];
		
		if ([v pointerValue] == [cookie pointerValue]) {
			[newClients removeObject: d];
			modified = YES;
		}
	}
		
	if (modified) {
		[audioClients release];
		audioClients = [newClients retain];
		
		CFShow(audioClients);
		// ...
	}	
}

- (void) messageReceived: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSString *messageName = [userInfo objectForKey: @PAOSX_MessageNameKey];
	NSValue *cookie = [userInfo objectForKey: kSocketServerCookieKey];
	NSMutableArray *newClients = [NSMutableArray arrayWithArray: audioClients];
	BOOL modified = NO;
	
	if ([messageName isEqualToString: @PAOSX_SocketConnectionAnnounceMessage])
		[audioClients addObject: userInfo];

	if ([messageName isEqualToString: @PAOSX_SocketConnectionSignOffMessage]) {
		for (NSDictionary *d in audioClients) {
			NSValue *v = [userInfo objectForKey: kSocketServerCookieKey];
			
			if ([v pointerValue] == [cookie pointerValue]) {
				[newClients removeObject: d];
				modified = YES;
			}
		}
	}

	
	
	if (modified) {
		[audioClients release];
		audioClients = [newClients retain];
		
		CFShow(audioClients);
		// ...
	}
	
	CFShow(userInfo);
	CFShow(messageName);
}

- (void) setSocketServer: (SocketServer *) newServer
{
	server = [newServer retain];

	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(messageReceived:) 
						     name: kSocketServerNotificationMessageReceived
						   object: server];	

	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionDied:) 
						     name: kSocketServerNotificationConnectionDied
						   object: server];
}

- (id) init
{
	[super init];
	
	audioClients = [NSMutableArray arrayWithCapacity: 0];

	return self;
}


@end
