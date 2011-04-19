/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Preferences.h"
#import "ObjectNames.h"

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
	
	[notificationCenter postNotificationName: @PAOSX_HelperMsgSetGrowlFlags
					  object: @PAOSX_HelperName
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
	
	[notificationCenter postNotificationName: @PAOSX_HelperMsgSetStatusBarEnabled
					  object: @PAOSX_HelperName
					userInfo: userInfo
			      deliverImmediately: YES];	
}

- (void) makeDefaults
{
	preferencesDict = [NSMutableDictionary dictionaryWithCapacity: 0];

	[preferencesDict setObject: [NSNumber numberWithBool: YES]
			    forKey: kPAPreferencesGrowlEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithBool: YES]
			    forKey: kPAPreferencesLocalServerEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithBool: NO]
			    forKey: kPAPreferencesStatusBarEnabledKey];
	[preferencesDict setObject: [NSNumber numberWithUnsignedLongLong: 0xffffffffffffffff]
			    forKey: kPAPreferencesGrowlFlagsKey];
	[preferencesDict writeToFile: [self preferencesFilePath]
			  atomically: YES];
}

- (id) init
{
	[super init];

	preferencesDict = [NSMutableDictionary dictionaryWithContentsOfFile: [self preferencesFilePath]];

	if (!preferencesDict) {
		NSLog(@"Unable to load config file, restoring defaults\n");
		[self makeDefaults];
	}
	
	notificationCenter = [[NSDistributedNotificationCenter defaultCenter] retain];
	
	[notificationCenter addObserver: self
			       selector: @selector(updateGrowlFlags:)
				   name: @PAOSX_HelperMsgSetGrowlFlags
				 object: nil];	
	
	[notificationCenter addObserver: self
			       selector: @selector(queryGrowlFlags:)
				   name: @PAOSX_HelperMsgQueryGrowlFlags
				 object: nil];

	[notificationCenter addObserver: self
			       selector: @selector(setLocalServerEnabled:)
				   name: @PAOSX_HelperMsgSetLocalServerEnabled
				 object: nil];	

	[notificationCenter addObserver: self
			       selector: @selector(setStatusBarEnabled:)
				   name: @PAOSX_HelperMsgSetStatusBarEnabled
				 object: nil];	
	
	[notificationCenter addObserver: self
			       selector: @selector(queryStatusBarEnabled:)
				   name: @PAOSX_HelperMsgQueryStatusBarEnabled
				 object: nil];	
	
	return self;
}

- (NSDistributedNotificationCenter *) notificationCenter
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
