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
#import "Introspect.h"

@implementation Introspect

@synthesize connection;

#pragma mark ### fooo ###
- (void) enableGUI: (BOOL) enabled
{
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

- (id) init
{
        [super init];

        outlineToplevel = [[NSMutableArray arrayWithCapacity: 0] retain];
        parameters = [[NSMutableDictionary dictionaryWithCapacity: 0] retain];
        properties = [[NSMutableDictionary dictionaryWithCapacity: 0] retain];

        return self;
}

- (void) serverReady
{
        [outlineToplevel removeAllObjects];
        [parameters removeAllObjects];
        [properties removeAllObjects];

        NSMutableDictionary *d;

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"General"
              forKey: @"label"];
        [d setObject: [NSArray arrayWithObject: [connection serverInfo]]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Cards"
              forKey: @"label"];
        [d setObject: [connection presentCards]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Sinks"
              forKey: @"label"];
        [d setObject: [connection presentSinks]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Sources"
              forKey: @"label"];
        [d setObject: [connection presentSources]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Sink inputs"
              forKey: @"label"];
        [d setObject: [connection presentSinkInputs]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Source outputs"
              forKey: @"label"];
        [d setObject: [connection presentSourceOutputs]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Clients"
              forKey: @"label"];
        [d setObject: [connection presentClients]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Modules"
              forKey: @"label"];
        [d setObject: [NSNumber numberWithBool: YES]
              forKey: @"canAdd"];
        [d setObject: [connection presentModules]
              forKey: @"children"];
        [outlineToplevel addObject: d];

        d = [NSMutableDictionary dictionaryWithCapacity: 0];
        [d setObject: @"Sample Cache"
              forKey: @"label"];
        [d setObject: [connection presentSamples]
              forKey: @"children"];
        [outlineToplevel addObject: d];
}

- (void) awakeFromNib
{
        [selectionTableView setEnabled: NO];
        [parameterTableView setEnabled: NO];
        [propertyTableView setEnabled: NO];

        // color picked from iTunes
        [selectionTableView setBackgroundColor: [NSColor colorWithCalibratedRed: 217.0 / 255.0
                                                                          green: 223.0 / 255.0
                                                                           blue: 230.0 / 255.0
                                                                          alpha: 1.0]];
}

- (void) dealloc
{
        [parameters release];
        [properties release];
        [super dealloc];
}

- (void) invalidateAll
{
}

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

- (NSObject *) childForRow: (NSInteger) rowIndex
{
        UInt32 index = 0;

        for (NSDictionary *d in outlineToplevel) {
                if (index == rowIndex)
                        return d;

                index++;

                NSArray *children = [d objectForKey: @"children"];

                if (index + [children count] > rowIndex) {
                        NSDictionary *child = [children objectAtIndex: rowIndex - index];
                        return child;
                }

                index += [children count];
        }

        return nil;
}

- (NSInteger) rowForChild: (PAElementInfo *) child
{
        UInt32 index = 0;

        for (NSDictionary *d in outlineToplevel) {
                index++;

                NSArray *children = [d objectForKey: @"children"];

                for (PAElementInfo *e in children) {
                        if (e == child)
                                return index;

                        index++;
                }
        }

        return -1;
}

- (void) setActiveItem: (NSObject *) item
{
        [parameters removeAllObjects];
        [properties removeAllObjects];

        if ([[item className] isEqualToString: @"PAServerInfo"]) {
                PAServerInfo *info = (PAServerInfo *) item;

                [parameters setObject: info.userName
                               forKey: @"User Name"];
                [parameters setObject: info.hostName
                               forKey: @"Host Name"];
                [parameters setObject: info.version
                               forKey: @"Server Version"];
                [parameters setObject: info.serverName
                               forKey: @"Server Name"];
                [parameters setObject: info.sampleSpec
                               forKey: @"Sample Spec"];
                [parameters setObject: info.channelMap
                               forKey: @"Channel Map"];
                [parameters setObject: info.defaultSinkName
                               forKey: @"Default Sink Name"];
                [parameters setObject: info.defaultSourceName
                               forKey: @"Default Source Name"];
                [parameters setObject: [NSNumber numberWithInt: info.cookie]
                               forKey: @"Cookie"];
        }

        if ([[item className] isEqualToString: @"PACardInfo"]) {
                PACardInfo *card = (PACardInfo *) item;

                [parameters setObject: card.name
                               forKey: @"Card Name"];
                [parameters setObject: card.driver
                               forKey: @"Driver"];

                [properties addEntriesFromDictionary: card.properties];
        }

        if ([[item className] isEqualToString: @"PASinkInfo"]) {
                PASinkInfo *sink = (PASinkInfo *) item;

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

                [properties addEntriesFromDictionary: sink.properties];
        }

        if ([[item className] isEqualToString: @"PASinkInputInfo"]) {
                PASinkInputInfo *input = (PASinkInputInfo *) item;

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

                [properties addEntriesFromDictionary: input.properties];
        }

        if ([[item className] isEqualToString: @"PASourceInfo"]) {
                PASourceInfo *source = (PASourceInfo *) item;

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

                [properties addEntriesFromDictionary: source.properties];
        }

        if ([[item className] isEqualToString: @"PASourceOutputInfo"]) {
                PASourceOutputInfo *output = (PASourceOutputInfo *) output;

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

                [properties addEntriesFromDictionary: output.properties];
        }

        if ([[item className] isEqualToString: @"PAModuleInfo"]) {
                PAModuleInfo *module = (PAModuleInfo *) item;

                [parameters setObject: module.name
                               forKey: @"Module Name"];
                [parameters setObject: module.argument ?: @""
                               forKey: @"Arguments"];
                [parameters setObject: [NSNumber numberWithInt: module.useCount]
                               forKey: @"Use count"];

                [properties addEntriesFromDictionary: module.properties];
        }

        if ([[item className] isEqualToString: @"PASampleInfo"]) {
                PASampleInfo *sample = (PASampleInfo *) item;

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
        }

        if ([[item className] isEqualToString: @"PAClientInfo"]) {
                PAClientInfo *client = (PAClientInfo *) item;

                [parameters setObject: client.name
                               forKey: @"Module Name"];
                [parameters setObject: client.driver
                               forKey: @"Driver"];

                [properties addEntriesFromDictionary: client.properties];
        }

        [parameterTableView reloadData];
        [propertyTableView reloadData];
}

