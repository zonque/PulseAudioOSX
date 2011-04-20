/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PreferencePane.h"
#import "ObjectNames.h"

@implementation PreferencePane

- (void) didSelect
{
}

- (void) didUnselect
{
}

- (void) mainViewDidLoad
{
	NSDistributedNotificationCenter *notificationCenter = [NSDistributedNotificationCenter defaultCenter];
	
	[notificationCenter postNotificationName: @PAOSX_HelperMsgQueryStatusBarEnabled
					  object: @PAOSX_PreferencePaneName
					userInfo: nil
			      deliverImmediately: YES];
}

#pragma mark ### GUI ###
- (IBAction) setStatusBarEnabled: (id) sender
{
	NSDistributedNotificationCenter *notificationCenter = [NSDistributedNotificationCenter defaultCenter];
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	BOOL enabled = ([sender state] == NSOnState);

	[userInfo setObject: [NSNumber numberWithBool: enabled]
		     forKey: @"enabled"];
	
	[notificationCenter postNotificationName: @PAOSX_HelperMsgSetStatusBarEnabled
					  object: @PAOSX_PreferencePaneName
					userInfo: userInfo
			      deliverImmediately: YES];		
}

- (IBAction) setPulseAudioEnabled: (id) sender
{
	[loginItemController toggleLoginItem: sender];
}

@end
