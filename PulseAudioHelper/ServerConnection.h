/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <PulseAudio/PulseAudio.h>
#import "Preferences.h"

@class ServerConnection;

@protocol ServerConnectionDelegate
- (void) ServerConnectionEstablished: (ServerConnection *) connection;
- (void) ServerConnectionFailed: (ServerConnection *) connection;
- (void) ServerConnectionEnded: (ServerConnection *) connection;
@end

@interface ServerConnection : NSObject <PAServerConnectionDelegate>
{
    PAServerConnection *connection;
    Preferences *preferences;
    BOOL networkEnabled;
    PAModuleInfo *networkModule;
    NSObject <ServerConnectionDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <ServerConnectionDelegate> *delegate;

- (void) connect;

/* PAServerConnectionDelegate */

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection;
- (void) PAServerConnectionFailed: (PAServerConnection *) connection;
- (void) PAServerConnectionEnded: (PAServerConnection *) connection;

@end
