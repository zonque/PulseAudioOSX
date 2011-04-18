#import <Cocoa/Cocoa.h>
/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Growl/Growl.h>
#import "ServerConnection.h"

@interface Notification : NSObject <
				GrowlApplicationBridgeDelegate,
				ServerConnectionDelegate,
				NSNetServiceDelegate,
				NSNetServiceBrowserDelegate
				>
{
	NSDistributedNotificationCenter *notificationCenter;
	NSNetServiceBrowser *serverBrowser;
	NSNetServiceBrowser *sourceBrowser;
	NSNetServiceBrowser *sinkBrowser;
	NSData *logoData;
	UInt64 notificationFlags;
	BOOL growlReady;
	BOOL growlEnabled;
}

/* GrowlApplicationBridgeDelegate */
- (NSString *) applicationNameForGrowl;
- (NSDictionary *) registrationDictionaryForGrowl;
- (NSData *) applicationIconDataForGrowl;
- (void) growlIsReady;

/* ServerConnectionDelegate */
- (void) serverConnection: (id) serverConnection
       newClientAnnounced: (NSString *) name
		     icon: (NSImage *) icon;
- (void) serverConnection: (id) serverConnection
	  clientSignedOff: (NSString *) name;

/* NSNetServiceDelegate */
- (void)netServiceDidResolveAddress:(NSNetService *)sender;
- (void) netService: (NSNetService *) sender
      didNotResolve: (NSDictionary *) errorDict;
- (void)netServiceDidStop:(NSNetService *)sender;

/* NSNetServiceBrowserDelegate */
- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	    didFindService: (NSNetService *) netService
	        moreComing: (BOOL) moreServicesComing;
- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	  didRemoveService: (NSNetService *) netService
		moreComing: (BOOL) moreServicesComing;

@end
