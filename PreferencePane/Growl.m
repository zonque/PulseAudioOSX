/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "Growl.h"

#define LOCAL_OBJECT @"PulseAudioPreferencePane"
#define REMOTE_OBJECT @"PulseAudioHelper"

@implementation Growl

- (void) updateGrowlFlags: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSNumber *number = [userInfo objectForKey: @"notificationFlags"];
	notificationFlags = [number unsignedLongLongValue];
	BOOL enabled = !!(notificationFlags & (1ULL << 63));
	
	if (enabled) {
		[tabView selectTabViewItemWithIdentifier: @"active"];
		[tableView reloadData];
	} else
		[tabView selectTabViewItemWithIdentifier: @"inactive"];
}

- (void) sendFlags
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	BOOL enabled = [tableView isEnabled];
	
	NSLog(@"%s()\n", __func__);

	[userInfo setObject: [NSNumber numberWithUnsignedLongLong: enabled ? notificationFlags : 0]
		     forKey: @"notificationFlags"];
	
	[notificationCenter postNotificationName: @"updateGrowlFlags"
					  object: LOCAL_OBJECT
					userInfo: userInfo
			      deliverImmediately: YES];
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
			 nil];
		
	[notificationCenter addObserver: self
			       selector: @selector(updateGrowlFlags:)
				   name: @"updateGrowlFlags"
				 object: REMOTE_OBJECT];

	[notificationCenter postNotificationName: @"queryGrowlFlags"
					  object: LOCAL_OBJECT
					userInfo: nil
			      deliverImmediately: YES];
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
	[tableView setEnabled: [sender state]];
	[self sendFlags];
}

@end
