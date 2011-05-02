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

	NSLog(@" %s() %d selectionTableView %@", __func__, enabled, selectionTableView);

	[selectionTableView setEnabled: enabled];
	[parameterTableView setEnabled: enabled];
	[propertyTableView setEnabled: enabled];
}

- (void) repaintViews
{
	[selectionTableView reloadData];
	[parameterTableView reloadData];
	[propertyTableView reloadData];
}

- (void) contentChanged
{
	activeItem = nil;
	[selectionTableView deselectAll: nil];
	[self repaintViews];
}

- (void) awakeFromNib
{
	outlineToplevel = [NSMutableArray arrayWithCapacity: 0];
	[outlineToplevel retain];

	serverinfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	cards = [NSMutableArray arrayWithCapacity: 0];
	sinks = [NSMutableArray arrayWithCapacity: 0];
	sinkInputs = [NSMutableArray arrayWithCapacity: 0];
	sources = [NSMutableArray arrayWithCapacity: 0];
	sourceOutputs = [NSMutableArray arrayWithCapacity: 0];
	clients = [NSMutableArray arrayWithCapacity: 0];
	modules = [NSMutableArray arrayWithCapacity: 0];
	samplecache = [NSMutableArray arrayWithCapacity: 0];
	
	NSMutableDictionary *d, *v;

	v = [NSMutableDictionary dictionaryWithCapacity: 0];
	[v setObject: @"Server Information"
	      forKey: @"label"];
	[v setObject: serverinfo
	      forKey: @"parameters"];

	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"General"
	      forKey: @"label"];
	[d setObject: [NSArray arrayWithObject: v]
	      forKey: @"children"];
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
	[d setObject: @"Sink inputs"
	      forKey: @"label"];
	[d setObject: sinkInputs
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Source outputs"
	      forKey: @"label"];
	[d setObject: sourceOutputs
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
	
	[selectionTableView setEnabled: NO];
	[parameterTableView setEnabled: NO];
	[propertyTableView setEnabled: NO];
}

- (void) dealloc
{
	[serverinfo removeAllObjects];
	[serverinfo release];
	
	[sinks removeAllObjects];
	[sinks release];

	[sinkInputs removeAllObjects];
	[sinkInputs release];

	[sources removeAllObjects];
	[sources release];

	[sourceOutputs removeAllObjects];
	[sourceOutputs release];
	
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
	[sinkInputs removeAllObjects];
	[sources removeAllObjects];
	[sourceOutputs removeAllObjects];
	[modules removeAllObjects];
	[clients removeAllObjects];
	[samplecache removeAllObjects];
	[serverinfo removeAllObjects];
	[cards removeAllObjects];
}	

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView: (NSTableView *) aTableView
   setObjectValue: obj
   forTableColumn: (NSTableColumn *)col
	      row: (NSInteger)rowIndex
{
}

- (id)tableView:(NSTableView *)tableView
objectValueForTableColumn:(NSTableColumn *)col
	    row:(NSInteger)rowIndex
{
	NSDictionary *item = nil;
	
	if (tableView == selectionTableView) {
		UInt32 index = 0;

		for (NSDictionary *d in outlineToplevel) {
			if (index == rowIndex)
				return [d objectForKey: @"label"];
			
			index++;

			NSArray *children = [d objectForKey: @"children"];

			if (index + [children count] > rowIndex) {
				NSDictionary *child = [children objectAtIndex: rowIndex - index];
				NSString *label = [child objectForKey: @"label"];
				return [NSString stringWithFormat: @"     %@", label];
			}

			index += [children count];
		}

		return @"???";
	}

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
	
	if (tableView == selectionTableView) {
		UInt32 count = 0;

		for (NSDictionary *d in outlineToplevel) {
			NSArray *children = [d objectForKey: @"children"];
			count += [children count] + 1;
		}
		
		return count;
	}
	
	if (!activeItem)
		return 0;
		
	if (tableView == parameterTableView)
		item = [activeItem valueForKey: @"parameters"];
	else if (tableView == propertyTableView)
		item = [activeItem valueForKey: @"properties"];
	
	return item ? [item count] : 0;
}

#pragma mark ### NSTableViewDelegate protocol ###

