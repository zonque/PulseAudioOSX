/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ServerConnection.h"

static NSString *tcpModuleName = @"module-native-protocol-tcp";

@implementation ServerConnection

#pragma mark ### NSNotificationCenter ###

- (void) preferencesChanged: (NSNotification *) notification
{
        networkEnabled = [[preferences valueForKey: @"localServerNetworkEnabled"] boolValue];

        if (!networkEnabled && networkModule)
                if ([networkModule unload]);
                        networkModule = nil;

        if (networkEnabled && !networkModule)
                [connection loadModuleWithName: tcpModuleName
                                     arguments: @"auth-anonymous=1"];
}

- (id) initWithPreferences: (Preferences *) p
{
        [super init];

        preferences = p;

        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(preferencesChanged:)
                                                     name: @"preferencesChanged"
                                                   object: preferences];

        connection = [[PAServerConnection alloc] init];
        connection.delegate = self;
        [connection connectToHost: nil
                             port: -1];

        return self;
}


/* PAServerConnectionDelegate */

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection
{
        NSLog(@"%s", __func__);
}

- (void) PAServerConnectionFailed: (PAServerConnection *) connection
{
        NSLog(@"%s", __func__);
        networkModule = nil;
}

- (void) PAServerConnectionEnded: (PAServerConnection *) connection
{
        NSLog(@"%s", __func__);
        networkModule = nil;
}

- (void) PAServerConnection: (PAServerConnection *) connection
            moduleInfoAdded: (PAModuleInfo *) module
{
        if ([module.name isEqualToString: tcpModuleName])
                networkModule = module;
}

- (void) PAServerConnection: (PAServerConnection *) connection
          moduleInfoRemoved: (PAModuleInfo *) module;
{
        if ([module.name isEqualToString: tcpModuleName])
                networkModule = nil;
}

@end
