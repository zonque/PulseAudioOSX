/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Cocoa/Cocoa.h>

@protocol ServerConnectionDelegate
@required
- (void) serverConnection: (id) serverConnection
       newClientAnnounced: (NSString *) name;
- (void) serverConnection: (id) serverConnection
	  clientSignedOff: (NSString *) name;

@end


@interface ServerConnection : NSObject {
	id<ServerConnectionDelegate> delegate;
	NSDistributedNotificationCenter *notificationCenter;
}

- (void) setDelegate: (id<ServerConnectionDelegate>) newDelegate;

@end
