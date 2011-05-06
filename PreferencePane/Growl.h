/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>

@protocol GrowlDelegate
@required
- (void) setPreferences: (id) value
                 forKey: (NSString *) key;
@end


@interface Growl : NSObject <NSTableViewDelegate> {
        IBOutlet NSTableView *tableView;
        IBOutlet NSButton *enabledButton;
        IBOutlet NSTabView *tabView;

        NSDistributedNotificationCenter *notificationCenter;
        NSArray *notifications;
        UInt64 notificationFlags;
        BOOL growlEnabled;

        NSObject <GrowlDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <GrowlDelegate> *delegate;

- (void) preferencesChanged: (NSDictionary *) preferences;

/* NSTableViewDelegate */
- (void)tableView:(NSTableView *)aTableView
   setObjectValue:obj
   forTableColumn:(NSTableColumn *)col
              row:(int)rowIndex;
- (id)tableView:(NSTableView *)tableView
objectValueForTableColumn:(NSTableColumn *)col
            row:(int)rowIndex;
- (int) numberOfRowsInTableView:(NSTableView *)tableView;

/* GUI */
- (IBAction) setEnabled: (id) sender;

@end
