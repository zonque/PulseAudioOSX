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

@class ConnectionClient;
@class ConnectionServer;

@protocol ConnectionClientDelegate
- (void) connectionClientDied: (ConnectionClient *) client;
- (void) connectionClientChangedAudioClients: (ConnectionClient *) client;
- (void) connectionClient: (ConnectionClient *) client
       changedPreferences: (NSDictionary *) changed;
- (void) connectionClient: (ConnectionClient *) client
     setAudioClientConfig: (NSDictionary *) config
		  forUUID: (NSString *) uuid;
- (NSArray *) allAudioClients;
@end

@interface ConnectionClient : NSObject <PAHelperConnectionDelegate>
{
	ConnectionServer *server;
	PAHelperConnection *connection;
        NSMutableArray *audioClients;
        NSLock *lock;
	
	NSObject <ConnectionClientDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <ConnectionClientDelegate> *delegate;
@property (nonatomic, readonly) ULINetSocket *socket;
@property (nonatomic, readonly) NSArray *audioClients;

- (id) initWithSocket: (ULINetSocket *) socket
	    forServer: (ConnectionServer *) server;
- (void) sendAudioClientsChanged : (NSArray *) clients;
- (void) sendPreferencesChanged : (NSDictionary *) preferences;
- (void) setConfig: (NSDictionary *) config
 forDeviceWithUUID: (NSString *) uuid;
- (void) sendSetAudioClientConfig: (NSDictionary *) config
			  forUUID: (NSString *) uuid;

@end
