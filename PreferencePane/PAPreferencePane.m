/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAPreferencePane.h"

@implementation PAPreferencePane

- (void) bonjourServiceAdded: (NSNotification *) notification
{
	//NSDictionary *userInfo = [notification userInfo];
}

- (void) scanClients: (NSTimer *) t
{
	NSMutableArray *newClientList = [NSMutableArray arrayWithArray: clientList];
	NSInteger selected = [clientTableView selectedRow];
	NSDictionary *selectedClient = nil;
	
	if (selected >= 0)
		selectedClient = [clientList objectAtIndex: [clientTableView selectedRow]];
	
	BOOL removed = NO;

	for (NSDictionary *client in clientList) {
		NSRunningApplication *app = [client objectForKey: @"application"];
		if (app.terminated) {
			[newClientList removeObject: client];
			removed = YES;
		}
	}
	
	if (removed) {
		[clientList release];
		clientList = [newClientList retain];
		[clientTableView reloadData];
		
		if (selectedClient && ![clientList containsObject: selectedClient])
			[self selectClient: nil];
	}
}

- (void) clientAnnounced: (NSNotification *) notification
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

- (void) clientUnannounced: (NSNotification *) notification
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

- (void) mainViewDidLoad
{
	[clientDetailsBox selectTabViewItemAtIndex: 1];

	clientList = [[NSMutableArray arrayWithCapacity: 0] retain];
	
	timer = [NSTimer timerWithTimeInterval: 1.0
					target: self
				      selector: @selector(scanClients:)
				      userInfo: nil
				       repeats: YES];
	[[NSRunLoop currentRunLoop] addTimer: timer
				     forMode: NSDefaultRunLoopMode];
	listener = [[BonjourListener alloc] initForService: "_pulse-server._tcp"];
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(bonjourServiceAdded:)
						     name: @"serviceAdded"
						   object: listener];
	[listener start];

	notificationCenter = [NSDistributedNotificationCenter defaultCenter];
	
	[notificationCenter addObserver: self
			       selector: @selector(clientAnnounced:)
				   name: @"announceDevice"
				 object: @"PAHP_Device"];	
	
	[notificationCenter addObserver: self
			       selector: @selector(clientUnannounced:)
				   name: @"unannounceDevice"
				 object: @"PAHP_Device"];	
	
	[notificationCenter postNotificationName: @"scanDevices"
					  object: @"PAHP_Device"
					userInfo: nil
			      deliverImmediately: YES];
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

	if (!client)
		return @"";

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

- (IBAction) selectClient: (id) sender
{
	NSInteger selected = [clientTableView selectedRow];
	NSDictionary *client = nil;

	if (selected >= 0)
		client = [clientList objectAtIndex: selected];

	if (client) {
		[clientDetailsBox selectTabViewItemAtIndex: 0];
		NSRunningApplication *app = [client objectForKey: @"application"];
		[imageView setImage: app.icon];
		[audioDeviceLabel setStringValue: [client objectForKey: @"audioDevice"]];
		[PIDLabel setStringValue: [[NSNumber numberWithInt: app.processIdentifier] stringValue]];
	} else {
		[clientDetailsBox selectTabViewItemAtIndex: 1];
	}

}

@end
