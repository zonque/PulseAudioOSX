/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <PulseAudio/PulseAudio.h>

#import "StreamListView.h"
#import "Server.h"

@implementation Server

@synthesize delegate;

- (void) stopProgressIndicator
{
	[connectionProgressIndicator stopAnimation: self];
	[connectionProgressIndicator setHidden: YES];
}

- (void) connectToServer: (NSString *) server;
{
	[introspect invalidateAll];

	[connectStatus setStringValue: @"Connecting ..."];
	[connectionProgressIndicator startAnimation: self];
	[connectionProgressIndicator setHidden: NO];
	
	[connection disconnect];	
	[connection connectToHost: server
			     port: -1];
}

- (void) enableGUI: (BOOL) enabled
{
	if (enabled)
		[connectionProgressIndicator stopAnimation: self];
	else
		[connectionProgressIndicator startAnimation: self];
	
	[connectionProgressIndicator setHidden: enabled];
	[statisticsTableView setEnabled: enabled];
	
	[introspect enableGUI: enabled];
}

- (void) repaintViews: (NSNotification *) notification
{
	[statisticsTableView reloadData];
	[introspect repaintViews];
}

- (id) init
{
	[super init];
	NSLog(@" INIT ");
	return self;
}

#pragma mark ### NSDocument ###

- (void) sheetDidEnd: (NSWindow *) sheet
	  returnCode: (NSInteger) returnCode
	 contextInfo: (void *)contextInfo
{
	NSLog(@"sheet did end");
}

- (void) awakeFromNib
{
	[statisticsTableView setEnabled: NO];
	
	statisticDict = [NSMutableDictionary dictionaryWithCapacity: 0];
	[statisticDict retain];
	
	connection = [[PAServerConnection alloc] init];
	connection.delegate = self;
	
	introspect.connection = connection;
	
	sinkStreamListView.streamType = StreamTypeSink;
	sourceStreamListView.streamType = StreamTypeSource;
}

- (void) dealloc
{
	[statisticDict removeAllObjects];
	[statisticDict release];
	
	
	[super dealloc];
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView: (NSTableView *) aTableView
   setObjectValue: obj
   forTableColumn: (NSTableColumn *) col
	      row: (NSInteger) rowIndex
{
}

- (id)tableView: (NSTableView *) tableView
objectValueForTableColumn: (NSTableColumn *) col
	    row: (NSInteger) rowIndex
{
	if (tableView == statisticsTableView) {
		if ([[col identifier] isEqualToString: @"key"])
			return [[statisticDict allKeys] objectAtIndex: rowIndex];
	
		if ([[col identifier] isEqualToString: @"value"])
			return [[statisticDict allValues] objectAtIndex: rowIndex];
	}

	return @"";
}

- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView
{
	NSDictionary *item = nil;
	
	if (tableView == statisticsTableView)
		item = statisticDict;
	
	return item ? [item count] : 0;
}

#pragma mark ### PAServerConnectionDelegate ###

- (void) PAServerConnectionEstablished: (PAServerConnection *) c
{
	[window setTitle: [connection serverName]];
	[self enableGUI: YES];
}

- (void) signalEnd
{
	[delegate serverSignalledEnd: self];	
}

- (void) PAServerConnectionFailed: (PAServerConnection *) c
{
	[self enableGUI: NO];
	NSBeginCriticalAlertSheet(@"Connection failed",
				  @"Close",
				  nil, nil,
				  window,
				  self,
				  @selector(signalEnd),
				  @selector(signalEnd),
				  NULL,
				  [connection lastError]);
}

- (void) PAServerConnectionEnded: (PAServerConnection *) c
{
	[self enableGUI: NO];
	NSBeginCriticalAlertSheet(@"Connection terminated",
				  @"Close",
				  nil, nil,
				  window,
				  self,
				  @selector(signalEnd),
				  @selector(signalEnd),
				  NULL,
				  [connection lastError]);	
}

- (void) PAServerConnection: (PAServerConnection *) connection
	  serverInfoChanged: (PAServerInfo *) info
{
	[introspect serverInfoChanged: info];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	       cardsChanged: (NSArray *) cards
{
	[introspect cardsChanged: cards];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	       sinksChanged: (NSArray *) sinks
{
	[sinkStreamListView sinksChanged: sinks];
	[introspect sinksChanged: sinks];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	  sinkInputsChanged: (NSArray *) inputs
{
	[introspect sinkInputsChanged: inputs];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     sourcesChanged: (NSArray *) sources
{
	[sourceStreamListView sourcesChanged: sources];
	[introspect sourcesChanged: sources];
}

- (void) PAServerConnection: (PAServerConnection *) connection
       sourceOutputsChanged: (NSArray *) outputs
{
	[introspect sourceOutputsChanged: outputs];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     clientsChanged: (NSArray *) clients
{
	[introspect clientsChanged: clients];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     modulesChanged: (NSArray *) modules
{
	[introspect modulesChanged: modules];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     samplesChanged: (NSArray *) samples
{
	[introspect samplesChanged: samples];
}

#if 0
- (void) statCallback: (const pa_stat_info *) i
{
	char t[32];
	
	[statisticDict setObject: [NSNumber numberWithInt: i->memblock_total]
			  forKey: @"Currently allocated memory blocks"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->memblock_total_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Current total size of allocated memory blocks"];
	[statisticDict setObject: [NSNumber numberWithInt: i->memblock_allocated]
			  forKey: @"Allocated memory blocks during the whole lifetime of the daemon"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->memblock_allocated_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Total size of all memory blocks allocated during the whole lifetime of the daemon"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->scache_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Total size of all sample cache entries"];     
	
	[self detailsChanged];
}
#endif

#pragma mark ### IBActions ###

- (IBAction) reloadStatistics: (id) sender
{
	//[serverConnection reloadStatistics];
}

@end