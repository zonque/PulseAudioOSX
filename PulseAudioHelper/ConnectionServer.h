/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <PulseAudio/ULINetSocket.h>

#import "ServerConnection.h"
#import "ConnectionClient.h"
#import "Preferences.h"

@interface ConnectionServer : NSObject <
        ULINetSocketDelegate,
        PAHelperConnectionDelegate,
        ConnectionClientDelegate,
        ServerConnectionDelegate
    >
{
    NSMutableArray *clientArray;
    NSConnection *currentConnection;
    NSConnection *serviceConnection;
    
	ULINetSocket *serviceSocket;
	
    Preferences *preferences;
}

@property (nonatomic, assign) NSConnection *currentConnection;
@property (nonatomic, retain) Preferences *preferences;

- (void) start;

@end
