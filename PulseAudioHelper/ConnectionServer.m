/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "ConnectionServer.h"
#import "ObjectNames.h"

@implementation ConnectionServer

- (void) connectionDied: (NSNotification *) notification
{
	NSConnection *connection = [notification object];
	NSLog(@"%s() %@", __func__, [connection statistics]);
	
	for (NSDistantObject *o in clientObjects)
		if ([o connectionForProxy] == connection) {
			[clientObjects removeObject: o];
			return;
		}
}

- (void) registerClientWithName: (NSString *) name
{
	NSDistantObject *proxy = [NSConnection rootProxyForConnectionWithRegisteredName: name
										   host: nil];

	NSLog(@"%s() %@", __func__, name);

	if (proxy)
		[clientObjects addObject: proxy];
	
	[proxy test: @"1235"];
}

- (BOOL) connection: (NSConnection *) parentConnection
	shouldMakeNewConnection: (NSConnection *) newConnnection
{
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionDied:)
						     name: NSConnectionDidDieNotification
						   object: newConnnection];

	return YES;
}

- (BOOL) connection: (NSConnection *) conn
      handleRequest: (NSDistantObjectRequest *) doReq
{
	NSLog(@"%s()", __func__);
	return NO;
}

- (id) init
{
	[super init];

	clientObjects = [NSMutableArray arrayWithCapacity: 0];

	NSConnection *connection = [NSConnection serviceConnectionWithName: @PAOSX_HelperName
								rootObject: self];	
	[connection setDelegate: self];
	
	[connection addRunLoop: [NSRunLoop currentRunLoop]];
	NSLog(@" conn = %p", connection);
	
	return self;
}

@end
