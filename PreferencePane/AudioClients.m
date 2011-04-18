/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "AudioClients.h"
#import "PreferencePane.h"
#import <netinet/in.h>

@implementation AudioClients

- (void) deviceAnnounced: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	pid_t pid = [[userInfo objectForKey: @"pid"] intValue];
	BOOL found = NO;

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
			[clientTableView reloadData];
		}
	}
}

- (void) deviceSignedOff: (NSNotification *) notification
{
	BOOL removed = NO;
	NSDictionary *userInfo = [notification userInfo];
	pid_t pid = [[userInfo objectForKey: @"pid"] intValue];
	NSMutableArray *newClientList = [NSMutableArray arrayWithArray: clientList];
	NSInteger selected = [clientTableView selectedRow];
	NSDictionary *selectedClient = nil;
	
	if (selected >= 0)
		selectedClient = [clientList objectAtIndex: [clientTableView selectedRow]];

	for (NSDictionary *client in clientList)
		if (pid == [[client objectForKey: @"pid"] intValue]) {
			[newClientList removeObject: client];
			[clientTableView reloadData];
			removed = YES;
		}
	
	if (removed) {
		[clientList release];
		clientList = [newClientList retain];
		[clientTableView reloadData];

		if (selectedClient && ![clientList containsObject: selectedClient])
			[self selectClient: nil];
	}
}

- (id) init
{
	[super init];	
	return self;
}

- (void) dealloc
{
	[clientList release];
	[serviceDict release];
	[netServiceBrowser release];

	[super dealloc];
}

