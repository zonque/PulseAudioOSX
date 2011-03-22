/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
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

	audioContentTypeStrings = [[NSMutableArray arrayWithCapacity: 0] retain];
	[audioContentTypeStrings addObject: @"Mixdown"];
	[audioContentTypeStrings addObject: @"Individual clients"];

	streamCreationTypeStrings = [[NSMutableArray arrayWithCapacity: 0] retain];
	[streamCreationTypeStrings addObject: @"Permanent"];
	[streamCreationTypeStrings addObject: @"On demand"];

	[channelsInPopup removeAllItems];
	[channelsOutPopup removeAllItems];

	for (i = 0; i < 5; i++) {
		NSString *s = [[NSNumber numberWithInt: 1UL << (i + 1)] stringValue];
		[channelsInPopup addItemWithTitle: s];
		[channelsOutPopup addItemWithTitle: s];
	}

	[blockSizePopup removeAllItems];

	for (i = 0; i < 6; i++) {
		NSString *s = [[NSNumber numberWithInt: 1UL << (i + 6)] stringValue];
		[blockSizePopup addItemWithTitle: s];
	}
	
	for (NSString *str in audioContentTypeStrings)
		[audioContentTypePopup addItemWithTitle: str];

	[audioContentTypePopup removeAllItems];
	for (NSString *str in audioContentTypeStrings)
		[audioContentTypePopup addItemWithTitle: str];

	[streamCreationTypePopup removeAllItems];
	for (NSString *str in streamCreationTypeStrings)
		[streamCreationTypePopup addItemWithTitle: str];

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
	[dict setValue: [serverNamePopup stringValue]
		forKey: @"server"];
	[dict setValue: [NSNumber numberWithInt: channelsIn]
		forKey: @"channelsIn"];
	[dict setValue: [NSNumber numberWithInt: channelsOut]
		forKey: @"channelsOut"];
	[dict setValue: [NSNumber numberWithInt: blockSize]
		forKey: @"blockSize"];
	[dict setValue: [NSNumber numberWithInt: [audioContentTypePopup indexOfSelectedItem]]
		forKey: @"audioContentType"];
	[dict setValue: [NSNumber numberWithInt: [streamCreationTypePopup indexOfSelectedItem]]
		forKey: @"streamCreationType"];

	[notificationCenter postNotificationName: @"addDevice"
					  object: LOCAL_OBJECT
					userInfo: dict
			      deliverImmediately: YES];	

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
	BOOL hidden = selected < 0;

	[channelsInLabel setHidden: hidden];
	[channelsOutLabel setHidden: hidden];
	[audioContentTypeLabel setHidden: hidden];
	[streamCreationTypeLabel setHidden: hidden];

	if (hidden)
		return;
	
	NSDictionary *dict = [deviceList objectAtIndex: selected];
	NSString *server = [dict valueForKey: @"server"];
	NSInteger channelsIn = [[dict valueForKey: @"channelsIn"] intValue];
	NSInteger channelsOut = [[dict valueForKey: @"channelsOut"] intValue];
	NSInteger audioContentType = [[dict valueForKey: @"audioContentType"] intValue];
	NSInteger streamCreationType = [[dict valueForKey: @"streamCreationType"] intValue];
	
	[serverNameLabel setStringValue: server];
	[channelsInLabel setIntValue: channelsIn];
	[channelsOutLabel setIntValue: channelsOut];
	[audioContentTypeLabel setStringValue: [audioContentTypeStrings objectAtIndex: audioContentType]];
	[streamCreationTypeLabel setStringValue: [streamCreationTypeStrings objectAtIndex: streamCreationType]];
}

@end
