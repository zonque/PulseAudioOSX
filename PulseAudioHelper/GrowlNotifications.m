/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "GrowlNotifications.h"
#import "Pathes.h"

#define defineNotification(x, n) \
	static NSString *x = @ #x; \
	static NSInteger x ##Flag = (1ULL << n);

defineNotification(kPulseAudioServerAppeared,		0);
defineNotification(kPulseAudioServerDisappeared,	1);
defineNotification(kPulseAudioSourceAppeared,		2);
defineNotification(kPulseAudioSourceDisappeared,	3);
defineNotification(kPulseAudioSinkAppeared,		4);
defineNotification(kPulseAudioSinkDisappeared,		5);
defineNotification(kPulseAudioClientConnected,		6);
defineNotification(kPulseAudioClientDisconnected,	7);

@implementation GrowlNotifications

- (void) preferencesChanged: (NSNotification *) notification
{
}

- (id) initWithPreferences: (Preferences *) p
{
	[super init];

	NSImage *img = [NSImage imageNamed: @"PulseAudio.png"];
	logoData = [[img TIFFRepresentation] retain];
	
	[GrowlApplicationBridge setGrowlDelegate: self];
	
	discovery = [[PAServiceDiscovery alloc] init];
	discovery.delegate = self;
	[discovery start];

	preferences = p;
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(preferencesChanged:)
						     name: @"preferencesChanged"
						   object: preferences];
	
	return self;
}

#pragma mark ### GrowlApplicationBridgeDelegate ###

- (NSString *) applicationNameForGrowl
{
	return @"PulseAudio";
}

- (NSDictionary *) registrationDictionaryForGrowl
{
	NSArray *notifications = [NSArray arrayWithObjects:
					kPulseAudioServerAppeared,
					kPulseAudioServerDisappeared,
					kPulseAudioSourceAppeared,
					kPulseAudioSourceDisappeared,
					kPulseAudioSinkAppeared,
					kPulseAudioSinkDisappeared,
					kPulseAudioClientConnected,
					kPulseAudioClientDisconnected,
					nil];
	
	NSDictionary *regDict = [NSDictionary dictionaryWithObjectsAndKeys:
				 @"PulseAudio", GROWL_APP_ID,
				 @"PulseAudio", GROWL_APP_NAME,
				 notifications, GROWL_NOTIFICATIONS_ALL,
				 notifications, GROWL_NOTIFICATIONS_DEFAULT,
				 nil];
	
	CFShow(regDict);
	
        return regDict;
}

- (NSData *) applicationIconDataForGrowl
{
	return logoData;
}

- (void) growlIsReady
{
}

#pragma mark ### ServerConnectionDelegate ###

- (void) serverConnection: (id) serverConnection
       newClientAnnounced: (NSString *) name
		     icon: (NSImage *) icon
{
	if (![preferences isGrowlEnabled])
		return;
	
	if (!([preferences growlNotificationFlags] & kPulseAudioClientConnectedFlag))
		return;

	NSString *description = [NSString stringWithFormat: @"New client attached: %@", name];

	[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
				    description: description
			       notificationName: kPulseAudioClientConnected
				       iconData: [icon TIFFRepresentation]
				       priority: 0
				       isSticky: NO
				   clickContext: nil];
}

- (void) serverConnection: (id) serverConnection
	  clientSignedOff: (NSString *) name
		     icon: (NSImage *) icon
{
	if (![preferences isGrowlEnabled])
		return;
	
	if (!([preferences growlNotificationFlags] & kPulseAudioClientDisconnectedFlag))
		return;

	NSString *description = [NSString stringWithFormat: @"Client disconnected: %@", name];
	
	[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
				    description: description
			       notificationName: kPulseAudioClientDisconnected
				       iconData: [icon TIFFRepresentation]
				       priority: 0
				       isSticky: NO
				   clickContext: nil];
	
}

#pragma mark ### PAServiceDiscoveryDelegate ###

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	     serverAppeared: (NSNetService *) service
{
	NSString *description = [NSString stringWithFormat: @"Server appeared: %@", [service name]];

	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioServerAppearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioServerAppeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	  serverDisappeared: (NSNetService *) service
{
	NSString *description = [NSString stringWithFormat: @"Server disappeared: %@", [service name]];
	
	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioServerDisappearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioServerDisappeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	       sinkAppeared: (NSNetService *) service
{
	NSString *description = [NSString stringWithFormat: @"Sink appeared: %@", [service name]];
	
	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioSinkAppearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioSinkAppeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	    sinkDisappeared: (NSNetService *) service
{
	NSString *description = [NSString stringWithFormat: @"Sink disappeared: %@", [service name]];
	
	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioSinkDisappearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioSinkDisappeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	     sourceAppeared: (NSNetService *) service
{
	NSString *description = [NSString stringWithFormat: @"Source appeared: %@", [service name]];
	
	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioSourceAppearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioSourceAppeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	  sourceDisappeared: (NSNetService *) service
{	
	NSString *description = [NSString stringWithFormat: @"Source disappeared: %@", [service name]];
	
	if ([preferences isGrowlEnabled] &&
	    ([preferences growlNotificationFlags] & kPulseAudioSourceDisappearedFlag))
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: kPulseAudioSourceDisappeared
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

@end
