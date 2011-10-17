/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ServerTask.h"
#import "Pathes.h"

@implementation ServerTask

- (void) start
{
    NSString *binpath = @"/Library/Frameworks/PulseAudio.framework/Contents/MacOS/bin/pulseaudio";
    NSString *modpath = @"/Library/Frameworks/PulseAudio.framework/Contents/MacOS/lib/pulse-0.98/modules/";

    NSArray *args = [NSArray arrayWithObjects:
                     @"-n",
                     @"-L", @"module-bonjour-publish",
                     @"-L", @"module-coreaudio-detect ioproc_frames=128",
                     @"-L", @"module-native-protocol-unix",
                     @"-L", @"module-simple-protocol-tcp",
                     @"-L", @"module-cli-protocol-unix",
                     @"-L", @"module-native-protocol-tcp auth-anonymous=1",
                     @"-p", modpath,
                     @"--exit-idle-time=-1",
                     nil];
    task = [NSTask launchedTaskWithLaunchPath: binpath
                                    arguments: args];
}

- (void) stop
{
    if (task) {
        [task terminate];
        [task waitUntilExit];
        task = nil;
    }
}

- (void) preferencesChanged: (NSNotification *) notification
{
    BOOL enabled = [[preferences valueForKey: @"localServerEnabled"] boolValue];
    
    if (enabled && !task)
        [self start];
    
    if (!enabled && task)
        [self stop];
}

- (void) taskTerminated: (NSNotification *) notification
{
    if (!task || [notification object] != task)
        return;
    
    NSLog(@"Server task ended.");
}

- (id) initWithPreferences: (Preferences *) p
{
    [super init];
    
    preferences = p;

    if ([[preferences valueForKey: @"localServerEnabled"] boolValue])
        [self start];
    
    [[NSNotificationCenter defaultCenter] addObserver: self
                                             selector: @selector(preferencesChanged:)
                                                 name: @"preferencesChanged"
                                               object: preferences];
    
    return self;
}

@end
