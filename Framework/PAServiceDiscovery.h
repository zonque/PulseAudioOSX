/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@class PAServiceDiscovery;

@protocol PAServiceDiscoveryDelegate
@optional

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	     serverAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	  serverDisappeared: (NSNetService *) service;

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	       sinkAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	    sinkDisappeared: (NSNetService *) service;

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	     sourceAppeared: (NSNetService *) service;
- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
	  sourceDisappeared: (NSNetService *) service;

@end

@interface PAServiceDiscovery : NSObject <NSNetServiceDelegate, NSNetServiceBrowserDelegate>
{
	NSNetServiceBrowser *serverBrowser;
	NSNetServiceBrowser *sourceBrowser;
	NSNetServiceBrowser *sinkBrowser;
	NSMutableArray *netServices;
	NSMutableArray *announcedServices;
	NSLock *lock;
	
	NSObject <PAServiceDiscoveryDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <PAServiceDiscoveryDelegate> *delegate;

- (void) start;
+ (NSString *) ipOfService: (NSNetService *) service;

@end
