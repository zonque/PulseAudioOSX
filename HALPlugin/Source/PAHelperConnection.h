/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import "PADevice.h"

@class PAHelperConnection;

@protocol PAHelperConnectionDelegate
@required
- (void) PAHelperConnectionDied: (PAHelperConnection *) connection;
- (void) PAHelperConnection: (PAHelperConnection *) connection
		  setConfig: (NSDictionary *) config
	  forDeviceWithName: (NSString *) name;
@end

@interface PAHelperConnection : NSObject <NSConnectionDelegate>
{
	id serverProxy;
	NSConnection *service;
	NSObject <PAHelperConnectionDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <PAHelperConnectionDelegate> *delegate;

- (BOOL) connect;
- (BOOL) isConnected;

- (void) deviceStarted: (PADevice *) device;
- (void) deviceStopped: (PADevice *) device;

// NSConnectionDelegate
//- (BOOL)connection: (NSConnection *) conn
//     handleRequest: (NSDistantObjectRequest *) doReq;

@end
