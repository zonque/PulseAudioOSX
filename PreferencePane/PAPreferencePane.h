/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PreferencePanes/PreferencePanes.h>

@interface PAPreferencePane : NSPreferencePane 
{
	IBOutlet NSTableView	*deviceTableView;
	IBOutlet NSPopUpButton	*serverNamePopup;
	IBOutlet NSPopUpButton	*channelsInPopup;
	IBOutlet NSPopUpButton	*channelsOutPopup;
	IBOutlet NSPopUpButton	*blockSizePopup;
	IBOutlet NSPopUpButton	*audioContentTypePopup;
	IBOutlet NSPopUpButton	*streamCreationTypePopup;
	IBOutlet NSTextField	*deviceNameField;
	IBOutlet NSPanel	*addDevicePanel;
	IBOutlet NSTextField	*serverNameLabel;
	IBOutlet NSTextField	*channelsInLabel;
	IBOutlet NSTextField	*channelsOutLabel;
	IBOutlet NSTextField	*audioContentTypeLabel;
	IBOutlet NSTextField	*streamCreationTypeLabel;

	NSDistributedNotificationCenter *notificationCenter;
	NSArray *deviceList;
	
	NSMutableArray *audioContentTypeStrings;
	NSMutableArray *streamCreationTypeStrings;
}

- (IBAction) addDevice: (id) sender;
- (IBAction) removeDevice: (id) sender;
- (IBAction) cancelAddDevice: (id) sender;
- (IBAction) doAddDevice: (id) sender;
- (IBAction) selectDevice: (id) sender;

@end
