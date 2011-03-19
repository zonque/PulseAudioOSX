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

#pragma mark ### static forwards ###

static void subscribe_callback	(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
static void context_event_cb	(pa_context *c, const char *name, pa_proplist *p, void *userdata);
static void context_state_cb	(pa_context *c, void *userdata);
static void pa_stat_cb		(pa_context *c, const pa_stat_info *i, void *userdata);
static void pa_server_info_cb	(pa_context *c, const struct pa_server_info *i, void *userdata);
static void pa_sink_info_cb	(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
static void pa_card_info_cb	(pa_context *c, const pa_card_info *i, int eol, void *userdata);
static void pa_source_info_cb	(pa_context *c, const pa_source_info *i, int eol, void *userdata);
static void client_info_callback(pa_context *c, const struct pa_client_info *i, int eol, void *userdata);
static void pa_module_info_cb	(pa_context *c, const struct pa_module_info *i, int eol, void *userdata);
static void pa_client_info_cb	(pa_context *c, const struct pa_client_info *i, int eol, void *userdata);
static void pa_sample_info_cb	(pa_context *c, const struct pa_sample_info *i, int eol, void *userdata);

#pragma mark ### helper methods ###

- (void) detailsChanged
{
	[[NSNotificationCenter defaultCenter] postNotificationName:@"detailsChanged"
							    object: self
							  userInfo: nil];
}

- (NSDictionary *) createDictionaryFromProplist: (pa_proplist *) plist
{
	NSMutableDictionary *dict;
	void *state = NULL;
	const char *key, *val;
	
	if (!plist)
		return nil;
	
	dict = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	do {
		key = pa_proplist_iterate(plist, &state);
		if (!key)
			break;
		
		val = pa_proplist_gets(plist, key);
		[dict setValue: [NSString stringWithCString: val
						   encoding: NSUTF8StringEncoding]
			forKey: [NSString stringWithFormat: @"%s", key]];
	} while (state);
	
	return dict;
}


- (void) addServerInfo: (const pa_server_info *) info
{
	char tmp[0x100];
	
	[serverinfo setObject: [NSString stringWithCString: info->user_name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"User Name"];
	[serverinfo setObject: [NSString stringWithCString: info->host_name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Host Name"];
	[serverinfo setObject: [NSString stringWithCString: info->server_version
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Server Version"];
	[serverinfo setObject: [NSString stringWithCString: info->server_name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Server Name"];
	[serverinfo setObject: [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sample Spec"];
	[serverinfo setObject: [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Channel Map"];
	[serverinfo setObject: [NSString stringWithCString: info->default_sink_name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Default Sink Name"];
	[serverinfo setObject: [NSString stringWithCString: info->default_source_name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Default Source Name"];
	[serverinfo setObject: [NSString stringWithFormat: @"%08x", info->cookie]
		       forKey: @"Cookie"];
	
	[self detailsChanged];
}

- (void) addCardInfo: (const pa_card_info *) info
{
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	[parameters setObject: [NSString stringWithCString: info->name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Card Name"];
	
	[parameters setObject: [NSString stringWithCString: info->driver ?: ""
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Driver"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	
	[d setObject: parameters
	      forKey: @"parameters"];
	
	[d setObject: [self createDictionaryFromProplist: info->proplist]
	      forKey: @"properties"];
	
	[cards addObject: d];
}

- (void) addSinkInfo: (const pa_sink_info *) info
{	
	char tmp[0x100];
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[parameters setObject: [NSString stringWithCString: info->name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sink Name"];
	
	[parameters setObject: [NSString stringWithCString: info->description
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Description"];
	
	[parameters setObject: [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sample Spec"];
	
	[parameters setObject: [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Channel Map"];
	
	[parameters setObject: [NSNumber numberWithInt: info->latency]
		       forKey: @"Latency (us)"];
	
	[parameters setObject: [NSString stringWithCString: info->driver
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Driver"];
	
	[parameters setObject: [NSNumber numberWithInt: info->configured_latency]
		       forKey: @"Configured Latency (us)"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->description
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	[d setObject: [NSNumber numberWithInt: pa_cvolume_avg(&info->volume)]
	      forKey: @"volume"];
	[d setObject: [NSNumber numberWithInt: info->n_volume_steps]
	      forKey: @"n_volume_steps"];
	[d setObject: parameters forKey: @"parameters"];
	[d setObject: [self createDictionaryFromProplist: info->proplist]
	      forKey: @"properties"];
	
	[sinks addObject: d];
}

- (void) addSourceInfo: (const pa_source_info *) info
{
	char tmp[0x100];
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[parameters setObject: [NSString stringWithCString: info->name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sink Name"];
	[parameters setObject: [NSString stringWithCString: info->description
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Description"];
	[parameters setObject: [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sample Spec"];
	[parameters setObject: [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Channel Map"];
	[parameters setObject: [NSNumber numberWithInt: info->latency]
		       forKey: @"Latency (us)"];
	[parameters setObject: [NSString stringWithCString: info->driver
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Driver"];
	[parameters setObject: [NSNumber numberWithInt: info->configured_latency]
		       forKey: @"Configured Latency (us)"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->description
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	
	[d setObject: parameters
	      forKey: @"parameters"];
	
	[d setObject: [self createDictionaryFromProplist: info->proplist]
	      forKey: @"properties"];
	
	[sources addObject: d];
}

- (void) addModuleInfo: (const pa_module_info *) info
{
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	[parameters setObject: [NSString stringWithCString: info->name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Module Name"];
	
	[parameters setObject: [NSString stringWithCString: info->argument ?: ""
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Arguments"];
	
	[parameters setObject: [NSNumber numberWithInt: info->n_used]
		       forKey: @"Use count"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	
	[d setObject: parameters
	      forKey: @"parameters"];
	
	[d setObject: [self createDictionaryFromProplist: info->proplist]
	      forKey: @"properties"];
	
	[modules addObject: d];
}

- (void) addClientInfo: (const pa_client_info *) info
{
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	[parameters setObject: [NSString stringWithCString: info->name
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Module Name"];
	
	[parameters setObject: [NSString stringWithCString: info->driver
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Driver"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	
	[d setObject: parameters
	      forKey: @"parameters"];
	
	[d setObject: [self createDictionaryFromProplist: info->proplist]
	      forKey: @"properties"];
	
	[clients addObject: d];
}

- (void) addSampleInfo: (const pa_sample_info *) info
{
	char tmp[0x100];
	
	NSMutableDictionary *parameters = [NSMutableDictionary dictionaryWithCapacity: 0];
	[parameters setObject: [NSString stringWithCString: info->name 
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Name"];
	[parameters setObject: [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Sample Spec"];
	[parameters setObject: [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						  encoding: NSUTF8StringEncoding]
		       forKey: @"Channel Map"];
	[parameters setObject: [NSString stringWithCString: info->filename
						  encoding: NSUTF8StringEncoding]
		       forKey: @"File Name"];
	
	[parameters setObject: [NSNumber numberWithInt: info->duration]
		       forKey: @"Duration"];
	
	[parameters setObject: [NSNumber numberWithInt: info->bytes]
		       forKey: @"bytes"];
	
	[parameters setObject: info->lazy ? @"YES" : @"NO"
		       forKey: @"Lazy"];
	
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	[d setObject: [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding]
	      forKey: @"label"];
	
	[d setObject: parameters
	      forKey: @"parameters"];
	
	[samplecache addObject: d];
}


#pragma mark ### PulseAudio event callbacks ###

- (void) contextSubscriptionEventCallback: (enum pa_subscription_event_type) type
				    index: (UInt) index
{
	switch (type & ~PA_SUBSCRIPTION_EVENT_TYPE_MASK) {
		case PA_SUBSCRIPTION_EVENT_SINK:
			[sinks removeAllObjects];
			pa_context_get_sink_info_list(context, pa_sink_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SOURCE:
			[sources removeAllObjects];
			pa_context_get_source_info_list(context, pa_source_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_MODULE:
			[modules removeAllObjects];
			pa_context_get_module_info_list(context, pa_module_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_CLIENT:
			[clients removeAllObjects];
			pa_context_get_client_info_list(context, pa_client_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SAMPLE_CACHE:
			[samplecache removeAllObjects];
			pa_context_get_sample_info_list(context, pa_sample_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SERVER:
			[serverinfo removeAllObjects];
			pa_context_get_server_info(context, pa_server_info_cb, self);
			break;
		case PA_SUBSCRIPTION_EVENT_CARD:
			[cards removeAllObjects];
			pa_context_get_card_info_list(context, pa_card_info_cb, self);
			break;
			
		default:
			return;
	}
}

- (void) contextEventCallback: (const char *) name propList: (pa_proplist *) propList
{
}

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

- (void) contextChangeCallback
{
	int state = pa_context_get_state(context);
	NSMutableDictionary *userdata = [NSDictionary dictionaryWithObject: [NSNumber numberWithInt: state]
								    forKey: @"state"];

	//if (state == PA_CONTEXT_FAILED) ...
	
	[[NSNotificationCenter defaultCenter] postNotificationName: @"connectionStateChanged"
							    object: self
							  userInfo: userdata];

	if (state == PA_CONTEXT_READY) {
		pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_ALL, NULL, NULL);
		pa_context_set_subscribe_callback(context, subscribe_callback, self);
		[self getServerInfo];
	}

	/*
	if (context) {
		pa_context_unref(context);
		context = NULL;
	}
	 */
}

#pragma mark ### static wrappers ###

static void subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
	ServerConnection *sc = userdata;
	[sc contextSubscriptionEventCallback: t index: index];
}

static void context_event_cb(pa_context *c, const char *name, pa_proplist *p, void *userdata)
{
	ServerConnection *sc = userdata;
	[sc contextEventCallback: name propList: p];
}

static void context_state_cb(pa_context *c, void *userdata)
{
	ServerConnection *sc = userdata;
	[sc contextChangeCallback];
}

static void pa_stat_cb(pa_context *c, const pa_stat_info *i, void *userdata)
{
	ServerConnection *sc = userdata;
	[sc statCallback: i];
}

static void pa_server_info_cb(pa_context *c, const struct pa_server_info *i, void *userdata)
{
	ServerConnection *sc = userdata;
	[sc addServerInfo: i];
}

static void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addSinkInfo: i];
}

static void pa_card_info_cb(pa_context *c, const pa_card_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addCardInfo: i];
}

static void pa_source_info_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addSourceInfo: i];
}

static void client_info_callback(pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addClientInfo: i];
}

static void pa_module_info_cb(pa_context *c, const struct pa_module_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addModuleInfo: i];
}

static void pa_client_info_cb(pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addClientInfo: i];
}

static void pa_sample_info_cb(pa_context *c, const struct pa_sample_info *i, int eol, void *userdata)
{
	ServerConnection *sc = userdata;
	if (eol)
		[sc detailsChanged];
	else
		[sc addSampleInfo: i];
}

- (void) getServerInfo
{
	pa_context_get_server_info(context, pa_server_info_cb, self);
	pa_context_get_card_info_list(context, pa_card_info_cb, self);
	pa_context_get_sink_info_list(context, pa_sink_info_cb, self);
	pa_context_get_source_info_list(context, pa_source_info_cb, self);
	pa_context_get_module_info_list(context, pa_module_info_cb, self);
	pa_context_get_client_info_list(context, pa_client_info_cb, self);
	pa_context_get_sample_info_list(context, pa_sample_info_cb, self);
	pa_context_stat(context, pa_stat_cb, self);
}

#pragma mark ### fooo ###

- (void) PAMainloopThread : (id) param
{
	int ret;
	char *currentServer = NULL;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	mainloop = pa_mainloop_new();
	
	while (1) {
		if (connectRequest) {
			if (context) {
				pa_context_unref(context);
				pa_xfree(currentServer);
				context = NULL;
			}
			
			currentServer = pa_xstrdup(connectRequest);
			pa_xfree(connectRequest);
			connectRequest = NULL;
			
			context = pa_context_new(pa_mainloop_get_api(mainloop),
						 [[[NSProcessInfo processInfo] processName] cStringUsingEncoding: NSASCIIStringEncoding]);
			
			pa_context_set_event_callback(context, context_event_cb, self);
			pa_context_set_state_callback(context, context_state_cb, self);
			pa_context_connect(context, currentServer, 0, NULL);
			printf(" connect to >%s<\n", currentServer);
		}
		
		pa_mainloop_iterate(mainloop, true, &ret);
		//if (ret)
		//    break;
	}
	
	printf("PAMainloopThread exiting, ret = %d\n", ret);
	
	[pool release];
}

- (id) init
{
	NSMutableDictionary *d;
	
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

	[NSThread detachNewThreadSelector: @selector(PAMainloopThread:)
				 toTarget: self
			       withObject: nil];

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
	
	connectRequest = pa_xstrdup([server cStringUsingEncoding: NSASCIIStringEncoding]);
	pa_mainloop_wakeup(mainloop);
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
	pa_context_stat(context, pa_stat_cb, self);
}

@end