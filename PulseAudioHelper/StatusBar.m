/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#define PATHHACK "/Users/daniel/src/pa/PulseAudioOSX/PulseAudioHelper/"

#import "StatusBar.h"
#import "Pathes.h"

@implementation StatusBar

@synthesize preferences;

#pragma mark ### NSMenuItem callback selectors ###

- (void) openPreferences: (id) sender
{
    NSString *file = PAOSX_ConfigFile;
    [[NSWorkspace sharedWorkspace] openFile: file];
}

- (void) openPulseConsole: (id) sender
{
    NSString *file = PAOSX_PulseConsoleBundle;
    [[NSWorkspace sharedWorkspace] openFile: file];
}

- (NSMenu *) createMenu
{
    NSZone *zone = [NSMenu menuZone];
    NSMenu *m = [[NSMenu allocWithZone: zone] init];
    
    NSMenuItem *item;
    
    item = [m addItemWithTitle: @"xxx"
                        action: @selector(startGrowl:)
                 keyEquivalent: @""];
    [item setTarget: self];
    [item setTag: 1];
    
    [m addItem:[NSMenuItem separatorItem]];
    
    item = [m addItemWithTitle: @"Open Preferences ..."
                        action: @selector(openPreferences:)
                 keyEquivalent: @""];
    [item setTarget: self];
    
    item = [m addItemWithTitle: @"Open PulseConsole ..."
                        action: @selector(openPulseConsole:)
                 keyEquivalent: @""];
    [item setTarget: self];
    
    return m;
}

- (BOOL) validateMenuItem: (NSMenuItem *) item
{
    return YES;
}

- (void) showMenu
{
    statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];
    
    NSMenu *m = [self createMenu];
    [statusItem setMenu: m]; // retains m
    [statusItem setToolTip: @"PulseAudio"];
    [statusItem setHighlightMode: YES];
    [statusItem setImage: icon];
}

- (void) hideMenu
{
    if (statusItem) {
        [[NSStatusBar systemStatusBar] removeStatusItem: statusItem];
        [statusItem release];
        statusItem = nil;
    }
}

#pragma mark ### NSNotificationCenter ###

- (void) preferencesChanged: (NSNotification *) notification
{
    BOOL enabled = [[preferences valueForKey: @"statusBarEnabled"] boolValue];
    
    if (enabled && !statusItem)
        [self showMenu];
    
    if (!enabled && statusItem)
        [self hideMenu];
}

#pragma mark ### NSApplicationDelegate ###

- (void) applicationDidFinishLaunching: (NSNotification *) aNotification
{
    icon = [NSImage imageNamed: @"statusBar.png"];
    
    if ([[preferences valueForKey: @"statusBarEnabled"] boolValue])
        [self showMenu];
    
    [[NSNotificationCenter defaultCenter] addObserver: self
                                             selector: @selector(preferencesChanged:)
                                                 name: @"preferencesChanged"
                                               object: preferences];
}

@end
