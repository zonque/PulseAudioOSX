/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PreferencePanes/PreferencePanes.h>


@interface PAPreferencePane : NSPreferencePane 
{
	IBOutlet NSTableView *deviceTableView;

	NSDistributedNotificationCenter *notificationCenter;
	NSArray *deviceList;
}

- (void) mainViewDidLoad;

- (IBAction) addDevice: (id) sender;
- (IBAction) removeDevice: (id) sender;

@end
