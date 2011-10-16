/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>

@protocol LocalServerDelegate
@required
- (void) setPreferences: (id) value
                 forKey: (NSString *) key;
@end

@interface LocalServer : NSObject {
    IBOutlet NSButton *enabledButton;
    IBOutlet NSButton *networkEnabledButton;
    
    NSDistributedNotificationCenter *notificationCenter;
    NSObject <LocalServerDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <LocalServerDelegate> *delegate;

- (void) preferencesChanged: (NSDictionary *) preferences;

- (IBAction) setEnabled: (id) sender;
- (IBAction) setNetworkEnabled: (id) sender;

@end
