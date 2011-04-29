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
- (NSDictionary *) getPreferences;
- (void) setPreferences: (id) object
		 forKey: (NSString *) key;
@end

@protocol PAHelperConnectionDelegate
@required
- (void) PAHelperConnectionDied: (PAHelperConnection *) connection;

@optional
// for preference pane
- (void) PAHelperConnection: (PAHelperConnection *) connection
	audioClientsChanged: (NSArray *) array;

- (void) PAHelperConnection: (PAHelperConnection *) connection
	 preferencesChanged: (NSDictionary *) preferences;

// for audio devices
- (void) PAHelperConnection: (PAHelperConnection *) connection
		  setConfig: (NSDictionary *) config
	  forDeviceWithName: (NSString *) name;

@end

@interface PAHelperConnection : NSObject <NSConnectionDelegate>
{
	id serverProxy;
	NSObject <PAHelperConnectionDelegate> *delegate;
	NSString *name;
}

@property (nonatomic, assign) id <PAHelperConnection> serverProxy;
@property (nonatomic, assign) NSObject <PAHelperConnectionDelegate> *delegate;
@property (nonatomic, readonly) NSString *name;

- (BOOL) connect;
- (BOOL) isConnected;

@end
