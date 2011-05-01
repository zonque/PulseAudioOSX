//
//  Introspect.m
//  PulseConsole
//
//  Created by Daniel on 30.04.11.
//  Copyright 2011 caiaq. All rights reserved.
//

#import <PulseAudio/PulseAudio.h>
#import "Introspect.h"

@implementation Introspect

@synthesize connection;

#pragma mark ### fooo ###
- (void) enableGUI: (BOOL) enabled
{
	if (!enabled)
		activeItem = nil;

	[outlineView setEnabled: enabled];
	[parameterTableView setEnabled: enabled];
	[propertyTableView setEnabled: enabled];
}

- (void) repaintViews
{
	[outlineView reloadItem: nil
		 reloadChildren: YES];
	[outlineView expandItem: nil
		 expandChildren: YES];
	[parameterTableView reloadData];
	[propertyTableView reloadData];
}

- (void) awakeFromNib
{
	outlineToplevel = [NSMutableArray arrayWithCapacity: 0];
	[outlineToplevel retain];
		
	serverinfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	cards = [NSMutableArray arrayWithCapacity: 0];
	sinks = [NSMutableArray arrayWithCapacity: 0];
	sources = [NSMutableArray arrayWithCapacity: 0];
	clients = [NSMutableArray arrayWithCapacity: 0];
	modules = [NSMutableArray arrayWithCapacity: 0];
	samplecache = [NSMutableArray arrayWithCapacity: 0];
	
	NSMutableDictionary *d;
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Server Information"
	      forKey: @"label"];
	[d setObject: serverinfo
	      forKey: @"parameters"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Cards"
	      forKey: @"label"];
	[d setObject: cards
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sinks"
	      forKey: @"label"];
	[d setObject: sinks
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sources"
	      forKey: @"label"];
	[d setObject: sources
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Clients"
	      forKey: @"label"];
	[d setObject: clients
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Modules"
	      forKey: @"label"];
	[d setObject: modules
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sample Cache"
	      forKey: @"label"];
	[d setObject: samplecache
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	[outlineView setEnabled: NO];
	[parameterTableView setEnabled: NO];
	[propertyTableView setEnabled: NO];
}

- (void) dealloc
{
	[serverinfo removeAllObjects];
	[serverinfo release];
	
	[sinks removeAllObjects];
	[sinks release];
	
	[sources removeAllObjects];
	[sources release];
	
	[cards removeAllObjects];
	[cards release];
	
	[modules removeAllObjects];
	[modules release];
	
	[clients removeAllObjects];
	[clients release];
	
	[super dealloc];
}

- (void) invalidateAll
{
	[sinks removeAllObjects];
	[sources removeAllObjects];
	[modules removeAllObjects];
	[clients removeAllObjects];
	[samplecache removeAllObjects];
	[serverinfo removeAllObjects];
	[cards removeAllObjects];
}	

#pragma mark ### NSOutlineViewDataSource protocol ###

- (NSInteger) outlineView: (NSOutlineView *) outlineView numberOfChildrenOfItem: (id) item
{
	if (item == nil) {
		NSEnumerator *enumerator = [outlineToplevel objectEnumerator];
		NSString *obj;
		UInt count = 0;
		
		while ((obj = [enumerator nextObject])) {
			NSDictionary *d = [obj valueForKey: @"children"];
			if (d && [d count]) {
				count++;
				continue;
			}
			
			d = [obj valueForKey: @"parameters"];
			if (d && [d count]) {
				count++;
				continue;
			}
		}
                
		return count;
	}
	
	NSArray *d = [item valueForKey: @"children"];
	
	return d ? [d count] : 0;
}

- (BOOL) outlineView: (NSOutlineView *) outlineView isItemExpandable: (id)item
{
	if (item == nil)
		return [outlineToplevel count] > 0;
	
	NSArray *d = [item valueForKey: @"children"];
	return d ? [d count] > 0 : NO;
}

- (id) outlineView: (NSOutlineView *) outlineView child: (NSInteger)index
	    ofItem: (id)item
{
	if (item == nil)
		return [outlineToplevel objectAtIndex: index];
	
	NSArray *d = [item valueForKey: @"children"];
	return [d objectAtIndex: index];
}

- (id) outlineView: (NSOutlineView *) outlineView objectValueForTableColumn: (NSTableColumn *) tableColumn
	    byItem: (id)item
{
	return [item valueForKey: @"label"];
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
	
	if (!activeItem)
		return @"";
	
	if (tableView == parameterTableView)
		item = [activeItem valueForKey: @"parameters"];
	else if (tableView == propertyTableView)
		item = [activeItem valueForKey: @"properties"];
	
	if (!item)
		return @"";
	
	if ([[col identifier] isEqualToString: @"key"])
		return [[item allKeys] objectAtIndex: rowIndex];
	
	if ([[col identifier] isEqualToString: @"value"])
		return [[item allValues] objectAtIndex: rowIndex];
	
	return @"";
}

- (NSInteger) numberOfRowsInTableView: (NSTableView *) tableView
{
	NSDictionary *item = nil;
	
	if (tableView == parameterTableView)
		item = [activeItem valueForKey: @"parameters"];
	else if (tableView == propertyTableView)
		item = [activeItem valueForKey: @"properties"];
	
	return item ? [item count] : 0;
}

#pragma mark ### delegate methods ###

- (BOOL) outlineView: (NSOutlineView *) outlineView shouldEditTableColumn: (NSTableColumn *)tableColumn item:(id)item
{
	return NO;
}

