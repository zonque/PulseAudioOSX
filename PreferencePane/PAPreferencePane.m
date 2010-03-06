/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAPreferencePane.h"

#define LOCAL_OBJECT @"PAPreferencePane"
#define REMOTE_OBJECT @"PADaemon"

@implementation PAPreferencePane

- (void) updateDeviceList: (NSNotification *) notification
{
	NSDictionary *dict = [notification userInfo];
	NSArray *arr = [dict objectForKey: @"array"];

	if (deviceList) {
		[deviceList release];
		deviceList = nil;
	}

	deviceList = [NSArray arrayWithArray: arr];
	[deviceTableView reloadData];
}

- (void) requestDeviceList
{
	[notificationCenter postNotificationName: @"sendDeviceList"
									  object: LOCAL_OBJECT
									userInfo: nil
						  deliverImmediately: YES];	
}

- (void) mainViewDidLoad
{
	notificationCenter = [NSDistributedNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
						   selector: @selector(updateDeviceList:)
							   name: @"updateDeviceList"
							 object: REMOTE_OBJECT];
	[self requestDeviceList];
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
	NSDictionary *dict = [deviceList objectAtIndex: rowIndex];
	
	if (!dict)
		return @"";

	return [dict valueForKey: @"name"];
}

- (int) numberOfRowsInTableView:(NSTableView *)tableView
{
	return deviceList ? [deviceList count] : 0;
}

#pragma mark ### IBActions ###

- (IBAction) addDevice: (id) sender
{
	
}

- (IBAction) removeDevice: (id) sender
{
	NSInteger selected = [deviceTableView selectedRow];
	
	if (selected < 0)
		return;
	
	NSNumber *index = [NSNumber numberWithInt: selected];
	NSDictionary *dict = [NSDictionary dictionaryWithObject: index
													 forKey: @"index"];
	
	[notificationCenter postNotificationName: @"removeDevice"
									  object: LOCAL_OBJECT
									userInfo: dict
						  deliverImmediately: YES];

	[index release];
	[dict release];
	[self requestDeviceList];
}


@end
