/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Preferences.h"

static NSString *kPAPreferencesGrowlFlagsKey		= @"growlNotificationFlags";
static NSString *kPAPreferencesGrowlEnabledKey		= @"growlNotificationsEnabled";
static NSString *kPAPreferencesLocalServerEnabledKey	= @"localServerEnabled";
static NSString *kPAPreferencesStatusBarEnabledKey	= @"statusBarEnabled";

@implementation Preferences

- (NSString *) preferencesFilePath
{
	return [NSString stringWithFormat: @"%@/Library/Preferences/org.pulseaudio.plist",
						NSHomeDirectory()];
}

#pragma mark ### NSDistributedNotificationCenter ###

#pragma mark ## Growl ##

- (void) updateGrowlFlags: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	[preferencesDict setObject: [userInfo objectForKey: @"notificationFlags"]
			    forKey: kPAPreferencesGrowlEnabledKey];
	[preferencesDict writeToFile: [self preferencesFilePath]
			  atomically: YES];
}

- (void) queryGrowlFlags: (NSNotification *) notification
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	UInt64 flags = [[preferencesDict objectForKey: kPAPreferencesGrowlFlagsKey] unsignedLongLongValue];
	BOOL growlEnabled = [[preferencesDict objectForKey: kPAPreferencesGrowlEnabledKey] boolValue];

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
	[preferencesDict setObject: [userInfo objectForKey: @"enabled"]
			    forKey: kPAPreferencesLocalServerEnabledKey];
	[preferencesDict writeToFile: [self preferencesFilePath]
			  atomically: YES];
}

#pragma mark ## Status Bar ##

- (void) setStatusBarEnabled: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	[preferencesDict setObject: [userInfo objectForKey: @"enabled"]
			    forKey: kPAPreferencesStatusBarEnabledKey];
	[preferencesDict writeToFile: [self preferencesFilePath]
			  atomically: YES];
}

- (void) queryStatusBarEnabled: (NSNotification *) notification
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[userInfo setObject: [preferencesDict objectForKey: kPAPreferencesStatusBarEnabledKey]
		     forKey: @"enabled"];
	
	[notificationCenter postNotificationName: @"setStatusBarEnabled"
					  object: LOCAL_OBJECT
					userInfo: userInfo
			      deliverImmediately: YES];	
}

- (void) makeDefaults
{
	[preferencesDict setObject: [NSNumber numberWithBool: YES]
			    forKey: kPAPreferencesGrowlEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithBool: YES]
			    forKey: kPAPreferencesLocalServerEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithBool: NO]
			    forKey: kPAPreferencesStatusBarEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithUnsignedLongLong: 0xffffffffffffffff]
			    forKey: kPAPreferencesGrowlFlagsKey];
}

- (id) init
{
	[super init];

	preferencesDict = [NSMutableDictionary dictionaryWithContentsOfFile: [self preferencesFilePath]];

	if (!preferencesDict) {
		NSLog(@"Unable to load config file, restoring defaults\n");
		preferencesDict = [NSMutableDictionary dictionaryWithCapacity: 0];
		[self makeDefaults];
		[preferencesDict writeToFile: [self preferencesFilePath]
				  atomically: YES];
	}
	
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

	[notificationCenter addObserver: self
			       selector: @selector(setStatusBarEnabled:)
				   name: @"setStatusBarEnabled"
				 object: REMOTE_OBJECT_PREFPANE];	
	
	[notificationCenter addObserver: self
			       selector: @selector(queryStatusBarEnabled:)
				   name: @"queryStatusBarEnabled"
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
	return [[preferencesDict objectForKey: kPAPreferencesGrowlEnabledKey] boolValue];
}

- (UInt64) growlNotificationFlags
{
	return [[preferencesDict objectForKey: kPAPreferencesGrowlFlagsKey] unsignedLongLongValue];
}

- (void) setGrowlReady: (BOOL) ready
{
	growlReady = ready;
}

#pragma mark ### Server Task ###

- (BOOL) isLocalServerEnabled
{
	return [[preferencesDict objectForKey: kPAPreferencesLocalServerEnabledKey] boolValue];
}

#pragma mark ### Status Bar ###

- (BOOL) isStatusBarEnabled
{
	return [[preferencesDict objectForKey: kPAPreferencesStatusBarEnabledKey] boolValue];
}

@end
