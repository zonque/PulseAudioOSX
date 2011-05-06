/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>
#import <Growl/Growl.h>
#import <PulseAudio/PAServiceDiscovery.h>
#import <PulseAudio/PulseAudio.h>

#import "Preferences.h"

@interface GrowlNotifications : NSObject <
                                GrowlApplicationBridgeDelegate,
                                PAServiceDiscoveryDelegate,
                                PAServerConnectionDelegate
                                >
{
        PAServiceDiscovery *discovery;
        NSData *logoData;
        BOOL growlReady;
        //ServerConnection *serverConnection;
        Preferences *preferences;
}

- (id) initWithPreferences: (Preferences *) p;

/* GrowlApplicationBridgeDelegate */
- (NSString *) applicationNameForGrowl;
- (NSDictionary *) registrationDictionaryForGrowl;
- (NSData *) applicationIconDataForGrowl;
- (void) growlIsReady;

/* PAServiceDiscoveryDelegate */

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
             serverAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
          serverDisappeared: (NSNetService *) service;

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
               sinkAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
            sinkDisappeared: (NSNetService *) service;

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
             sourceAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
          sourceDisappeared: (NSNetService *) service;

/* PAServerConnectionDelegate */
//...

@end
