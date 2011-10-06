/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import "ULINetSocket.h"

#define PAOSX_HelperName                @"org.pulseaudio.PulseAudioHelper"
#define PAOSX_HelperMsgServiceStarted   @"org.pulseaudio.PulseAudioHelper.serviceStarted"

#define PAOSX_HelperSocket              @"/tmp/PAOSX_HelperSocket"
#define PAOSX_HelperMagic 0xaffedead


#define PAOSX_MessageNameKey			@"MessageName"
#define PAOSX_MessageDictionaryKey		@"MessageDictionaryKey"
#define PAOSX_MessageRegisterClient		@"MessageRegisterClient"
#define PAOSX_MessageAudioClientStarted		@"MessageAudioClientStarted"
#define PAOSX_MessageAudioClientStopped		@"MessageAudioClientStopped"
#define PAOSX_MessageAudioClientsUpdate		@"MessageAudioClientsUpdate"
#define PAOSX_MessageRequestPreferences		@"MessageRequestPreferences"
#define PAOSX_MessageSetPreferences		@"MessageSetPreferences"
#define PAOSX_MessageSetAudioDevices		@"MessageSetAudioDevices"
#define PAOSX_MessageSetAudioClientConfig	@"MessageSetAudioClientConfig"

typedef struct PAHelperProtocolHeader {
	UInt32 magic;
	UInt32 length;
} PAHelperProtocolHeader;

@class PAHelperConnection;

@protocol PAHelperConnectionDelegate
@optional
- (void) PAHelperConnectionEstablished: (PAHelperConnection *) connection;
- (void) PAHelperConnectionDied: (PAHelperConnection *) connection;
- (void) PAHelperConnection: (PAHelperConnection *) connection
            receivedMessage: (NSString *) name
                       dict: (NSDictionary *) msg;
@end

@interface PAHelperConnection : NSObject
{
    NSObject <PAHelperConnectionDelegate> *delegate;
    
@private
	ULINetSocket *socket;
	NSMutableData *inboundData;
	BOOL retry;
	NSTimer *retryTimer;
}

@property (nonatomic, assign) NSObject <PAHelperConnectionDelegate> *delegate;
@property (nonatomic, readonly) ULINetSocket *socket;

- (id) initWithSocket: (ULINetSocket *) socket;
- (void) scheduleOnCurrentRunLoop;

- (BOOL) connectWithRetry: (BOOL) retry;
- (BOOL) isConnected;
- (void) sendMessage: (NSString *) name
                dict: (NSDictionary *) msg;

@end
