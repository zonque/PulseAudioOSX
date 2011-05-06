/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import <PreferencePanes/PreferencePanes.h>
#import <PulseAudio/PAServiceDiscovery.h>

@interface AudioClients : NSObject <PAServiceDiscoveryDelegate>
{
        IBOutlet NSTableView                *clientTableView;
        IBOutlet IKImageView                *imageView;
        IBOutlet NSTabView                *clientDetailsBox;
        IBOutlet NSTextField                *clientNameLabel;
        IBOutlet NSTextField                *audioDeviceLabel;
        IBOutlet NSTextField                *PIDLabel;
        IBOutlet NSTextField                *IOBufferSizeLabel;
        IBOutlet NSPopUpButton                *serverSelectButton;
        IBOutlet NSPopUpButton                *sinkSelectButton;
        IBOutlet NSPopUpButton                *sourceSelectButton;
        IBOutlet NSTextField                *userNameField;
        IBOutlet NSSecureTextField        *passwordField;
        IBOutlet NSTextField                *connectionStatusTextField;
        IBOutlet NSButton                *persistenCheckButton;

        NSMutableArray                        *clientList;
        PAServiceDiscovery                *discovery;

        NSMutableArray *knownServers;
        NSMutableArray *knownSinks;
        NSMutableArray *knownSources;
}

/* Helper callbacks */
- (void) audioClientsChanged: (NSArray *) clients;

/* NSTableViewDelegate */
- (void) tableView:(NSTableView *)aTableView
    setObjectValue:obj
    forTableColumn: (NSTableColumn *) col
               row: (int) rowIndex;
- (id) tableView: (NSTableView *) tableView
 objectValueForTableColumn: (NSTableColumn *) col
            row: (int) rowIndex;
- (int) numberOfRowsInTableView: (NSTableView *) tableView;

/* GUI */
- (IBAction) selectClient: (id) sender;
- (IBAction) selectServer: (id) sender;
- (IBAction) connectClient: (id) sender;

@end
