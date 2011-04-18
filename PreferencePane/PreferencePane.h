/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <PreferencePanes/PreferencePanes.h>
#import "AudioClients.h"

#define LOCAL_OBJECT @"PulseAudioPreferencePane"
#define REMOTE_OBJECT_HELPER @"PulseAudioHelper"
#define REMOTE_OBJECT_HALPLUGIN @"PAHP_Device"

@interface PreferencePane : NSPreferencePane
{
	IBOutlet AudioClients *audioClients;
	IBOutlet NSButton *statusBarEnabledButton;
}

- (IBAction) setStatusBarEnabled: (id) sender;

@end
