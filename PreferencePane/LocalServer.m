/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "LocalServer.h"
#import "PreferencePane.h"

@implementation LocalServer

- (void) setLocalServerEnabled: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	BOOL enabled = [[userInfo objectForKey: @"enabled"] boolValue];
	[enabledButton setState: enabled ? NSOnState : NSOffState];
}

- (void) awakeFromNib
{
	notificationCenter = [NSDistributedNotificationCenter defaultCenter];
	
	
	[notificationCenter addObserver: self
			       selector: @selector(setLocalServerEnabled:)
				   name: @"setLocalServerEnabled"
				 object: REMOTE_OBJECT_HELPER];	
	
	[notificationCenter postNotificationName: @"queryLocalServerEnabled"
					  object: LOCAL_OBJECT
					userInfo: nil
			      deliverImmediately: YES];
	
}

- (IBAction) setEnabled: (id) sender
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	BOOL enabled = ([sender state] == NSOnState);
	
	[userInfo setObject: [NSNumber numberWithBool: enabled]
		     forKey: @"enabled"];
	
	[notificationCenter postNotificationName: @"setLocalServerEnabled"
					  object: LOCAL_OBJECT
					userInfo: userInfo
			      deliverImmediately: YES];
}

@end
