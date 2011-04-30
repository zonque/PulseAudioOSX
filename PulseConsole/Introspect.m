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

@synthesize serverConnection;

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
	[outlineView setEnabled: NO];
	[parameterTableView setEnabled: NO];
	[propertyTableView setEnabled: NO];
}

#pragma mark ### NSOutlineViewDataSource protocol ###

- (NSInteger) outlineView: (NSOutlineView *) outlineView numberOfChildrenOfItem: (id) item
{
	if (item == nil) {
		NSEnumerator *enumerator = [serverConnection.outlineToplevel objectEnumerator];
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
		return [serverConnection.outlineToplevel count] > 0;
	
	NSArray *d = [item valueForKey: @"children"];
	return d ? [d count] > 0 : NO;
}

- (id) outlineView: (NSOutlineView *) outlineView child: (NSInteger)index
	    ofItem: (id)item
{
	if (item == nil)
		return [serverConnection.outlineToplevel objectAtIndex: index];
	
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

- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView
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

@end
