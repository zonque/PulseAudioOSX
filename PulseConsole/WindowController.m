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
#import "WindowController.h"

@implementation WindowController

#pragma mark ### ServerConnection callbacks ###

- (void) connectionEstablished
{
	[self enableGUI: YES];
}

- (void) connectionEnded
{
	[self enableGUI: NO];	
}

- (void) stopProgressIndicator
{
	[connectionProgressIndicator stopAnimation: self];
	[connectionProgressIndicator setHidden: YES];
}

- (void) bonjourServiceAdded: (NSNotification *) notification
{
	NSDictionary *dict = [notification userInfo];
	NSString *name = [dict valueForKey: @"name"];
	
	[serverSelector addItemWithTitle: name];
	
	if ([serverSelector numberOfItems] == 1) {
		[self connectToServer: name];
		[connectionProgressIndicator startAnimation: self];
	}
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
	
	//	[window setTitle: [NSString stringWithFormat: @"%@@%@",
	//			   [serverConnection.serverinfo valueForKey: @"Server Name"],
	//			   [serverConnection.serverinfo valueForKey: @"Host Name"]]];
	
	[sinkStreamListView removeAllStreams];
	[sourceStreamListView removeAllStreams];
	[playbackStreamListView removeAllStreams];
	[recordStreamListView removeAllStreams];	
	
	NSDictionary *item;
	
	for (item in serverConnection.sinks)
		[sinkStreamListView addStreamView: [[item objectForKey: @"infoPointer"] pointerValue]
					   ofType: StreamTypeSink
					     name: [item objectForKey: @"label"]];
	
	[introspect repaintViews];
}

- (void) awakeFromNib
{
	[statisticsTableView setEnabled: NO];

	serverConnection = [[ServerConnection alloc] init];

	introspect.serverConnection = serverConnection;
	
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionEstablished)
						     name: @"connectionEstablished"
						   object: serverConnection];
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionEnded)
						     name: @"connectionEnded"
						   object: serverConnection];
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(repaintViews:)
						     name: @"detailsChanged"
						   object: serverConnection];
		
	[serverConnection connectToServer: @"localhost"];
}

- (void) dealloc
{
	[serverConnection release];
	[super dealloc];
}

- (void) connectToServer: (NSString *) server
{
	[connectStatus setStringValue: @"Connecting ..."];
	[connectionProgressIndicator startAnimation: self];
	[connectionProgressIndicator setHidden: NO];
	
	[serverConnection connectToServer: server];
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView:(NSTableView *)aTableView
   setObjectValue:obj
   forTableColumn:(NSTableColumn *)col
	      row:(NSInteger)rowIndex
{
}

- (id)tableView:(NSTableView *)tableView
objectValueForTableColumn:(NSTableColumn *)col
	    row:(NSInteger)rowIndex
{
	NSDictionary *item = nil;
	
	if (tableView == statisticsTableView)
		item = serverConnection.statisticDict;
	
	if (!item)
		return @"";
	
	if ([[col identifier] isEqualToString: @"key"])
		return [[item allKeys] objectAtIndex: rowIndex];
	
	if ([[col identifier] isEqualToString: @"value"])
		return [[item allValues] objectAtIndex: rowIndex];
	
	return @"";
}

- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView
{
	NSDictionary *item = nil;
	
	if (tableView == statisticsTableView)
		item = serverConnection.statisticDict;
	
	return item ? [item count] : 0;
}

#pragma mark ### IBActions ###

- (IBAction) connectToServerAction: (id) sender
{	
	[introspect enableGUI: NO];
	//[self repaintViews: nil];
	
	[self connectToServer: [sender titleOfSelectedItem]];
}

- (IBAction) displayAbout: (id) sender
{
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	//[d setValue: @"(c) 2009-2011 Daniel Mack"
	//     forKey: @"Copyright"];
	[d setValue: [NSString stringWithFormat: @"pulseaudio library version %@",
						[PAServerConnection pulseAudioLibraryVersion]]
	     forKey: @"Copyright"];
	
	[NSApp orderFrontStandardAboutPanelWithOptions: d];
}

- (IBAction) reloadStatistics: (id) sender
{
	[serverConnection reloadStatistics];
}

@end