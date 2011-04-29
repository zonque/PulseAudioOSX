/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <netinet/in.h>
#import <Foundation/Foundation.h>
#import "PAServiceDiscovery.h"

static NSString *kMDNSPulseServer = @"_pulse-server._tcp";
static NSString *kMDNSPulseSink   = @"_pulse-sink._tcp";
static NSString *kMDNSPulseSource = @"_pulse-source._tcp";
static NSString *kMDNSLocalDomain = @"local.";

@implementation PAServiceDiscovery

@synthesize delegate;

- (id) init
{
	[super init];
	
	serverBrowser	= [[[NSNetServiceBrowser alloc] init] retain];
	sinkBrowser	= [[[NSNetServiceBrowser alloc] init] retain];
	sourceBrowser	= [[[NSNetServiceBrowser alloc] init] retain];
	
	[serverBrowser	setDelegate: self];
	[sinkBrowser	setDelegate: self];
	[sourceBrowser	setDelegate: self];

	netServices = [NSMutableArray arrayWithCapacity: 0];
	announcedServices = [NSMutableArray arrayWithCapacity: 0];

	lock = [[NSLock alloc] init];
	
	return self;
}

- (void) dealloc
{
	[serverBrowser release];
	[sinkBrowser release];
	[sourceBrowser release];
	[netServices release];
	[announcedServices release];
	[lock release];
	[super dealloc];
}

- (void) start
{
	[serverBrowser searchForServicesOfType: kMDNSPulseServer
				      inDomain: kMDNSLocalDomain];
	[sinkBrowser searchForServicesOfType: kMDNSPulseSink
				    inDomain: kMDNSLocalDomain];
	[sourceBrowser searchForServicesOfType: kMDNSPulseSource
				      inDomain: kMDNSLocalDomain];	
}

+ (NSString *) ipOfService: (NSNetService *) service
{
	NSData *addr = [[service addresses] objectAtIndex: 0];
	struct sockaddr_in *address_sin = (struct sockaddr_in *)[addr bytes];
	return [NSString stringWithFormat: @"%d.%d.%d.%d",
		(address_sin->sin_addr.s_addr >> 0) & 0xff,
		(address_sin->sin_addr.s_addr >> 8) & 0xff,
		(address_sin->sin_addr.s_addr >> 16) & 0xff,
		(address_sin->sin_addr.s_addr >> 24) & 0xff];
}


#pragma mark ### NSNetServiceDelegate ###

- (void)netServiceDidResolveAddress:(NSNetService *)sender
{
	NSArray *addresses = [sender addresses];
	
	if (!delegate)
		return;

	if ([addresses count] == 0)
		return;
	
	[lock lock];
	
	if ([announcedServices containsObject: sender]) {
		[lock unlock];
		return;
	}
	
	NSString *type = [sender type];
	
	if ([type hasPrefix: kMDNSPulseServer] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:serverAppeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
			      serverAppeared: sender];
	}
	
	if ([type hasPrefix: kMDNSPulseSink] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:sinkAppeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
				sinkAppeared: sender];
	}
	
	if ([type hasPrefix: kMDNSPulseSource] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:sourceAppeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
			      sourceAppeared: sender];
	}
	
	[announcedServices addObject: sender];
	[lock unlock];
}

- (void) netService: (NSNetService *) sender
      didNotResolve: (NSDictionary *) errorDict
{
	[lock lock];
	[netServices removeObject: sender];
	[announcedServices removeObject: sender];
	[lock unlock];
}

- (void)netServiceDidStop:(NSNetService *)sender
{
	[lock lock];
	[netServices removeObject: sender];
	[announcedServices removeObject: sender];
	[lock unlock];
}

#pragma mark ### NSNetServiceBrowserDelegate ###

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	    didFindService: (NSNetService *) netService
	        moreComing: (BOOL) moreServicesComing
{
	[lock lock];
	[netServices addObject: netService];
	[lock unlock];

	[netService setDelegate: self];
	[netService resolveWithTimeout: 10.0];
}

- (void) netServiceBrowser: (NSNetServiceBrowser *) netServiceBrowser
	  didRemoveService: (NSNetService *) netService
		moreComing: (BOOL) moreServicesComing
{
	if (!delegate)
		return;

	NSString *type = [netService type];
	
	if ([type hasPrefix: kMDNSPulseServer] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:serverDisappeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
			   serverDisappeared: netService];
	}
	
	if ([type hasPrefix: kMDNSPulseSink] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:sinkDisappeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
			     sinkDisappeared: netService];
	}
	
	if ([type hasPrefix: kMDNSPulseSource] &&
	    [delegate respondsToSelector: @selector(PAServiceDiscovery:sourceDisappeared:)]) {
		NSLog(@"%s() :%d", __func__, __LINE__);
		[delegate PAServiceDiscovery: self
			   sourceDisappeared: netService];
	}
	
	[lock lock];
	[netServices removeObject: netService];
	[announcedServices removeObject: netService];
	[lock unlock];	
}

@end
