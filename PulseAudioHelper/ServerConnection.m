/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ServerConnection.h"

#define REMOTE_OBJECT @"PAHP_Device"

@implementation ServerConnection

- (void) deviceAnnounced: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	pid_t pid = [[userInfo objectForKey: @"pid"] intValue];
	
	NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier: pid];
	
	if (app) {
		[delegate serverConnection: self
			newClientAnnounced: app.localizedName
				      icon: app.icon];
	}
}

- (void) deviceSignedOff: (NSNotification *) notification
{
}

- (id) init
{
	[super init];

	notificationCenter = [NSDistributedNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
			       selector: @selector(deviceAnnounced:)
				   name: @"announceDevice"
				 object: REMOTE_OBJECT];	
	
	[notificationCenter addObserver: self
			       selector: @selector(deviceSignedOff:)
				   name: @"signOffDevice"
				 object: REMOTE_OBJECT];	

	return self;
}

- (void) setDelegate: (id<ServerConnectionDelegate>) newDelegate
{
	delegate = newDelegate;
}

@end
