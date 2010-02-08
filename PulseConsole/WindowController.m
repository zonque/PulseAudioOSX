/***
  This file is part of PulseConsole

  Copyright 2010 Daniel Mack <daniel@caiaq.de>

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

#import "StreamListView.h"
#import "WindowController.h"

@implementation WindowController

#pragma mark ### static wrappers ###

static void subscribe_callback(struct pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
    WindowController *w = userdata;
    [w contextSubscriptionEventCallback: t index: index];
}

static void context_event_cb(pa_context *c, const char *name, pa_proplist *p, void *userdata)
{
    WindowController *w = userdata;
    [w contextEventCallback: name propList: p];
}

static void context_state_cb(pa_context *c, void *userdata)
{
    WindowController *w = userdata;
    [w contextChangeCallback];
}

static void pa_stat_cb(pa_context *c, const pa_stat_info *i, void *userdata)
{
    WindowController *w = userdata;
    [w statCallback: i];
}

static void pa_server_info_cb(struct pa_context *c, const struct pa_server_info *i, void *userdata)
{
    WindowController *w = userdata;
    [w addServerInfo: i];
}

static void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addSinkInfo: i];
}

static void pa_card_info_cb(pa_context *c, const pa_card_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addCardInfo: i];
}

static void pa_source_info_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addSourceInfo: i];
}

static void client_info_callback(struct pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addClientInfo: i];
}

static void pa_module_info_cb(struct pa_context *c, const struct pa_module_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addModuleInfo: i];
}

static void pa_client_info_cb(struct pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addClientInfo: i];
}

static void pa_sample_info_cb(struct pa_context *c, const struct pa_sample_info *i, int eol, void *userdata)
{
    WindowController *w = userdata;
    if (eol)
        [w performSelectorOnMainThread: @selector(repaintViews)
                            withObject: nil
                         waitUntilDone: NO];
    else
        [w addSampleInfo: i];
}

#pragma mark ### PulseAudio event callbacks ###

- (void) contextSubscriptionEventCallback: (enum pa_subscription_event_type) type index: (UInt) index
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

    [self performSelectorOnMainThread: @selector(repaintViews) withObject: nil waitUntilDone: NO];
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

    [self performSelectorOnMainThread: @selector(repaintViews) withObject: nil waitUntilDone: NO];
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
	[sinkStreamListView addStreamView: info
							   ofType: StreamTypeSink
								 name: [NSString stringWithCString: info->description
														  encoding: NSUTF8StringEncoding]];
	
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
	[sourceStreamListView addStreamView: info
								 ofType: StreamTypeSource
								   name: [NSString stringWithCString: info->description
															encoding: NSUTF8StringEncoding]];

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

- (void) contextChangeCallback
{
    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_CONNECTING:
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: [NSString stringWithFormat: @"Connection to %s ...", pa_context_get_server(context)]
										 waitUntilDone: NO];
            return;

        case PA_CONTEXT_AUTHORIZING:
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: @"Authorizing ..."
										 waitUntilDone: NO];
            return;

        case PA_CONTEXT_SETTING_NAME:
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: @"Setting name ..." waitUntilDone: NO];
			return;

        case PA_CONTEXT_READY: {
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: @"Connection established"
										 waitUntilDone: NO];
            [self getServerInfo];
            [self performSelectorOnMainThread: @selector(enableGUI)
								   withObject: nil
								waitUntilDone: NO];

            pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_ALL, NULL, NULL);
            pa_context_set_subscribe_callback(context, subscribe_callback, self);

            return;
        }

        case PA_CONTEXT_TERMINATED:
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: @"Connection terminated"
										 waitUntilDone: NO];
            [self performSelectorOnMainThread: @selector(stopProgressIndicator)
								   withObject: nil
								waitUntilDone: NO];

            break;

        case PA_CONTEXT_FAILED:
            printf("FAILED %d\n", pa_context_errno(context));
            [connectStatus performSelectorOnMainThread: @selector(setStringValue:)
											withObject: @"Connection failed"
										 waitUntilDone: NO];
            [self performSelectorOnMainThread: @selector(stopProgressIndicator)
								   withObject: nil
								waitUntilDone: NO];
            break;
    }

    if (context) {
        pa_context_unref(context);
        context = NULL;
    }
}

#pragma mark ### fooo ###

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

- (void) repaintViews
{
    [outlineView reloadItem: nil
			 reloadChildren: YES];
    [outlineView expandItem: nil
			 expandChildren: YES];
    [parameterTableView reloadData];
    [propertyTableView reloadData];
    [statisticsTableView reloadData];

    [window setTitle: [NSString stringWithFormat: @"%@@%@",
                [serverinfo valueForKey: @"Server Name"],
                [serverinfo valueForKey: @"Host Name"]]];
}

- (void) stopProgressIndicator
{
    [connectionProgressIndicator stopAnimation: self];
    [connectionProgressIndicator setHidden: YES];
}

- (void) enableGUI
{
    [self stopProgressIndicator];
    [outlineView setEnabled: YES];
    [parameterTableView setEnabled: YES];
    [propertyTableView setEnabled: YES];
    [statisticsTableView setEnabled: YES];
}

- (void) bonjourServiceAdded: (NSNotification *) notification
{
    NSDictionary *dict = [notification userInfo];
    NSString *name = [dict valueForKey: @"name"];

    [serverSelector addItemWithTitle: name];
    
    if ([serverSelector numberOfItems] == 1) {
        [self connectToServer: name];
        [connectionProgressIndicator startAnimation: self];
    }
}

- (void) awakeFromNib
{
    NSMutableDictionary *d;
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

    [outlineView setEnabled: NO];
    [parameterTableView setEnabled: NO];
    [propertyTableView setEnabled: NO];
    [statisticsTableView setEnabled: NO];

    listener = [[BonjourListener alloc] initForService: "_pulse-server._tcp"];

	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(bonjourServiceAdded:)
		name:@"serviceAdded" object: listener];

	[NSThread detachNewThreadSelector: @selector(PAMainloopThread:) toTarget: self withObject: nil];
//    [serverSelector addItemWithTitle: @"daniel@ubuntu.local."];
//    [serverSelector addItemWithTitle: @"172.16.193.164"];

    [listener start];
}

#pragma mark ### NSOutlineViewSource protocol ###

- (NSInteger) outlineView: (NSOutlineView *) outlineView numberOfChildrenOfItem: (id) item
{
    if (item == nil) {
        NSEnumerator *enumerator = [outlineToplevel objectEnumerator];
        NSString *obj;
        UInt count = 0;
 
        while ((obj = [enumerator nextObject])) {
            NSDictionary *d = [obj valueForKey: @"children"];
            if (d && [d count]) {
                count++;
                continue;
            }

            d = [obj valueForKey: @"parameters"];
            if (d && [d count]) {
                count++;
                continue;
            }
        }
                
        return count;
    }

    NSArray *d = [item valueForKey: @"children"];

    return d ? [d count] : 0;
}

- (BOOL) outlineView: (NSOutlineView *) outlineView isItemExpandable:(id)item
{
    if (item == nil)
        return [outlineToplevel count] > 0;

    NSArray *d = [item valueForKey: @"children"];
    return d ? [d count] > 0 : NO;
}

- (id) outlineView: (NSOutlineView *) outlineView child: (NSInteger)index ofItem:(id)item
{
    if (item == nil)
        return [outlineToplevel objectAtIndex: index];

    NSArray *d = [item valueForKey: @"children"];
    return [d objectAtIndex: index];
}

- (id) outlineView: (NSOutlineView *) outlineView objectValueForTableColumn: (NSTableColumn *) tableColumn byItem: (id)item
{
    return [item valueForKey: @"label"];
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView:(NSTableView *)aTableView
    setObjectValue:obj
    forTableColumn:(NSTableColumn *)col
    row:(int)rowIndex
{
}

- (id)tableView:(NSTableView *)tableView
    objectValueForTableColumn:(NSTableColumn *)col
    row:(int)rowIndex
{
    NSDictionary *item = nil;

    if (!activeItem && (tableView != statisticsTableView))
        return @"";

    if (tableView == parameterTableView)
        item = [activeItem valueForKey: @"parameters"];
    else if (tableView == propertyTableView)
        item = [activeItem valueForKey: @"properties"];
    else if (tableView == statisticsTableView)
        item = statisticDict;
    
    if (!item)
        return @"";

    if ([[col identifier] isEqualToString: @"key"])
        return [[item allKeys] objectAtIndex: rowIndex];

    if ([[col identifier] isEqualToString: @"value"])
        return [[item allValues] objectAtIndex: rowIndex];

    return @"";
}

- (int) numberOfRowsInTableView:(NSTableView *)tableView
{
    NSDictionary *item = nil;

    if (tableView == parameterTableView)
        item = [activeItem valueForKey: @"parameters"];
    else if (tableView == propertyTableView)
        item = [activeItem valueForKey: @"properties"];
    else if (tableView == statisticsTableView)
        item = statisticDict;

    return item ? [item count] : 0;
}

#pragma mark ### delegate methods ###

- (BOOL) outlineView: (NSOutlineView *) outlineView shouldEditTableColumn: (NSTableColumn *)tableColumn item:(id)item
{
    return NO;
}

- (void) outlineViewSelectionDidChange: (NSNotification *) notification
{
    NSDictionary *d = [outlineView itemAtRow: [outlineView selectedRow]];

    activeItem = d;
    [parameterTableView reloadData];
    [propertyTableView reloadData];
}

- (void) connectToServer: (NSString *) server
{
	activeItem = nil;

    [connectStatus setStringValue: @"Connecting ..."];
    [connectionProgressIndicator startAnimation: self];
    [connectionProgressIndicator setHidden: NO];

    connectRequest = pa_xstrdup([server cStringUsingEncoding: NSASCIIStringEncoding]);
    pa_mainloop_wakeup(mainloop);
}

#pragma mark ### IBActions ###

- (IBAction) connectToServerAction: (id) sender
{
    [serverinfo removeAllObjects];
    [sinks removeAllObjects];
    [sources removeAllObjects];
    [cards removeAllObjects];
    [modules removeAllObjects];
    [clients removeAllObjects];
    [statisticDict removeAllObjects];
	
	[sinkStreamListView removeAllStreams];
	[sourceStreamListView removeAllStreams];
	[playbackStreamListView removeAllStreams];
	[recordStreamListView removeAllStreams];
	
    [outlineView setEnabled: NO];
    [parameterTableView setEnabled: NO];
    [propertyTableView setEnabled: NO];
    [self repaintViews];

    [self connectToServer: [sender titleOfSelectedItem]];
}

- (IBAction) displayAbout: (id) sender
{
    NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
    [d setValue: @"(c) 2009 Daniel Mack"
		 forKey: @"Copyright"];
    [d setValue: [NSString stringWithFormat: @"pulseaudio library version %s", pa_get_library_version()]
		 forKey: @"Copyright"];

    [NSApp orderFrontStandardAboutPanelWithOptions: d];
}

- (IBAction) reloadStatistics: (id) sender
{
    pa_context_stat(context, pa_stat_cb, self);
}

@end