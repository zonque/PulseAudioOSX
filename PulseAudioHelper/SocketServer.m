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

#import "SocketServer.h"
#import "ObjectNames.h"
#import "SocketCommunicationDefs.h"

@implementation SocketServer

- (void) connectionEOF: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSFileHandle *handle = [userInfo objectForKey: NSFileHandleNotificationFileHandleItem];

	NSLog(@"%s() handle %p\n", __func__, handle);
	
	[endPoints removeObject: handle];
}

- (void) dataAvailable: (NSNotification *) notification
{
	NSData *data = [[notification object] availableData];
	
	if ([data length] == 0) {
		[self connectionEOF: notification];
		return;
	}

	NSString *errorString = nil;
	NSDictionary *dict = [NSPropertyListSerialization propertyListFromData: data
							      mutabilityOption: NSPropertyListImmutable
									format: NULL
							      errorDescription: &errorString];
	
	if (errorString || !dict) {
		NSLog(@"ERROR: %@\n", errorString);
		return;
	}

	[[NSNotificationCenter defaultCenter] postNotificationName: kSocketServerNotificationMessageReceived
							    object: self
							  userInfo: dict];
	
	[dict release];
	NSLog(@"%s() %@\n", __func__, dict);
}

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

- (void) connectionAccepted: (NSNotification *) notification
{
	NSDictionary *userInfo = [notification userInfo];
	NSFileHandle *handle = [userInfo objectForKey: NSFileHandleNotificationFileHandleItem];
	
	NSLog(@"%s() handle = %p\n", __func__, handle);

	if ([endPoints containsObject: handle]) {
		NSLog(@"%s(): object already known?\n", __func__);
		return;
	}

	[endPoints addObject: handle];
	
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(dataAvailable:) 
						     name: NSFileHandleDataAvailableNotification
						   object: handle];
	
	[[NSNotificationCenter defaultCenter] addObserver: self
						 selector: @selector(connectionEOF:) 
						     name: NSFileHandleReadToEndOfFileCompletionNotification
						   object: handle];
	
	[handle waitForDataInBackgroundAndNotify];
}

- (id) init
{
	[super init];
	
	endPoints = [[NSMutableArray arrayWithCapacity: 0] retain];

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
	
	return self;
}

- (void) dealloc
{
	[serverSocketHandle release];
	[endPoints release];
	[super dealloc];
}

@end
