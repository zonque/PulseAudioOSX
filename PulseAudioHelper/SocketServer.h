/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>
#import "SocketCommunicationDefs.h"

#define kSocketServerNotificationMessageReceived @"messageReceived"
#define kSocketServerNotificationConnectionDied	 @"connectionDied"

#define kSocketServerCookieKey	@"__cookie"

@interface SocketServer : NSObject {
	NSFileHandle *serverSocketHandle;
	NSMutableArray *endPoints;
}

- (BOOL) start;
- (void) stop;
- (void) postMessageName: (NSString *) name
		userInfo: (NSDictionary *) userInfo;

@end
