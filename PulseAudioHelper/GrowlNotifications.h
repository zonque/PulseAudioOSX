/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>
#import <Growl/Growl.h>
#import "Preferences.h"

@interface GrowlNotifications : NSObject <
				GrowlApplicationBridgeDelegate,
				NSNetServiceDelegate,
				NSNetServiceBrowserDelegate
				>
{
	NSNetServiceBrowser *serverBrowser;
	NSNetServiceBrowser *sourceBrowser;
	NSNetServiceBrowser *sinkBrowser;
	NSData *logoData;
	BOOL growlReady;
	//ServerConnection *serverConnection;
	Preferences *prefs;
}

- (void) setPreferences: (Preferences *) newPrefs;

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
	  clientSignedOff: (NSString *) name
		     icon: (NSImage *) icon;

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
