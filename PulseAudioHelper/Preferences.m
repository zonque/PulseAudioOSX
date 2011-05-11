/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Preferences.h"

static NSString *kPAPreferencesGrowlFlagsKey                = @"growlNotificationFlags";
static NSString *kPAPreferencesGrowlEnabledKey                = @"growlNotificationsEnabled";
static NSString *kPAPreferencesLocalServerEnabledKey        = @"localServerEnabled";
static NSString *kPAPreferencesStatusBarEnabledKey        = @"statusBarEnabled";

@implementation Preferences

@synthesize preferencesDict;

- (NSString *) preferencesFilePath
{
        return [NSString stringWithFormat: @"%@/Library/Preferences/org.pulseaudio.plist",
                                                NSHomeDirectory()];
}

#pragma mark ### NSDistributedNotificationCenter ###

#pragma mark ## Growl ##

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
        [preferencesDict setObject: [NSNumber numberWithBool: YES]
                            forKey: @"localServerNetworkEnabled"];
        [preferencesDict setObject: @"localhost"
                            forKey: @"defaultServer"];
	
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

        return self;
}

- (id) valueForKey: (NSString *) key
{
        return [preferencesDict valueForKey: key];
}

- (void) setValue: (id) value
           forKey: (NSString *) key
{
        if (![preferencesDict objectForKey: key]) {
                NSLog(@"key %@ unknown, refusing to set", key);
                return;
        }

        NSLog(@"Setting prefs key: %@ -> %@", key, value);

        [preferencesDict setValue: value
                           forKey: key];

        [[NSNotificationCenter defaultCenter] postNotificationName: @"preferencesChanged"
                                                            object: self];
        [preferencesDict writeToFile: [self preferencesFilePath]
                          atomically: YES];
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
