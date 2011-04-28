/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import "ServerConnection.h"

@implementation ServerConnection

@synthesize outlineToplevel;
@synthesize statisticDict;
@synthesize serverinfo;
@synthesize sinks;

#pragma mark ### helper methods ###

- (void) sendDetailsChanged
{
	NSNotification *notification = [NSNotification notificationWithName: @"detailsChanged"
								     object: self];
	
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread: @selector(postNotification:)
							       withObject: notification
							    waitUntilDone: YES];
}

#pragma mark ### PulseAudio event callbacks ###

#if 0
- (void) statCallback: (const pa_stat_info *) i
{
	char t[32];
	
	[statisticDict setObject: [NSNumber numberWithInt: i->memblock_total]
			  forKey: @"Currently allocated memory blocks"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->memblock_total_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Current total size of allocated memory blocks"];
	[statisticDict setObject: [NSNumber numberWithInt: i->memblock_allocated]
			  forKey: @"Allocated memory blocks during the whole lifetime of the daemon"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->memblock_allocated_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Total size of all memory blocks allocated during the whole lifetime of the daemon"];
	[statisticDict setObject: [NSString stringWithCString: pa_bytes_snprint(t, sizeof(t), i->scache_size)
						     encoding: NSUTF8StringEncoding]
			  forKey: @"Total size of all sample cache entries"];     
	
	[self detailsChanged];
}
#endif

#pragma mark ### ServerConnection ###

- (id) init
{
	[super init];
	
	outlineToplevel = [NSMutableArray arrayWithCapacity: 0];
	[outlineToplevel retain];
	
	statisticDict = [NSMutableDictionary dictionaryWithCapacity: 0];
	[statisticDict retain];
	
	serverinfo = [NSMutableDictionary dictionaryWithCapacity: 0];
	cards = [NSMutableArray arrayWithCapacity: 0];
	sinks = [NSMutableArray arrayWithCapacity: 0];
	sources = [NSMutableArray arrayWithCapacity: 0];
	clients = [NSMutableArray arrayWithCapacity: 0];
	modules = [NSMutableArray arrayWithCapacity: 0];
	samplecache = [NSMutableArray arrayWithCapacity: 0];

	NSMutableDictionary *d;
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Server Information"
	      forKey: @"label"];
	[d setObject: serverinfo
	      forKey: @"parameters"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Cards"
	      forKey: @"label"];
	[d setObject: cards
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sinks"
	      forKey: @"label"];
	[d setObject: sinks
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sources"
	      forKey: @"label"];
	[d setObject: sources
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Clients"
	      forKey: @"label"];
	[d setObject: clients
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Modules"
	      forKey: @"label"];
	[d setObject: modules
	      forKey: @"children"];
	[outlineToplevel addObject: d];
	
	d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: @"Sample Cache"
	      forKey: @"label"];
	[d setObject: samplecache
	      forKey: @"children"];
	[outlineToplevel addObject: d];

	return self;
}

- (void) connectToServer: (NSString *) server;
{
	[sinks removeAllObjects];
	[sources removeAllObjects];
	[modules removeAllObjects];
	[clients removeAllObjects];
	[samplecache removeAllObjects];
	[serverinfo removeAllObjects];
	[cards removeAllObjects];

	if (connection)
		[connection release];
	
	connection = [[PAServerConnection alloc] init];
	connection.delegate = self;
	[connection connectToHost: server
			     port: -1];
}

- (void) dealloc
{
	[serverinfo removeAllObjects];
	[serverinfo release];

	[sinks removeAllObjects];
	[sinks release];

	[sources removeAllObjects];
	[sources release];

	[cards removeAllObjects];
	[cards release];

	[modules removeAllObjects];
	[modules release];

	[clients removeAllObjects];
	[clients release];

	[statisticDict removeAllObjects];
	[statisticDict release];

	[super dealloc];
}

- (void) reloadStatistics
{
	//pa_context_stat(context, pa_stat_cb, self);
}

#pragma mark ### PAServerConnectionDelegate ###

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection
{
	NSNotification *notification = [NSNotification notificationWithName: @"connectionEstablished"
								     object: self];
	
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread: @selector(postNotification:)
							       withObject: notification
							    waitUntilDone: YES];
}

- (void) PAServerConnectionFailed: (PAServerConnection *) connection
{
	NSLog(@"%s()", __func__);
}

