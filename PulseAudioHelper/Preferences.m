/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Preferences.h"

@implementation Preferences

#pragma mark ### NSDistributedNotificationCenter ###

#pragma mark ## Growl ##

- (void) updateGrowlFlags: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSNumber *number = [userInfo objectForKey: @"notificationFlags"];
	growlNotificationFlags = [number unsignedLongLongValue];
}

- (void) queryGrowlFlags: (NSNotification *) notification
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	UInt64 flags = growlNotificationFlags;
	
	if (growlReady)
		flags |= 1ULL << 63;
	
	if (growlEnabled)
		flags |= 1ULL << 62;
	
	[userInfo setObject: [NSNumber numberWithUnsignedLongLong: flags]
		     forKey: @"notificationFlags"];
	
	[notificationCenter postNotificationName: @"updateGrowlFlags"
					  object: LOCAL_OBJECT
					userInfo: userInfo
			      deliverImmediately: YES];
}

#pragma mark ## Local Server ##

- (void) setLocalServerEnabled: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	localServerEnabled = [[userInfo objectForKey: @"enabled"] boolValue];
}

- (id) init
{
	[super init];

	growlNotificationFlags = 0xffffffffffffffff;
	localServerEnabled = YES;

	notificationCenter = [[NSDistributedNotificationCenter defaultCenter] retain];
	
	[notificationCenter addObserver: self
			       selector: @selector(updateGrowlFlags:)
				   name: @"updateGrowlFlags"
				 object: REMOTE_OBJECT_PREFPANE];	
	
	[notificationCenter addObserver: self
			       selector: @selector(queryGrowlFlags:)
				   name: @"queryGrowlFlags"
				 object: REMOTE_OBJECT_PREFPANE];

	[notificationCenter addObserver: self
			       selector: @selector(setLocalServerEnabled:)
				   name: @"setLocalServerEnabled"
				 object: REMOTE_OBJECT_PREFPANE];	
	
	return self;
}

- (NSDistributedNotificationCenter *) getCenter
{
	return notificationCenter;
}

#pragma mark ### Growl Notifications ###

- (BOOL) isGrowlEnabled
{
	return YES;
}

- (UInt64) growlNotificationFlags
{
	return growlNotificationFlags;
}

- (void) setGrowlReady: (BOOL) ready
{
	growlReady = ready;
}

#pragma mark ### Server Task ###

- (BOOL) isLocalServerEnabled
{
	return localServerEnabled;
}

@end
