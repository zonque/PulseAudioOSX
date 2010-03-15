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
	NSInteger i;

	[channelsInPopup removeAllItems];
	[channelsOutPopup removeAllItems];
	[blockSizePopup removeAllItems];
	[clockingSourcePopup removeAllItems];
	[clockingSourcePopup addItemWithTitle: @"kernel driver"];
	[clockingSourcePopup addItemWithTitle: @"PulseAudio"];

	for (i = 0; i < 5; i++) {
		NSString *s = [[NSNumber numberWithInt: 1UL << (i + 1)] stringValue];
		[channelsInPopup addItemWithTitle: s];
		[channelsOutPopup addItemWithTitle: s];
	}

	for (i = 0; i < 6; i++) {
		NSString *s = [[NSNumber numberWithInt: 1UL << (i + 6)] stringValue];
		[blockSizePopup addItemWithTitle: s];
	}
	
	notificationCenter = [NSDistributedNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
						   selector: @selector(updateDeviceList:)
							   name: @"updateDeviceList"
							 object: REMOTE_OBJECT];
	[self requestDeviceList];
	[self selectDevice: nil];
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

- (IBAction) cancelAddDevice: (id) sender
{
	[NSApp endSheet: addDevicePanel];
}

- (IBAction) doAddDevice: (id) sender
{
	[NSApp endSheet: addDevicePanel];

	NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity: 0];

	NSInteger channelsIn  = 1UL << ([channelsInPopup indexOfSelectedItem] + 1);
	NSInteger channelsOut = 1UL << ([channelsOutPopup indexOfSelectedItem] + 1);
	NSInteger blockSize  = 1UL << ([blockSizePopup indexOfSelectedItem] + 5);

	[dict setValue: [deviceNameField stringValue]
			forKey: @"name"];
	[dict setValue: [NSNumber numberWithInt: channelsIn]
			forKey: @"channelsIn"];
	[dict setValue: [NSNumber numberWithInt: channelsOut]
			forKey: @"channelsOut"];
	[dict setValue: [NSNumber numberWithInt: blockSize]
			forKey: @"blockSize"];

	[notificationCenter postNotificationName: @"addDevice"
									  object: LOCAL_OBJECT
									userInfo: dict
						  deliverImmediately: YES];	
	
	[dict release];
	
	[self requestDeviceList];
}

- (void)didEndSheet: (NSWindow *) sheet
		 returnCode: (NSInteger) returnCode
		contextInfo: (void *)contextInfo
{
    [sheet orderOut:self];
}

- (IBAction) addDevice: (id) sender
{
	[deviceNameField setStringValue: @""];

	NSWindow *window = [[self mainView] window];

	[NSApp beginSheet: addDevicePanel
	   modalForWindow: window
		modalDelegate: self
	   didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
		  contextInfo: nil];
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

- (IBAction) selectDevice: (id) sender
{
	NSInteger selected = [deviceTableView selectedRow];
	
	if (selected < 0) {
		[channelsInLabel setHidden: YES];
		[channelsOutLabel setHidden: YES];
		[clockingSourceLabel setHidden: YES];
		return;
	}

	[channelsInLabel setHidden: NO];
	[channelsOutLabel setHidden: NO];
	[clockingSourceLabel setHidden: NO];
	
	NSDictionary *dict = [deviceList objectAtIndex: selected];
	NSInteger channelsIn = [[dict valueForKey: @"channelsIn"] intValue];
	NSInteger channelsOut = [[dict valueForKey: @"channelsOut"] intValue];
	
	[channelsInLabel setIntValue: channelsIn];
	[channelsOutLabel setIntValue: channelsOut];
}

@end
