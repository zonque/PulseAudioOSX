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
	IBOutlet NSTableView	*deviceTableView;
	IBOutlet NSPopUpButton	*channelsInPopup;
	IBOutlet NSPopUpButton	*channelsOutPopup;
	IBOutlet NSPopUpButton	*clockingSourcePopup;
	IBOutlet NSTextField	*deviceNameField;
	IBOutlet NSPanel		*addDevicePanel;
	IBOutlet NSTextField	*channelsInLabel;
	IBOutlet NSTextField	*channelsOutLabel;
	IBOutlet NSTextField	*clockingSourceLabel;

	NSDistributedNotificationCenter *notificationCenter;
	NSArray *deviceList;
}

- (void) mainViewDidLoad;

- (IBAction) addDevice: (id) sender;
- (IBAction) removeDevice: (id) sender;
- (IBAction) cancelAddDevice: (id) sender;
- (IBAction) doAddDevice: (id) sender;
- (IBAction) selectDevice: (id) sender;

@end
