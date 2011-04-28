/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

#define PAOSX_HelperName			@"org.pulseaudio.PulseAudioHelper"
#define PAOSX_HelperMsgServiceStarted		@"org.pulseaudio.PulseAudioHelper.serviceStarted"

@class PAHelperConnection;

@protocol PAHelperConnection
@optional
- (void) registerClientWithName: (NSString *) s;
- (void) announceDevice: (NSDictionary *) device;
- (void) signOffDevice: (NSString *) name;
- (void) audioClientsChanged: (NSArray *) array;
@end

@protocol PAHelperConnectionDelegate
@required
- (void) PAHelperConnectionDied: (PAHelperConnection *) connection;
- (void) PAHelperConnection: (PAHelperConnection *) connection
		  setConfig: (NSDictionary *) config
	  forDeviceWithName: (NSString *) name;
@end

@interface PAHelperConnection : NSObject <NSConnectionDelegate>
{
	NSConnection *service;
	id serverProxy;
	id <PAHelperConnectionDelegate> delegate;
}

@property (nonatomic, assign) id <PAHelperConnection> serverProxy;
@property (nonatomic, assign) id <PAHelperConnectionDelegate> delegate;

- (BOOL) connect;
- (BOOL) isConnected;

@end
