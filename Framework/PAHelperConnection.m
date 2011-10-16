/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import "PAHelperConnection.h"
#import "ULINetSocket.h"

@implementation PAHelperConnection

@synthesize delegate;
@synthesize socket;

#pragma mark ### NotificationCenter callbacks ###

- (void) dispatchMessage: (NSData *) msg
{
	if (!delegate ||
	    ![delegate respondsToSelector: @selector(PAHelperConnection:receivedMessage:dict:)])
		return;
	
	NSString *errorString;
	NSDictionary *dict = [NSPropertyListSerialization propertyListFromData: msg
                                                          mutabilityOption: NSPropertyListImmutable
                                                                    format: NULL
                                                          errorDescription: &errorString];
    
	if (errorString || !dict) {
		NSLog(@"ERROR: %@\n", errorString);
		return;
	}
	
	[delegate PAHelperConnection: self
                 receivedMessage: [dict objectForKey: PAOSX_MessageNameKey]
                            dict: [dict objectForKey: PAOSX_MessageDictionaryKey]];
}

- (void) sendMessage: (NSString *) name
                dict: (NSDictionary *) dict;
{
	NSMutableDictionary *msg = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[msg setObject: name
            forKey: PAOSX_MessageNameKey];
	[msg setObject: dict
            forKey: PAOSX_MessageDictionaryKey];
	
	if (![socket isConnected])
		return;
    
	NSString *errorString;
	NSData *data = [NSPropertyListSerialization dataFromPropertyList: msg
                                                              format: NSPropertyListXMLFormat_v1_0
                                                    errorDescription: &errorString];
    
	if (errorString || !data) {
		NSLog(@"ERROR: %@\n", errorString);
		return;
	}
	
	PAHelperProtocolHeader hdr;
	hdr.magic = PAOSX_HelperMagic;
	hdr.length = [data length];
	
	NSMutableData *payload = [NSMutableData dataWithBytes: &hdr
                                                   length: sizeof(hdr)];
	[payload appendData: data];	
	[socket writeData: payload];
}

- (id) init
{
	[super init];
    
	inboundData = [[NSMutableData data] retain];
	
	socket = [[ULINetSocket alloc] init];
	[socket setDelegate: self];
	[socket retain];
	
	return self;
}

- (id) initWithSocket: (ULINetSocket *) s
{
	[super init];
    
	inboundData = [[NSMutableData data] retain];
	socket = [s retain];
	[socket setDelegate: self];
	[socket open];
    
	return self;	
}

- (void) scheduleOnCurrentRunLoop
{
	[socket scheduleOnCurrentRunLoop];
}

- (void) dealloc
{
	[socket close];
	[socket release];
	[inboundData release];
	
	if (retryTimer)
		[retryTimer invalidate];
    
	[super dealloc];
}

- (void) retryConnect: (NSTimer *) timer
{
	if ([socket connectToLocalSocketPath: PAOSX_HelperSocket]) {
		NSLog(@"success!");
		[socket scheduleOnCurrentRunLoop];
		[retryTimer invalidate];
	}
}

- (BOOL) connectWithRetry: (BOOL) _retry
{
	retry = _retry;
    
	if ([socket isConnected])
		return YES;

	if ([socket connectToLocalSocketPath: PAOSX_HelperSocket])
		return [socket scheduleOnCurrentRunLoop];
	else {
		if (retry) {
			retryTimer = [NSTimer timerWithTimeInterval: 5.0
                                                 target: self
                                               selector: @selector(retryConnect:)
                                               userInfo: nil
                                                repeats: YES];
			[[NSRunLoop currentRunLoop] addTimer: retryTimer
                                         forMode: NSRunLoopCommonModes];
		}
		return NO;
	}
}

- (BOOL) isConnected
{
    return [socket isConnected];
}

#pragma mark ### ULINetSocketDelegate

-(void) netsocketConnected: (ULINetSocket*) inNetSocket
{
	if (delegate &&
	    [delegate respondsToSelector: @selector(PAHelperConnectionEstablished:)])
		[delegate PAHelperConnectionEstablished: self];
}

-(void)	netsocketDisconnected: (ULINetSocket*) inNetSocket
{
	if (delegate &&
	    [delegate respondsToSelector: @selector(PAHelperConnectionDied:)])
		[delegate PAHelperConnectionDied: self];
	
	if (retry)
		[self connectWithRetry: YES];
}

-(void)	netsocket: (ULINetSocket*) inNetSocket
connectionTimedOut: (NSTimeInterval) inTimeout
{
	[self netsocketDisconnected: inNetSocket];
    
	if (retry)
		[self connectWithRetry: YES];
}

-(void)	netsocket: (ULINetSocket*) inNetSocket
    dataAvailable: (unsigned) inAmount
{
	[inboundData appendData: [inNetSocket readData]];
	NSUInteger pos = 0;
    
	for (;;) {
		if (pos >= [inboundData length])
			break;
		
		Byte *b = ((Byte *) [inboundData bytes]) + pos;
		const PAHelperProtocolHeader *hdr = (const PAHelperProtocolHeader *) b;
        
		if (hdr->magic != PAOSX_HelperMagic) {
			NSLog(@"Protocol error");
			[socket close];
			[socket release];
			socket = nil;
			return;
		}
		
		// if the message is not yet fully received, just wait for more data
		if (hdr->length > [inboundData length])
			break;
        
		pos += sizeof(*hdr);
		[self dispatchMessage: [inboundData subdataWithRange: NSMakeRange(pos, hdr->length)]];
		pos += hdr->length;
	}
	
	if ([inboundData length] == pos) {
		[inboundData setLength: 0];
	} else {
		NSData *newData = [inboundData subdataWithRange: NSMakeRange(pos, [inboundData length] - pos)];
		[inboundData setData: newData];
	}
}

@end
