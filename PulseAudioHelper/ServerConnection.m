/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ServerConnection.h"
#import "Preferences.h"
#import "ObjectNames.h"

@implementation ServerConnection

- (void) announceDeviceSignOff: (NSRunningApplication *) app
{
	NSNumber *number = [NSNumber numberWithInteger: app.processIdentifier];
	NSDictionary *userInfo = [NSDictionary dictionaryWithObject: number
							     forKey: @"pid"];

	// send on behalf of the HAL plugin
	[[prefs notificationCenter] postNotificationName: @PAOSX_HALPluginMsgSignOffDevice
						  object: @PAOSX_HALPluginName
						userInfo: userInfo
				      deliverImmediately: YES];	
	
	[delegate serverConnection: self
		   clientSignedOff: app.localizedName
			      icon: app.icon];
	
	//[userInfo release];
}

- (void) deviceSignedOff: (NSNotification *) notification
{
	BOOL removed = NO;
	NSDictionary *userInfo = [notification userInfo];
	pid_t pid = [[userInfo objectForKey: @"pid"] intValue];
	NSMutableArray *newClientList = [NSMutableArray arrayWithArray: clientList];

	for (NSDictionary *client in clientList)
		if (pid == [[client objectForKey: @"pid"] intValue]) {
			[self announceDeviceSignOff: [client objectForKey: @"application"]];
			[newClientList removeObject: client];
			removed = YES;
		}
	
	if (removed) {
		[clientList release];
		clientList = [newClientList retain];
	}
}

- (void) scanClients: (NSTimer *) t
{
	NSMutableArray *newClientList = [NSMutableArray arrayWithArray: clientList];
	BOOL removed = NO;

	for (NSDictionary *client in clientList) {
		NSRunningApplication *app = [client objectForKey: @"application"];
		if (app.terminated) {
			[self announceDeviceSignOff: app];
			[newClientList removeObject: client];
			removed = YES;
		}
	}
	
	if (removed) {
		[clientList release];
		clientList = [newClientList retain];
	}
}

- (void) deviceAnnounced: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	pid_t pid = [[userInfo objectForKey: @"pid"] intValue];
	BOOL found = NO;
	
	NSLog(@"%s()\n", __func__);
	
	for (NSDictionary *client in clientList) {
		if (pid == [[client objectForKey: @"pid"] intValue])
			found = YES;
	}
	
	if (!found) {
		NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier: pid];
		
		if (app) {
			NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary: userInfo];
			[dict setObject: app
				 forKey: @"application"];
			[clientList addObject: dict];

			[delegate serverConnection: self
				newClientAnnounced: app.localizedName
					      icon: app.icon];
		}
	}
}

- (id) init
{
	[super init];

	clientList = [NSMutableArray arrayWithCapacity: 0];

	timer = [NSTimer timerWithTimeInterval: 1.0
					target: self
				      selector: @selector(scanClients:)
				      userInfo: nil
				       repeats: YES];
	[[NSRunLoop currentRunLoop] addTimer: timer
				     forMode: NSDefaultRunLoopMode];

	return self;
}

- (void) setDelegate: (id<ServerConnectionDelegate>) newDelegate
{
	delegate = newDelegate;
}

- (void) setPreferences: (Preferences *) newPrefs
{
	prefs = newPrefs;

	[[prefs notificationCenter] addObserver: self
				       selector: @selector(deviceAnnounced:)
					   name: @PAOSX_HALPluginMsgAnnounceDevice
					 object: @PAOSX_HALPluginName];	
	
	[[prefs notificationCenter] addObserver: self
				       selector: @selector(deviceSignedOff:)
					   name: @PAOSX_HALPluginMsgSignOffDevice
					 object: @PAOSX_HALPluginName];	
}

@end