- (void) contentChanged: (PAElementInfo *) item
{
        [parameters removeAllObjects];
        [properties removeAllObjects];

        NSInteger selectedRow = [selectionTableView selectedRow];

        if (selectedRow >= 0) {
                PAElementInfo *active = (PAElementInfo *) [self childForRow: selectedRow];
                [selectionTableView deselectAll: nil];
                [selectionTableView reloadData];

                NSInteger row = [self rowForChild: active];
                if (row > 0) {
                        [selectionTableView selectRowIndexes: [NSIndexSet indexSetWithIndex: row]
                                        byExtendingSelection: NO];
                        [self setActiveItem: active];
                }
        }

        [self repaintViews];
}

#pragma mark ### NSTableViewSource protocol ###

- (void) tableView: (NSTableView *) aTableView
    setObjectValue: obj
    forTableColumn: (NSTableColumn *)col
               row: (NSInteger)rowIndex
{
}

- (id)tableView: (NSTableView *) tableView
objectValueForTableColumn: (NSTableColumn *) col
            row: (NSInteger) rowIndex
{
        NSDictionary *item = nil;

        if (tableView == selectionTableView) {
                NSObject *child = [self childForRow: rowIndex];

                if ([[col identifier] isEqualToString: @"image"]) {
                        return nil;
                }


                NSString *label = @"";

                if ([[child className] isEqualToString: @"PACardInfo"]   ||
                    [[child className] isEqualToString: @"PAClientInfo"] ||
                    [[child className] isEqualToString: @"PAModuleInfo"] ||
                    [[child className] isEqualToString: @"PASampleInfo"] ||
                    [[child className] isEqualToString: @"PAServerInfo"]) {
                        PAElementInfo *info = (PAElementInfo *) child;
                        label = info.name;
                }

                if ([[child className] isEqualToString: @"PASinkInfo"]) {
                        PASinkInfo *info = (PASinkInfo *) child;
                        NSDictionary *p = info.properties;
                        label = [NSString stringWithFormat: @"%@: %@",
                                                [p objectForKey: @"device.string"],
                                                info.name];
                }

                if ([[child className] isEqualToString: @"PASourceInfo"]) {
                        PASourceInfo *info = (PASourceInfo *) child;
                        NSDictionary *p = info.properties;
                        label = [NSString stringWithFormat: @"%@: %@",
                                 [p objectForKey: @"device.string"],
                                 info.name];
                }

                if ([[child className] isEqualToString: @"PASinkInputInfo"]) {
                        PASinkInputInfo *info = (PASinkInputInfo *) child;
                        NSDictionary *p = info.properties;
                        label = [NSString stringWithFormat: @"%@: %@",
                                 [p objectForKey: @"application.name"],
                                 info.name];
                }

                if ([[child className] isEqualToString: @"PASourceOutputInfo"]) {
                        PASourceOutputInfo *info = (PASourceOutputInfo *) child;
                        NSDictionary *p = info.properties;
                        label = [NSString stringWithFormat: @"%@: %@",
                                 [p objectForKey: @"application.name"],
                                 info.name];
                }

                if ([label isEqualToString: @""]) {
                        NSDictionary *d = (NSDictionary *) child;
                        label = [d objectForKey: @"label"];
                }

                if ([self tableView: tableView
                        isHeaderRow: rowIndex])
                        return label;

                return [NSString stringWithFormat: @"     %@", label];
        }

        if (tableView == parameterTableView)
                item = parameters;
        else if (tableView == propertyTableView)
                item = properties;

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
        if (tableView == selectionTableView) {
                UInt32 count = 0;

                for (NSDictionary *d in outlineToplevel) {
                        NSArray *children = [d objectForKey: @"children"];
                        count += [children count] + 1;
                }

                return count;
        }

        if (tableView == parameterTableView)
                return [parameters count];

        else if (tableView == propertyTableView)
                return [properties count];

        return 0;
}

#pragma mark ### NSTableViewDelegate protocol ###

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
                        [self setActiveItem: [children objectAtIndex: selected - index]];
                        [self repaintViews];
                        return;
                }

                index += [children count];
        }
}

- (void) tableView: (NSTableView *) aTableView
   willDisplayCell: (id) aCell
    forTableColumn: (NSTableColumn *) tableColumn
               row: (NSInteger) rowIndex
{
        if (aTableView != selectionTableView)
                return;

        if ([[tableColumn identifier] isEqualToString: @"image"])
                return;

        if ([self tableView: aTableView
                isHeaderRow: rowIndex]) {
                [aCell setFont: [NSFont boldSystemFontOfSize: 12.0]];
                [aCell setTextColor: [NSColor darkGrayColor]];
        } else {
                [aCell setFont: [NSFont systemFontOfSize: 11.0]];
                [aCell setTextColor: [NSColor blackColor]];
        }
}

@end