- (BOOL) tableView: (NSTableView *) tableView
       isHeaderRow: (NSInteger) row
{
	if (tableView != selectionTableView)
		return NO;

	UInt32 index = 0;

	for (NSDictionary *d in outlineToplevel) {
		if (index == row)
			return YES;

		NSArray *children = [d objectForKey: @"children"];
		index += [children count] + 1;
	}

	return NO;
}

- (BOOL)tableView: (NSTableView *) aTableView
  shouldSelectRow: (NSInteger) rowIndex
{
	return ![self tableView: aTableView
		    isHeaderRow: rowIndex];
}

- (void) tableViewSelectionDidChange: (NSNotification *) notification
{
	if ([notification object] != selectionTableView)
		return;
	
	UInt32 selected = [selectionTableView selectedRow];
	UInt32 index = 0;
	
	for (NSDictionary *d in outlineToplevel) {
		index++;

		NSArray *children = [d objectForKey: @"children"];
		
		if (index + [children count] > selected) {
			activeItem = [children objectAtIndex: selected - index];
			[self repaintViews];
			return;
		}
		
		index += [children count];
	}
}

- (void) tableView: (NSTableView *) aTableView
   willDisplayCell: (id) aCell
    forTableColumn: (NSTableColumn *) aTableColumn
	       row: (NSInteger) rowIndex
{
	if (aTableView != selectionTableView)
		return;
	
	if ([self tableView: aTableView
		isHeaderRow: rowIndex]) {
		[aCell setFont: [NSFont boldSystemFontOfSize: 12.0]];
	} else {
		[aCell setFont: [NSFont systemFontOfSize: 11.0]];
	}

	
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
	
	[self contentChanged];
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
	
	[self contentChanged];
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
	
	[self contentChanged];
}

- (void) sinkInputsChanged: (NSArray *) inputs
{
	[sinkInputs removeAllObjects];
	
	for (PASinkInputInfo *input in inputs) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: input.name
			       forKey: @"Sink Input Name"];
		[parameters setObject: [NSNumber numberWithInt: [input.channelNames count]]
			       forKey: @"Number of channels"];
		[parameters setObject: [NSNumber numberWithInt: input.sinkUsec]
			       forKey: @"Sink Latency (us)"];		
		[parameters setObject: [NSNumber numberWithInt: input.bufferUsec]
			       forKey: @"Buffer Latency (us)"];		
		[parameters setObject: input.driver
			       forKey: @"Driver"];

		if (input.resampleMethod)
			[parameters setObject: input.resampleMethod
				       forKey: @"Resample method"];
		
		[parameters setObject: [NSNumber numberWithInt: input.index]
			       forKey: @"Index"];
		[parameters setObject: [NSNumber numberWithInt: input.volume]
			       forKey: @"Volume"];
		[parameters setObject: [NSNumber numberWithBool: input.muted]
			       forKey: @"Muted"];
		[parameters setObject: [NSNumber numberWithBool: input.volumeWriteable]
			       forKey: @"Volume writeable"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: input.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: input.properties
		      forKey: @"properties"];
		
		[sinkInputs addObject: d];		
	}
	
	[self contentChanged];
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
	
	[self contentChanged];
}

- (void) sourceOutputsChanged: (NSArray *) outputs
{
	[sourceOutputs removeAllObjects];
	
	for (PASourceOutputInfo *output in outputs) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: output.name
			       forKey: @"Source Output Name"];
		[parameters setObject: [NSNumber numberWithInt: [output.channelNames count]]
			       forKey: @"Number of channels"];
		[parameters setObject: [NSNumber numberWithInt: output.sourceUsec]
			       forKey: @"Source Latency (us)"];		
		[parameters setObject: [NSNumber numberWithInt: output.bufferUsec]
			       forKey: @"Buffer Latency (us)"];		
		[parameters setObject: output.driver
			       forKey: @"Driver"];
		
		if (output.resampleMethod)
			[parameters setObject: output.resampleMethod
				       forKey: @"Resample method"];
				
		[parameters setObject: [NSNumber numberWithInt: output.index]
			       forKey: @"Index"];
		[parameters setObject: [NSNumber numberWithBool: output.corked]
			       forKey: @"Corked"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: output.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: output.properties
		      forKey: @"properties"];
		
		[sourceOutputs addObject: d];		
	}
	
	[self contentChanged];
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
	
	[self contentChanged];
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
	
	[self contentChanged];
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
	
	[self contentChanged];
}

@end