- (void) awakeFromNib
{
	[clientDetailsBox selectTabViewItemAtIndex: 1];

	clientList = [[NSMutableArray arrayWithCapacity: 0] retain];
	serviceDict = [[NSMutableDictionary dictionaryWithCapacity: 0] retain];

	[serverSelectButton removeAllItems];
	[serverSelectButton addItemWithTitle: @"localhost"];

	netServiceBrowser = [[[NSNetServiceBrowser alloc] init] retain];
	[netServiceBrowser setDelegate: self];
	[netServiceBrowser searchForServicesOfType: @"_pulse-server._tcp"
					  inDomain: @"local."];

	notificationCenter = [NSDistributedNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
			       selector: @selector(deviceAnnounced:)
				   name: MSG_ANNOUNCE_DEVICE
				 object: REMOTE_OBJECT_HALPLUGIN];	

	[notificationCenter addObserver: self
			       selector: @selector(deviceSignedOff:)
				   name: MSG_SIGNOFF_DEVICE
				 object: REMOTE_OBJECT_HALPLUGIN];	

	[notificationCenter postNotificationName: @"scanDevices"
					  object: REMOTE_OBJECT_HALPLUGIN
					userInfo: nil
			      deliverImmediately: YES];
	
	NSLog(@"%s()\n", __func__);
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView:(NSTableView *)aTableView
   setObjectValue:obj
   forTableColumn:(NSTableColumn *)col
	      row:(int)rowIndex
{
}

- (id)tableView:(NSTableView *)tableView
	objectValueForTableColumn:(NSTableColumn *)col
	row:(int)rowIndex
{
	NSDictionary *client = [clientList objectAtIndex: rowIndex];
	NSRunningApplication *app = [client objectForKey: @"application"];

	if ([[col identifier] isEqualToString: @"icon"])
		return app.icon;

	if ([[col identifier] isEqualToString: @"name"])
		return app.localizedName;
	
	return @"";
}

- (int) numberOfRowsInTableView:(NSTableView *)tableView
{
	return clientList ? [clientList count] : 0;
}

#pragma mark ### IBActions ###

enum {
	kClientDetailBoxEnabledTab = 0,
	kClientDetailBoxDisabledTab = 1,
};

- (IBAction) selectClient: (id) sender
{
	NSInteger selected = [clientTableView selectedRow];
	NSDictionary *client = nil;
	NSArray *connectionStatusStrings = [NSArray arrayWithObjects:
						    @"Unconnected",
						    @"Connecting",
						    @"Authorizing",
						    @"Setting name",
						    @"Connected",
						    @"Failed",
						    @"Terminated",
						    nil];

	if (selected >= 0)
		client = [clientList objectAtIndex: selected];

	if (!client) {
		[clientDetailsBox selectTabViewItemAtIndex: kClientDetailBoxDisabledTab];
		return;
	}

	NSRunningApplication *app = [client objectForKey: @"application"];

	NSString *arch = @"unknown";
	
	switch (app.executableArchitecture) {
		case NSBundleExecutableArchitectureI386:
			arch = @"i386 (32bit)";
			break;
		case NSBundleExecutableArchitecturePPC:
			arch = @"PPC (32bit)";
			break;
		case NSBundleExecutableArchitectureX86_64:
			arch = @"x86_64";
			break;
		case NSBundleExecutableArchitecturePPC64:
			arch = @"PPC64";
			break;
	}
	
	[clientDetailsBox selectTabViewItemAtIndex: kClientDetailBoxEnabledTab];
	[imageView setImage: app.icon];
	[clientNameLabel setStringValue: app.localizedName];
	[audioDeviceLabel setStringValue: [client objectForKey: @"audioDevice"]];
	[PIDLabel setStringValue: [NSString stringWithFormat: @"%d, %@", app.processIdentifier, arch]];
	[IOBufferSizeLabel setStringValue: [[client objectForKey: @"IOBufferFrameSize"] stringValue]];
	
	NSString *serverIP = [client objectForKey: @"serverName"];
	
	id key;
	BOOL found = NO;
	NSEnumerator *enumerator = [serviceDict keyEnumerator];
	while (key = [enumerator nextObject]) {
		NSNetService *service = [serviceDict objectForKey: key];
		NSString *ip = [self ipOfService: service];

		if ([ip isEqualToString: serverIP]) {
			[serverSelectButton selectItemWithTitle: [service name]];
			found = YES;
		}
	}
	
	if (!found)
		[serverSelectButton selectItemWithTitle: serverIP];

	NSInteger connectionStatus = [[client objectForKey: @"connectionStatus"] intValue];
	[connectionStatusTextField setStringValue: [connectionStatusStrings objectAtIndex: connectionStatus]];
	
	[persistenCheckButton setTitle: [NSString stringWithFormat:
			@"Always use these parameters for %@", app.localizedName]];		
}

- (NSString *) ipOfService: (NSNetService *) service
{
	NSData *addr = [[service addresses] objectAtIndex: 0];
	struct sockaddr_in *address_sin = (struct sockaddr_in *)[addr bytes];
	return [NSString stringWithFormat: @"%d.%d.%d.%d",
		    (address_sin->sin_addr.s_addr >> 0) & 0xff,
		    (address_sin->sin_addr.s_addr >> 8) & 0xff,
		    (address_sin->sin_addr.s_addr >> 16) & 0xff,
		    (address_sin->sin_addr.s_addr >> 24) & 0xff];
}

- (IBAction) connectClient: (id) sender
{
	NSInteger selected = [clientTableView selectedRow];
	NSMutableDictionary *client = [clientList objectAtIndex: selected];
	NSNumber *pid = [client objectForKey: @"pid"];
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	NSString *serverName = [serverSelectButton titleOfSelectedItem];
	NSNetService *service = [serviceDict objectForKey: serverName];
	NSNumber *serverPort = NULL;
	NSString *serverIP;
	
	if (service) {
		serverIP = [self ipOfService: service];
		serverPort = [NSNumber numberWithInt: [service port]];
	} else {
		serverIP = @"localhost";
	}

	[userInfo setObject: serverIP
		     forKey: @"serverName"];
	[userInfo setObject: pid
		     forKey: @"pid"];
	if (serverPort)
		[userInfo setObject: serverPort
			     forKey: @"serverPort"];
	
	[notificationCenter postNotificationName: MSG_SETCONFIGURATION
					  object: REMOTE_OBJECT_HELPER
					userInfo: userInfo
			      deliverImmediately: YES];
	
	[client setObject: serverName
		   forKey: @"serverName"];
}

#pragma mark ### NSNetServiceDelegate ###

- (void)netServiceDidResolveAddress:(NSNetService *)sender
{
	NSString *name = [sender name];
	NSArray *addresses = [sender addresses];
	
	if ([addresses count] == 0)
		return;
	
	[serverSelectButton addItemWithTitle: name];		
}

- (void) netService: (NSNetService *) sender
      didNotResolve: (NSDictionary *)errorDict
{
	[serviceDict removeObjectForKey: [sender name]];
	[serverSelectButton removeItemWithTitle: [sender name]];		
}

- (void)netServiceDidStop:(NSNetService *)sender
{
	[serviceDict removeObjectForKey: [sender name]];
	[serverSelectButton removeItemWithTitle: [sender name]];		
}

#pragma mark ### NSNetServiceBrowserDelegate ###

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	    didFindService: (NSNetService *) netService
	        moreComing: (BOOL) moreServicesComing
{
	[serviceDict setObject: netService
			forKey: [netService name]];
	
	[netService setDelegate: self];
	[netService resolveWithTimeout: 10.0];
}

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	  didRemoveService: (NSNetService *) netService
		moreComing: (BOOL) moreServicesComing
{
	NSString *name = [netService name];
	[serviceDict removeObjectForKey: name];
	[serverSelectButton removeItemWithTitle: name];	
}

@end