- (void) PAServerConnectionEnded: (PAServerConnection *) connection
{
	NSNotification *notification = [NSNotification notificationWithName: @"connectionEnded"
								     object: self];
	
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread: @selector(postNotification:)
							       withObject: notification
							    waitUntilDone: YES];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	  serverInfoChanged: (PAServerInfo *) info
{
	[serverinfo setObject: info.userName
		       forKey: @"User Name"];
	[serverinfo setObject: info.hostName
		       forKey: @"Host Name"];
	[serverinfo setObject: info.version
		       forKey: @"Server Version"];
	[serverinfo setObject: info.serverName
		       forKey: @"Server Name"];
	[serverinfo setObject: info.sampleSpec
		       forKey: @"Sample Spec"];
	[serverinfo setObject: info.channelMap
		       forKey: @"Channel Map"];
	[serverinfo setObject: info.defaultSinkName
		       forKey: @"Default Sink Name"];
	[serverinfo setObject: info.defaultSourceName
		       forKey: @"Default Source Name"];
	[serverinfo setObject: [NSNumber numberWithInt: info.cookie]
			forKey: @"Cookie"];

	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	       cardsChanged: (NSArray *) _cards
{
	[cards removeAllObjects];
	
	for (PACardInfo *card in _cards) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: card.name
			       forKey: @"Card Name"];
		[parameters setObject: card.driver
			       forKey: @"Driver"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: card.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: card.properties
		      forKey: @"properties"];
		
		[cards addObject: d];
	}
	
	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	       sinksChanged: (NSArray *) _sinks
{
	[sinks removeAllObjects];
	
	for (PASinkInfo *sink in _sinks) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: sink.name
			       forKey: @"Sink Name"];
		[parameters setObject: sink.description
			       forKey: @"Description"];
		[parameters setObject: sink.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: sink.channelMap
			       forKey: @"Channel Map"];		
		[parameters setObject: [NSNumber numberWithInt: sink.latency]
			       forKey: @"Latency (us)"];		
		[parameters setObject: sink.driver
			       forKey: @"Driver"];		
		[parameters setObject: [NSNumber numberWithInt: sink.configuredLatency]
			       forKey: @"Configured Latency (us)"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: sink.name
		      forKey: @"label"];
		[d setObject: [NSNumber numberWithInt: sink.volume]
		      forKey: @"volume"];
		[d setObject: [NSNumber numberWithInt: sink.nVolumeSteps]
		      forKey: @"n_volume_steps"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: sink.properties
		      forKey: @"properties"];
		[d setObject: [NSValue valueWithPointer: sink]
		      forKey: @"infoPointer"];
		
		[sinks addObject: d];		
	}
	
	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     sourcesChanged: (NSArray *) _sources
{
	[sources removeAllObjects];
	
	for (PASourceInfo *source in _sources) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		
		[parameters setObject: source.name
			       forKey: @"Source Name"];
		[parameters setObject: source.description
			       forKey: @"Description"];
		[parameters setObject: source.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: source.channelMap
			       forKey: @"Channel Map"];		
		[parameters setObject: [NSNumber numberWithInt: source.latency]
			       forKey: @"Latency (us)"];		
		[parameters setObject: source.driver
			       forKey: @"Driver"];		
		[parameters setObject: [NSNumber numberWithInt: source.configuredLatency]
			       forKey: @"Configured Latency (us)"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: source.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: source.properties
		      forKey: @"properties"];
		[d setObject: [NSValue valueWithPointer: source]
		      forKey: @"infoPointer"];
		
		[sources addObject: d];		
	}	

	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     clientsChanged: (NSArray *) _clients
{
	[clients removeAllObjects];

	for (PAClientInfo *client in _clients) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: client.name
			       forKey: @"Module Name"];
		[parameters setObject: client.driver
			       forKey: @"Driver"];

		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: client.name
		      forKey: @"label"];
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: client.properties
		      forKey: @"properties"];
		
		[clients addObject: d];		
	}

	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     modulesChanged: (NSArray *) _modules
{
	[modules removeAllObjects];
	
	for (PAModuleInfo *module in _modules) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: module.name
			       forKey: @"Module Name"];
		[parameters setObject: module.argument ?: @""
			       forKey: @"Arguments"];
		[parameters setObject: [NSNumber numberWithInt: module.useCount]
			       forKey: @"Use count"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: module.name
		      forKey: @"label"];		
		[d setObject: parameters
		      forKey: @"parameters"];
		[d setObject: module.properties
		      forKey: @"properties"];
		
		[modules addObject: d];
	}	

	[self sendDetailsChanged];
}

- (void) PAServerConnection: (PAServerConnection *) connection
	     samplesChanged: (NSArray *) _samples
{
	[samplecache removeAllObjects];
	
	for (PASampleInfo *sample in _samples) {
		NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
		[parameters setObject: sample.name
			       forKey: @"Name"];
		[parameters setObject: sample.sampleSpec
			       forKey: @"Sample Spec"];
		[parameters setObject: sample.channelMap
			       forKey: @"Channel Map"];
		[parameters setObject: sample.fileName
			       forKey: @"File Name"];		
		[parameters setObject: [NSNumber numberWithInt: sample.duration]
			       forKey: @"Duration"];
		[parameters setObject: [NSNumber numberWithInt: sample.bytes]
			       forKey: @"bytes"];
		[parameters setObject: sample.lazy ? @"YES" : @"NO"
			       forKey: @"Lazy"];
		
		NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
		[d setObject: sample.name
		      forKey: @"label"];		
		[d setObject: parameters
		      forKey: @"parameters"];

		[samplecache addObject: d];
	}		

	[self sendDetailsChanged];
}

- (void) getServerInfo
{
}

@end