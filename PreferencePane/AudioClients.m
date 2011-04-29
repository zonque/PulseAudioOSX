/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "AudioClients.h"

@implementation AudioClients

- (void) dealloc
{
	[clientList release];
	[discovery release];

	[super dealloc];
}

- (void) awakeFromNib
{
	[clientDetailsBox selectTabViewItemAtIndex: 1];

	clientList = [[NSMutableArray arrayWithCapacity: 0] retain];
	serviceDict = [[NSMutableDictionary dictionaryWithCapacity: 0] retain];

	[serverSelectButton removeAllItems];
	[serverSelectButton addItemWithTitle: @"localhost"];

	discovery = [[PAServiceDiscovery alloc] init];
	discovery.delegate = self;
	[discovery start];
	
	NSLog(@"%s()\n", __func__);
}

- (void) audioClientsChanged: (NSArray *) clients
{
	[clientList removeAllObjects];
	
	NSLog(@"%@", clients);
	
	for (NSDictionary *c in clients) {
		pid_t pid = [[c objectForKey: @"pid"] intValue];
		NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier: pid];
		
		if (app) {
			NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary: c];
			[dict setObject: app
				 forKey: @"application"];
			[clientList addObject: dict];
		}
	}
	
	[clientTableView reloadData];
	[self selectClient: nil];
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView: (NSTableView *) aTableView
   setObjectValue: obj
   forTableColumn: (NSTableColumn *) col
	      row: (int) rowIndex
{
}

- (id)tableView: (NSTableView *) tableView
	objectValueForTableColumn: (NSTableColumn *) col
	row: (int) rowIndex
{
	NSDictionary *client = [clientList objectAtIndex: rowIndex];
	NSRunningApplication *app = [client objectForKey: @"application"];

	if ([[col identifier] isEqualToString: @"icon"])
		return app.icon;

	if ([[col identifier] isEqualToString: @"name"])
		return app.localizedName;
	
	return @"";
}

- (int) numberOfRowsInTableView: (NSTableView *) tableView
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
	[audioDeviceLabel setStringValue: [client objectForKey: @"deviceName"]];
	[PIDLabel setStringValue: [NSString stringWithFormat: @"%d, %@", app.processIdentifier, arch]];
	[IOBufferSizeLabel setStringValue: [[client objectForKey: @"ioProcBufferSize"] stringValue]];
		
	id key;
	BOOL found = NO;
	NSEnumerator *enumerator = [serviceDict keyEnumerator];
	while (key = [enumerator nextObject]) {
		NSNetService *service = [serviceDict objectForKey: key];
		NSString *ip = [PAServiceDiscovery ipOfService: service];

		//if ([ip isEqualToString: serverIP]) {
		//	[serverSelectButton selectItemWithTitle: [service name]];
		//	found = YES;
		//}
	}
	
	//if (!found)
	//	[serverSelectButton selectItemWithTitle: serverIP];

	NSInteger connectionStatus = [[client objectForKey: @"connectionStatus"] intValue];
	[connectionStatusTextField setStringValue: [connectionStatusStrings objectAtIndex: connectionStatus]];
	
	[persistenCheckButton setTitle: [NSString stringWithFormat:
			@"Always use these parameters for %@", app.localizedName]];		
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

	// ...
	
	[client setObject: serverName
		   forKey: @"serverName"];
}

#pragma mark ### PAServiceDiscoveryDelegate ###

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	       sinkAppeared: (NSNetService *) service
{
	NSString *name = [service name];
	NSArray *addresses = [service addresses];
	
	if ([addresses count] == 0)
		return;
	
	[serverSelectButton addItemWithTitle: name];	
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	    sinkDisappeared: (NSNetService *) service
{
	[serverSelectButton removeItemWithTitle: [service name]];
}

@end