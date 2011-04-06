/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "BonjourListener.h"
#import <PreferencePanes/PreferencePanes.h>
#import <Quartz/Quartz.h>

@interface PAPreferencePane : NSPreferencePane 
{
	IBOutlet NSTableView		*clientTableView;
	IBOutlet IKImageView		*imageView;
	IBOutlet NSTabView		*clientDetailsBox;
	IBOutlet NSTextField		*clientNameLabel;
	IBOutlet NSTextField		*audioDeviceLabel;
	IBOutlet NSTextField		*PIDLabel;
	IBOutlet NSTextField		*IOBufferSizeLabel;
	IBOutlet NSPopUpButton		*serverSelectButton;
	IBOutlet NSTextField		*userNameField;
	IBOutlet NSSecureTextField	*passwordField;
	IBOutlet NSTextField		*connectionStatusTextField;
	IBOutlet NSButton		*persistenCheckButton;

	NSDistributedNotificationCenter *notificationCenter;
	NSMutableArray *clientList;
	NSTimer *timer;
	BonjourListener *listener;
}

- (IBAction) selectClient: (id) sender;
- (IBAction) connectClient: (id) sender;

@end
