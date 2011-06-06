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

@synthesize delegate;

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
        knownServers = [[NSMutableArray arrayWithCapacity: 0] retain];
        knownSinks = [[NSMutableArray arrayWithCapacity: 0] retain];
        knownSources = [[NSMutableArray arrayWithCapacity: 0] retain];

        [serverSelectButton removeAllItems];
        [sinkSelectButton removeAllItems];
        [sourceSelectButton removeAllItems];

//        [serverSelectButton addItemWithTitle: @"localhost"];

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

- (void) tableView: (NSTableView *) aTableView
    setObjectValue: obj
    forTableColumn: (NSTableColumn *) col
               row: (int) rowIndex
{
}

- (id) tableView: (NSTableView *) tableView
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
        [serverSelectButton selectItemWithTitle: [client objectForKey: @"serverName"]];

        NSData *data;

        for (NSNetService *s in knownSinks) {
                NSDictionary *txt = [NSNetService dictionaryFromTXTRecordData: [s TXTRecordData]];
                data = [txt objectForKey: @"description"];
                NSString *description = [NSString stringWithCString: [data bytes]
                                                           encoding: NSASCIIStringEncoding];
                data = [txt objectForKey: @"device"];
                NSString *device = [NSString stringWithCString: [data bytes]
                                                      encoding: NSASCIIStringEncoding];

                if ([device isEqualToString: [client objectForKey: @"sinkForPlayback"]]) {
                        [sinkSelectButton selectItemWithTitle: description];
                        break;
                }
        }

        for (NSNetService *s in knownSources) {
                NSDictionary *txt = [NSNetService dictionaryFromTXTRecordData: [s TXTRecordData]];
                data = [txt objectForKey: @"description"];
                NSString *description = [NSString stringWithCString: [data bytes]
                                                           encoding: NSASCIIStringEncoding];
                data = [txt objectForKey: @"device"];
                NSString *device = [NSString stringWithCString: [data bytes]
                                                      encoding: NSASCIIStringEncoding];

                if ([device isEqualToString: [client objectForKey: @"sourceForRecord"]]) {
                        [sinkSelectButton selectItemWithTitle: description];
                        break;
                }
        }

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
        NSMutableDictionary *config = [NSMutableDictionary dictionaryWithCapacity: 0];
        NSNumber *serverPort = NULL;
        NSNetService *service = [knownServers objectAtIndex: [serverSelectButton indexOfSelectedItem]];
        NSString *serverName = [service hostName]; //[PAServiceDiscovery ipOfService: service];
	
        [config setObject: [serverSelectButton titleOfSelectedItem]
                   forKey: @"serverName"];
        [config setObject: [sinkSelectButton titleOfSelectedItem]
                   forKey: @"sinkName"];
        [config setObject: [sourceSelectButton titleOfSelectedItem]
                   forKey: @"sourceName"];
        [config setObject: pid
                   forKey: @"pid"];

        if (serverPort)
                [config setObject: serverPort
                           forKey: @"serverPort"];
        
        [config setObject: serverName
                   forKey: @"serverName"];
	
        [delegate setAudioDeviceConfig: config
                     forDeviceWithUUID: [client objectForKey: @"uuid"]];
}

- (IBAction) selectServer: (id) sender
{
        NSInteger selected = [serverSelectButton indexOfSelectedItem];
	
	NSLog(@" ... selected %d", selected);
	
	if (selected < 0)
		return;

        NSNetService *server = [knownServers objectAtIndex: selected];
        NSDictionary *txt = [NSNetService dictionaryFromTXTRecordData: [server TXTRecordData]];
        NSData *serverMachine = [txt objectForKey: @"machine-id"];

	[sinkSelectButton removeAllItems];
        for (NSNetService *s in knownSinks) {
                NSDictionary *txt = [NSNetService dictionaryFromTXTRecordData: [s TXTRecordData]];
                NSData *machine = [txt objectForKey: @"machine-id"];

                if (machine && [machine isEqualToData: serverMachine]) {
                        NSData *description = [txt objectForKey: @"description"];
                        [sinkSelectButton addItemWithTitle: [NSString stringWithCString: [description bytes]
                                                                               encoding: NSASCIIStringEncoding]];
                }
        }

        [sourceSelectButton removeAllItems];
        for (NSNetService *s in knownSources) {
                NSDictionary *txt = [NSNetService dictionaryFromTXTRecordData: [s TXTRecordData]];
                NSData *machine = [txt objectForKey: @"machine-id"];

                if (machine && [machine isEqualToData: serverMachine]) {
                        NSData *description = [txt objectForKey: @"description"];
                        [sourceSelectButton addItemWithTitle: [NSString stringWithCString: [description bytes]
                                                                                 encoding: NSASCIIStringEncoding]];
                }
        }
}

#pragma mark ### PAServiceDiscoveryDelegate ###

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
             serverAppeared: (NSNetService *) service
{
        NSString *name = [service name];
        NSArray *addresses = [service addresses];

        if ([addresses count] == 0)
                return;

        [knownServers addObject: service];
        [serverSelectButton addItemWithTitle: name];

	NSInteger selected = [serverSelectButton indexOfSelectedItem];
	if (selected < 0)
		[serverSelectButton selectItemAtIndex: 0];	

        [self selectServer: nil];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
          serverDisappeared: (NSNetService *) service
{
        [knownServers addObject: service];
        [serverSelectButton removeItemWithTitle: [service name]];
        [self selectServer: nil];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
               sinkAppeared: (NSNetService *) service
{
        [knownSinks addObject: service];
        [self selectServer: nil];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
            sinkDisappeared: (NSNetService *) service
{
        [knownSinks removeObject: service];
        [self selectServer: nil];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
             sourceAppeared: (NSNetService *) service
{
        [knownSources addObject: service];
        [self selectServer: nil];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
          sourceDisappeared: (NSNetService *) service
{
        [knownSources removeObject: service];
        [self selectServer: nil];
}

@end
