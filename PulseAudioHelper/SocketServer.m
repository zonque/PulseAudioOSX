/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <netinet/in.h>
#import <sys/socket.h>
#import <sys/un.h>
#import <sys/ioctl.h>

#import "SocketServer.h"
#import "ObjectNames.h"

@implementation SocketServer

- (void) postMessageName: (NSString *) name
		userInfo: (NSDictionary *) userInfo
{
	if ([userInfo objectForKey: @PAOSX_MessageNameKey]) {
		NSLog(@"ERROR: userInfo must not contain value for key %@\n", @PAOSX_MessageNameKey);
		return;
	}

	NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary: userInfo];
	[dict setObject: name
		 forKey: @PAOSX_MessageNameKey];
	
	NSString *errorString = nil;
	NSData *data = [NSPropertyListSerialization dataFromPropertyList: dict
								  format: NSPropertyListXMLFormat_v1_0
							errorDescription: &errorString];
	[dict release];
	
	if (errorString) {
		NSLog(@"ERROR: Unable to serialize data: %@\n", errorString);
		[errorString release];
		return;
	}
	
	for (NSFileHandle *handle in endPoints)
		[handle writeData: data];

	[data release];
}

- (void) threadEntry: (NSFileHandle *) handle
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSData *data;
	
	while (data = [handle availableData]) {

		if ([data length] == 0) {
			[endPoints performSelectorOnMainThread: @selector(removeObject:)
						    withObject: handle
						 waitUntilDone: YES];

			NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithObject: [NSValue valueWithPointer: handle]
										       forKey: kSocketServerCookieKey];

			NSNotification *notification = [NSNotification notificationWithName: kSocketServerNotificationConnectionDied
										     object: self
										   userInfo: dict];
			
			[[NSNotificationCenter defaultCenter] performSelectorOnMainThread: @selector(postNotification:)
									       withObject: notification
									    waitUntilDone: YES];

			[pool release];
			return;
		}
		
		NSLog(@" .. %d\n", [data length]);
		
		NSString *errorString = nil;
		NSMutableDictionary *dict = [NSPropertyListSerialization propertyListFromData: data
									     mutabilityOption: NSPropertyListImmutable
										       format: NULL
									     errorDescription: &errorString];
		if (errorString || !dict) {
			NSLog(@"ERROR: %@\n", errorString);
			continue;
		}
		
		[dict setObject: [NSValue valueWithPointer: handle]
			 forKey: kSocketServerCookieKey];
		
		NSNotification *notification = [NSNotification notificationWithName: kSocketServerNotificationMessageReceived
									     object: self
									   userInfo: dict];

		[[NSNotificationCenter defaultCenter] performSelectorOnMainThread: @selector(postNotification:)
								       withObject: notification
								    waitUntilDone: YES];
	}
	
	[pool release];
}

- (void) connectionAccepted: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSFileHandle *handle = [userInfo objectForKey: NSFileHandleNotificationFileHandleItem];
	
	if ([endPoints containsObject: handle]) {
		NSLog(@"%s(): object already known?\n", __func__);
		return;
	}

	[endPoints addObject: handle];

	[NSThread detachNewThreadSelector: @selector(threadEntry:)
				 toTarget: self
			       withObject: handle];
	
	[serverSocketHandle acceptConnectionInBackgroundAndNotify];
}

- (BOOL) start
{
	unlink(PAOSX_HelperSocketName);
	
	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	server.sun_len = sizeof(server);
	strlcpy(server.sun_path, PAOSX_HelperSocketName, sizeof(server.sun_path));
	
	NSData *address = [NSData dataWithBytes: &server
					 length: sizeof(server)];
	
	NSSocketPort *sock = [NSSocketPort alloc];
	[sock initWithProtocolFamily: AF_UNIX
			  socketType: SOCK_STREAM
			    protocol: 0
			     address: address];
	
	serverSocketHandle = [[NSFileHandle alloc] initWithFileDescriptor: [sock socket]
							   closeOnDealloc: YES];
	
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionAccepted:) 
						     name: NSFileHandleConnectionAcceptedNotification
						   object: serverSocketHandle];
	
	[serverSocketHandle acceptConnectionInBackgroundAndNotify];
	
	[[NSDistributedNotificationCenter defaultCenter] postNotificationName: @PAOSX_HelperMsgServiceStarted
								       object: @PAOSX_HelperName
								     userInfo: nil
							   deliverImmediately: YES];
	
	return YES;
}

- (void) stop
{
	[serverSocketHandle release];	
}

- (id) init
{
	[super init];
	
	endPoints = [[NSMutableArray arrayWithCapacity: 0] retain];
	
	return self;
}

- (void) dealloc
{
	[self stop];
	[endPoints release];
	[super dealloc];
}

@end
