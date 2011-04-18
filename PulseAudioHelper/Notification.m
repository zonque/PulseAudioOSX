/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Notification.h"

#define REMOTE_OBJECT @"PulseAudioPreferencePane"
#define LOCAL_OBJECT @"PulseAudioHelper"

#define PATHHACK "/Users/daniel/src/pa/PulseAudioOSX/PulseAudioHelper/"

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

static NSString *kMDNSPulseServer = @"_pulse-server._tcp";
static NSString *kMDNSPulseSink   = @"_pulse-sink._tcp";
static NSString *kMDNSPulseSource = @"_pulse-source._tcp";
static NSString *kMDNSLocalDomain = @"local.";

@implementation Notification

- (void) updateGrowlFlags: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSNumber *number = [userInfo objectForKey: @"notificationFlags"];
	notificationFlags = [number unsignedLongLongValue];
}

- (void) queryGrowlFlags: (NSNotification *) notification
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	UInt64 flags = notificationFlags;
	
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

- (id) init
{
	[super init];

	NSImage *img = [[NSImage alloc] initWithContentsOfFile: @PATHHACK"PulseAudio.png"];
	logoData = [[img TIFFRepresentation] retain];
	
	/* fixme */
	notificationFlags = 0xffffffffffffffff;
	growlEnabled = YES;
	
	[GrowlApplicationBridge setGrowlDelegate: self];
	
	serverBrowser	= [[[NSNetServiceBrowser alloc] init] retain];
	sinkBrowser	= [[[NSNetServiceBrowser alloc] init] retain];
	sourceBrowser	= [[[NSNetServiceBrowser alloc] init] retain];

	[serverBrowser	setDelegate: self];
	[sinkBrowser	setDelegate: self];
	[sourceBrowser	setDelegate: self];
	
	[serverBrowser searchForServicesOfType: kMDNSPulseServer
				      inDomain: kMDNSLocalDomain];
	[sinkBrowser searchForServicesOfType: kMDNSPulseSink
				    inDomain: kMDNSLocalDomain];
	[sourceBrowser searchForServicesOfType: kMDNSPulseSource
				      inDomain: kMDNSLocalDomain];
	
	notificationCenter = [[NSDistributedNotificationCenter defaultCenter] retain];
	
	[notificationCenter addObserver: self
			       selector: @selector(updateGrowlFlags:)
				   name: @"updateGrowlFlags"
				 object: REMOTE_OBJECT];	

	[notificationCenter addObserver: self
			       selector: @selector(queryGrowlFlags:)
				   name: @"queryGrowlFlags"
				 object: REMOTE_OBJECT];
	
	
	ServerConnection *serverConnection = [[ServerConnection alloc] init];
	[serverConnection setDelegate: self];
	
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
	growlReady = YES;
}

#pragma mark ### ServerConnectionDelegate ###

- (void) serverConnection: (id) serverConnection
       newClientAnnounced: (NSString *) name
{
	if (!growlEnabled)
		return;
	
	if (!(notificationFlags & kPulseAudioClientConnectedFlag))
		return;

	NSString *description = [NSString stringWithFormat: @"New client attached: %@", name];
	
	[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
				    description: description
			       notificationName: kPulseAudioClientConnected
				       iconData: logoData
				       priority: 0
				       isSticky: NO
				   clickContext: nil];
	
}

- (void) serverConnection: (id) serverConnection
	  clientSignedOff: (NSString *) name
{
	
}

#pragma mark ### NSNetServiceDelegate ###

- (void)netServiceDidResolveAddress:(NSNetService *)sender
{
	NSArray *addresses = [sender addresses];
	
	if ([addresses count] == 0)
		return;

	NSString *name = [sender name];
	NSString *type = [sender type];
	NSString *notification = nil;
	NSString *description = nil;
	
	if ([type hasPrefix: kMDNSPulseServer] &&
	    (notificationFlags & kPulseAudioServerAppearedFlag)) {
		notification = kPulseAudioServerAppeared;
		description = [NSString stringWithFormat: @"Server appeared: %@", name];
	}
	
	if ([type hasPrefix: kMDNSPulseSink] &&
	    (notificationFlags & kPulseAudioSinkAppearedFlag)) {
		notification = kPulseAudioSinkAppeared;
		description = [NSString stringWithFormat: @"Sink appeared: %@", name];
	}
	
	if ([type hasPrefix: kMDNSPulseSource] &&
	    (notificationFlags & kPulseAudioSourceAppearedFlag)) {
		notification = kPulseAudioSourceAppeared;
		description = [NSString stringWithFormat: @"Source appeared: %@", name];
	}
	
	if (notification && growlEnabled)
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: notification
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];	
}

- (void) netService: (NSNetService *) sender
      didNotResolve: (NSDictionary *)errorDict
{
}

- (void)netServiceDidStop:(NSNetService *)sender
{
}

#pragma mark ### NSNetServiceBrowserDelegate ###

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	    didFindService: (NSNetService *) netService
	        moreComing: (BOOL) moreServicesComing
{
	[netService retain];
	[netService setDelegate: self];
	[netService resolveWithTimeout: 10.0];
}

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	  didRemoveService: (NSNetService *) netService
		moreComing: (BOOL) moreServicesComing
{
	NSString *name = [netService name];
	NSString *type = [netService type];
	NSString *notification = nil;
	NSString *description = nil;
	
	if ([type hasPrefix: kMDNSPulseServer] &&
	    (notificationFlags & kPulseAudioServerDisappearedFlag)) {
		notification = kPulseAudioServerDisappeared;
		description = [NSString stringWithFormat: @"Server disappeared: %@", name];
	}
	
	if ([type hasPrefix: kMDNSPulseSink] &&
	    (notificationFlags & kPulseAudioSinkDisappearedFlag)) {
		notification = kPulseAudioSinkDisappeared;
		description = [NSString stringWithFormat: @"Sink disappeared: %@", name];
	}
	
	if ([type hasPrefix: kMDNSPulseSource] &&
	    (notificationFlags & kPulseAudioSourceDisappearedFlag)) {
		notification = kPulseAudioSourceDisappeared;
		description = [NSString stringWithFormat: @"Source disappeared: %@", name];
	}
	
	if (notification && growlEnabled)
		[GrowlApplicationBridge notifyWithTitle: @"PulseAudio"
					    description: description
				       notificationName: notification
					       iconData: logoData
					       priority: 0
					       isSticky: NO
					   clickContext: nil];
}

@end
