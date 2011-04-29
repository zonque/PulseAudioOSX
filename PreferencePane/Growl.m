/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Growl.h"

@implementation Growl

@synthesize delegate;

- (void) preferencesChanged: (NSDictionary *) preferences
{
	NSNumber *number = [preferences objectForKey: @"growlNotificationFlags"];
	notificationFlags = [number unsignedLongLongValue];
	BOOL growlReady = !!(notificationFlags & (1ULL << 63));
	
	growlEnabled = !!(notificationFlags & (1ULL << 62));
	
	[enabledButton setEnabled: growlEnabled];
	[tableView setEnabled: growlEnabled];

	/*
	if (growlReady) {
		[tabView selectTabViewItemWithIdentifier: @"active"];
		[tableView reloadData];
	} else
		[tabView selectTabViewItemWithIdentifier: @"inactive"];	
	 */
}

- (void) sendFlags
{
	UInt64 flags = notificationFlags;

	if (growlEnabled)
		flags |= 1ULL << 62;

	[delegate setPreferences: [NSNumber numberWithUnsignedLongLong: flags]
			  forKey: @"growlNotificationFlags"];
}

- (void) awakeFromNib
{
	notificationCenter = [NSDistributedNotificationCenter defaultCenter];

	notifications = [NSArray arrayWithObjects: 
			 @"Server appeared",
			 @"Server disappeared",
			 @"Source appeared",
			 @"Source disappeared",
			 @"Sink appeared",
			 @"Sink disappeared",
			 @"Client connected",
			 @"Client disconnected",
			 nil];
}

#pragma mark ### NSTableView ###

- (void)tableView:(NSTableView *)aTableView
   setObjectValue:obj
   forTableColumn:(NSTableColumn *)col
	      row:(int)rowIndex
{
	if ([obj intValue])
		notificationFlags |= (1ULL << rowIndex);
	else
		notificationFlags &= ~(1ULL << rowIndex);

	[self sendFlags];
}

- (id)tableView:(NSTableView *)tableView
objectValueForTableColumn:(NSTableColumn *)col
	    row:(int)rowIndex
{
	if ([[col identifier] isEqualToString: @"name"])
		return [notifications objectAtIndex: rowIndex];

	if ([[col identifier] isEqualToString: @"check"])
		return [NSNumber numberWithBool: notificationFlags & (1ULL << rowIndex)];

	return nil;
}

- (int) numberOfRowsInTableView:(NSTableView *)tableView
{
	return [notifications count];
}

#pragma mark ### GUI ###
- (IBAction) setEnabled: (id) sender
{
	growlEnabled = ([sender state] == NSOnState);
	[tableView setEnabled: growlEnabled];
	[self sendFlags];
}

@end