- (void) outlineViewSelectionDidChange: (NSNotification *) notification
{
	NSDictionary *d = [outlineView itemAtRow: [outlineView selectedRow]];
	
	activeItem = d;
	[parameterTableView reloadData];
	[propertyTableView reloadData];
}

#pragma mark ### PAServerConnectionDelegate ###

- (void) serverInfoChanged: (PAServerInfo *) info
{
	[serverinfo setObject: info.userName
		       forKey: @"User Name"];
	[serverinfo setObject: info.hostName
		       forKey: @"Host Name"];
	[serverinfo setObject: info.version
		       forKey: @"Server Version"];
	[serverinfo setObject: info.serverName
		       forKey: @"Server Name"];
	[serverinfo setObject: info.sampleSpec
		       forKey: @"Sample Spec"];
	[serverinfo setObject: info.channelMap
		       forKey: @"Channel Map"];
	[serverinfo setObject: info.defaultSinkName
		       forKey: @"Default Sink Name"];
	[serverinfo setObject: info.defaultSourceName
		       forKey: @"Default Source Name"];
	[serverinfo setObject: [NSNumber numberWithInt: info.cookie]
		       forKey: @"Cookie"];
	
	[self repaintViews];
}

- (void) cardsChanged: (NSArray *) _cards
{
	[cards removeAllObjects];
	
	for (PACardInfo *card in _cards) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: card.name
			       forKey: @"Card Name"];
		[parameters setObject: card.driver
			       forKey: @"Driver"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: card.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: card.properties
		      forKey: @"properties"];
		
		[cards addObject: d];
	}
	
	[self repaintViews];
}

- (void) sinksChanged: (NSArray *) _sinks
{
	[sinks removeAllObjects];
	
	for (PASinkInfo *sink in _sinks) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: sink.name
			       forKey: @"Sink Name"];
		[parameters setObject: sink.description
			       forKey: @"Description"];
		[parameters setObject: sink.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: sink.channelMap
			       forKey: @"Channel Map"];		
		[parameters setObject: [NSNumber numberWithInt: sink.latency]
			       forKey: @"Latency (us)"];		
		[parameters setObject: sink.driver
			       forKey: @"Driver"];		
		[parameters setObject: [NSNumber numberWithInt: sink.configuredLatency]
			       forKey: @"Configured Latency (us)"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: sink.name
		      forKey: @"label"];
		[d setObject: [NSNumber numberWithInt: sink.volume]
		      forKey: @"volume"];
		[d setObject: [NSNumber numberWithInt: sink.nVolumeSteps]
		      forKey: @"n_volume_steps"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: sink.properties
		      forKey: @"properties"];
		[d setObject: [NSValue valueWithPointer: sink]
		      forKey: @"infoPointer"];
		
		[sinks addObject: d];		
	}
	
	[self repaintViews];
}

- (void) sourcesChanged: (NSArray *) _sources
{
	[sources removeAllObjects];
	
	for (PASourceInfo *source in _sources) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: source.name
			       forKey: @"Source Name"];
		[parameters setObject: source.description
			       forKey: @"Description"];
		[parameters setObject: source.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: source.channelMap
			       forKey: @"Channel Map"];		
		[parameters setObject: [NSNumber numberWithInt: source.latency]
			       forKey: @"Latency (us)"];		
		[parameters setObject: source.driver
			       forKey: @"Driver"];		
		[parameters setObject: [NSNumber numberWithInt: source.configuredLatency]
			       forKey: @"Configured Latency (us)"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: source.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: source.properties
		      forKey: @"properties"];
		[d setObject: [NSValue valueWithPointer: source]
		      forKey: @"infoPointer"];
		
		[sources addObject: d];		
	}	
	
	[self repaintViews];
}

- (void) clientsChanged: (NSArray *) _clients
{
	[clients removeAllObjects];
	
	for (PAClientInfo *client in _clients) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: client.name
			       forKey: @"Module Name"];
		[parameters setObject: client.driver
			       forKey: @"Driver"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: client.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: client.properties
		      forKey: @"properties"];
		
		[clients addObject: d];		
	}
	
	[self repaintViews];
}

- (void) modulesChanged: (NSArray *) _modules
{
	[modules removeAllObjects];
	
	for (PAModuleInfo *module in _modules) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: module.name
			       forKey: @"Module Name"];
		[parameters setObject: module.argument ?: @""
			       forKey: @"Arguments"];
		[parameters setObject: [NSNumber numberWithInt: module.useCount]
			       forKey: @"Use count"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: module.name
		      forKey: @"label"];		
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: module.properties
		      forKey: @"properties"];
		
		[modules addObject: d];
	}	
	
	[self repaintViews];
}

- (void) samplesChanged: (NSArray *) _samples
{
	[samplecache removeAllObjects];
	
	for (PASampleInfo *sample in _samples) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: sample.name
			       forKey: @"Name"];
		[parameters setObject: sample.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: sample.channelMap
			       forKey: @"Channel Map"];
		[parameters setObject: sample.fileName
			       forKey: @"File Name"];		
		[parameters setObject: [NSNumber numberWithInt: sample.duration]
			       forKey: @"Duration"];
		[parameters setObject: [NSNumber numberWithInt: sample.bytes]
			       forKey: @"bytes"];
		[parameters setObject: sample.lazy ? @"YES" : @"NO"
			       forKey: @"Lazy"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: sample.name
		      forKey: @"label"];		
		[d setObject: parameters
		      forKey: @"parameters"];
		
		[samplecache addObject: d];
	}		
	
	[self repaintViews];
}

@end
