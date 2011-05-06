/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>


@interface ConnectionClient : NSObject
{
        NSConnection *connection;
        NSMutableArray *audioClients;
        NSString *connectionName;
}

@property (nonatomic, readonly) NSConnection *connection;
@property (nonatomic, readonly) NSArray *audioClients;

- (id) initWithConnection: (NSConnection *) c;
- (void) registerClientWithName: (NSString *) name;
- (void) announceDevice: (NSDictionary *) device;
- (void) signOffDevice: (NSString *) signedOffName;
- (void) audioClientsChanged : (NSArray *) clients;
- (void) preferencesChanged : (NSDictionary *) preferences;
- (void) setConfig: (NSDictionary *) config
 forDeviceWithUUID: (NSString *) uuid;

@end
