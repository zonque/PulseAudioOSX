/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>

#import "PulseAudio.h"
#import "PAServerConnectionAudio.h"

static NSString *frameworkPath = @"/Library/Frameworks/PulseAudio.framework/";

#pragma mark ### hidden interface ###

@interface PAServerConnection (hidden)
	char			 procName[100];

	pa_threaded_mainloop	*PAMainLoop;
	pa_context		*PAContext;

	PAServerConnectionAudio *audio;

// context
- (void) contextStateCallback;
- (void) contextSubscribeCallback: (pa_subscription_event_type_t) type
			    index: (UInt32) index;
- (void) contextEventCallback: (const char *) name
		     propList: (pa_proplist *) propList;

// info

- (void) statInfoCallback: (const pa_stat_info *) i;
- (void) serverInfoCallback: (const pa_server_info *) i;
- (void) cardInfoCallback: (const pa_card_info *) i
		      eol: (BOOL) eol;
- (void) sinkInfoCallback: (const pa_sink_info *) i
		      eol: (BOOL) eol;
- (void) sourceInfoCallback: (const pa_source_info *) i
			eol: (BOOL) eol;
- (void) clientInfoCallback: (const pa_client_info *) i
			eol: (BOOL) eol;
- (void) moduleInfoCallback: (const pa_module_info *) i
			eol: (BOOL) eol;
- (void) sampleInfoCallback: (const pa_sample_info *) i
			eol: (BOOL) eol;

// modules
- (void) moduleLoadedCallback: (UInt32) index;
- (void) moduleUnloadedCallback: (BOOL) success;

@end

#pragma mark ### static wrappers ###

// context
static void staticContextStateCallback(pa_context *context, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc contextStateCallback];
}

static void staticContextSubscribeCallback(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc contextSubscribeCallback: t
			       index: index];
}

static void staticContextEventCallback(pa_context *c, const char *name, pa_proplist *p, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc contextEventCallback: name
			propList: p];	 
}

// info callbacks

static void staticContextStatInfoCallback(pa_context *c, const pa_stat_info *i, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc statInfoCallback: i];
}

static void staticServerInfoCallback(pa_context *c, const struct pa_server_info *i, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc serverInfoCallback: i];
}

static void staticSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc sinkInfoCallback: i
			 eol: !!eol];
}

static void staticCardInfoCallback(pa_context *c, const pa_card_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc cardInfoCallback: i
			 eol: !!eol];	
}

static void staticSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc sourceInfoCallback: i
			   eol: !!eol];
}

static void staticClientInfoCallback(pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc clientInfoCallback: i
			   eol: !!eol];
}

static void staticModuleInfoCallback(pa_context *c, const struct pa_module_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc moduleInfoCallback: i
			   eol: !!eol];
}

static void staticSampleInfoCallback(pa_context *c, const struct pa_sample_info *i, int eol, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc sampleInfoCallback: i
			   eol: !!eol];
}

// module load/unload

static void staticModuleLoadedCallback(pa_context *c, uint32_t index, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc moduleLoadedCallback: index];
}

static void staticModuleUnloadedCallback(pa_context *c, int success, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc moduleUnloadedCallback: !!success];
}

#pragma mark ### hidden implementation ###

@implementation PAServerConnection (hidden)

#pragma mark ### delegate handling ###

- (void) sendDelegateConnectionEstablished
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionEstablished:)])
		[delegate PAServerConnectionEstablished: self];
}

- (void) sendDelegateConnectionEnded
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionEnded:)])
		[delegate PAServerConnectionEnded: self];
}

- (void) sendDelegateConnectionFailed
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionFailed:)])
		[delegate PAServerConnectionFailed: self];
}

- (void) sendDelegateServerInfoChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:serverInfoChanged:)])
		[delegate PAServerConnection: self
			   serverInfoChanged: serverInfo];
}

- (void) sendDelegateCardsChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:cardsChanged:)])
		[delegate PAServerConnection: self
				cardsChanged: cards];	
}

- (void) sendDelegateSinksChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinksChanged:)])
		[delegate PAServerConnection: self
				sinksChanged: sinks];
}

- (void) sendDelegateSourcesChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourcesChanged:)])
		[delegate PAServerConnection: self
			      sourcesChanged: sources];
}

- (void) sendDelegateClientsChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:clientsChanged:)])
		[delegate PAServerConnection: self
			      clientsChanged: clients];
}

- (void) sendDelegateModulesChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:modulesChanged:)])
		[delegate PAServerConnection: self
			      modulesChanged: modules];
}

- (void) sendDelegateSamplesChanged
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:samplesChanged:)])
		[delegate PAServerConnection: self
			      samplesChanged: samples];
}

#pragma mark ### callback catcher ###


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

- (void) contextStateCallback
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	int state = pa_context_get_state(PAContext);
	
	NSLog(@"Context state changed to %d", state);
	
	switch (state) {
		case PA_CONTEXT_READY:
			NSLog(@"Connection ready.");
			pa_context_get_sink_info_list(PAContext, staticSinkInfoCallback, self);
			pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
			pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
			pa_context_get_client_info_list(PAContext, staticClientInfoCallback, self);
			pa_context_get_sample_info_list(PAContext, staticSampleInfoCallback, self);
			pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
			pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
			NSLog(@"%s() :%d", __func__, __LINE__);
			[self performSelectorOnMainThread: @selector(sendDelegateConnectionEstablished)
					       withObject: nil
					    waitUntilDone: NO];
			NSLog(@"%s() :%d", __func__, __LINE__);
			break;
		case PA_CONTEXT_TERMINATED:
			NSLog(@"Connection terminated.");
			[self performSelectorOnMainThread: @selector(sendDelegateConnectionEnded)
					       withObject: nil
					    waitUntilDone: YES];
			pa_context_unref(PAContext);
			PAContext = NULL;
			break;
		case PA_CONTEXT_FAILED:
			NSLog(@"Connection failed.");
			[self performSelectorOnMainThread: @selector(sendDelegateConnectionFailed)
					       withObject: nil
					    waitUntilDone: YES];
			pa_context_unref(PAContext);
			PAContext = NULL;
			break;
		default:
			break;
	}
	
	[pool drain];
}

- (void) contextSubscribeCallback: (pa_subscription_event_type_t) type
			    index: (UInt32) index
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	switch (type & ~PA_SUBSCRIPTION_EVENT_TYPE_MASK) {
		case PA_SUBSCRIPTION_EVENT_SINK:
			[sinks removeAllObjects];
			pa_context_get_sink_info_list(PAContext, staticSinkInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SOURCE:
			[sources removeAllObjects];
			pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_MODULE:
			[modules removeAllObjects];
			pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_CLIENT:
			[clients removeAllObjects];
			pa_context_get_client_info_list(PAContext, staticClientInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SAMPLE_CACHE:
			[samples removeAllObjects];
			pa_context_get_sample_info_list(PAContext, staticSampleInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SERVER:
			pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_CARD:
			[cards removeAllObjects];
			pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
			break;

		default:
			break;
	}
	
	[pool drain];
}

- (void) contextEventCallback: (const char *) name
		     propList: (pa_proplist *) propList
{
}

// info

- (void) statInfoCallback: (const pa_stat_info *) i
{
}

- (void) serverInfoCallback: (const pa_server_info *) info
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	char tmp[0x100];
	
	serverInfo.userName = [NSString stringWithCString: info->user_name
						 encoding: NSUTF8StringEncoding];
	serverInfo.hostName = [NSString stringWithCString: info->host_name
						 encoding: NSUTF8StringEncoding];
	serverInfo.serverName = [NSString stringWithCString: info->server_name
						   encoding: NSUTF8StringEncoding];
	serverInfo.version = [NSString stringWithCString: info->server_version
						encoding: NSUTF8StringEncoding];
	serverInfo.sampleSpec = [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						   encoding: NSUTF8StringEncoding];
	serverInfo.channelMap = [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						   encoding: NSUTF8StringEncoding];
	serverInfo.defaultSinkName = [NSString stringWithCString: info->default_sink_name
							encoding: NSUTF8StringEncoding];
	serverInfo.defaultSourceName = [NSString stringWithCString: info->default_source_name
							  encoding: NSUTF8StringEncoding];
	serverInfo.cookie = info->cookie;

	[self performSelectorOnMainThread: @selector(sendDelegateServerInfoChanged)
			       withObject: nil
			    waitUntilDone: NO];
	
	[pool drain];
}

- (void) cardInfoCallback: (const pa_card_info *) info
		      eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PACardInfo *card = [[PACardInfo alloc] init];	
		
		card.name = [NSString stringWithCString: info->name
					       encoding: NSUTF8StringEncoding];
		
		if (info->driver)
			card.driver = [NSString stringWithCString: info->driver
							 encoding: NSUTF8StringEncoding];

		card.properties = [self createDictionaryFromProplist: info->proplist];
		
		[cards addObject: card];
	}
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateCardsChanged)
				       withObject: nil
				    waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sinkInfoCallback: (const pa_sink_info *) info
		      eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PASinkInfo *sink = [[PASinkInfo alloc] init];
		char tmp[0x100];

		sink.name = [NSString stringWithCString: info->name
					       encoding: NSUTF8StringEncoding];
		sink.description = [NSString stringWithCString: info->description
						      encoding: NSUTF8StringEncoding];
		sink.sampleSpec = [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						     encoding: NSUTF8StringEncoding];
		sink.channelMap = [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						     encoding: NSUTF8StringEncoding];
		sink.driver = [NSString stringWithCString: info->driver
						 encoding: NSUTF8StringEncoding];
			
		sink.latency = info->latency;
		sink.configuredLatency = info->configured_latency;
		sink.nVolumeSteps = info->n_volume_steps;
		sink.volume = pa_cvolume_avg(&info->volume);	
		sink.properties = [self createDictionaryFromProplist: info->proplist];

		[sinks addObject: sink];
	}

	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSinksChanged)
				       withObject: nil
				    waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sourceInfoCallback: (const pa_source_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PASourceInfo *source = [[PASourceInfo alloc] init];
		char tmp[0x100];

		source.name = [NSString stringWithCString: info->name
						 encoding: NSUTF8StringEncoding];
		source.description = [NSString stringWithCString: info->description
							encoding: NSUTF8StringEncoding];
		source.sampleSpec = [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						       encoding: NSUTF8StringEncoding];
		source.channelMap = [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						       encoding: NSUTF8StringEncoding];
		source.driver = [NSString stringWithCString: info->driver
						   encoding: NSUTF8StringEncoding];
		source.latency = info->latency;
		source.configuredLatency = info->configured_latency;
		source.properties = [self createDictionaryFromProplist: info->proplist];
		[sources addObject: source];
	}
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSourcesChanged)
				       withObject: nil
				    waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) clientInfoCallback: (const pa_client_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PAClientInfo *client = [[PAClientInfo alloc] init];

		client.name = [NSString stringWithCString: info->name
						 encoding: NSUTF8StringEncoding];
		client.driver = [NSString stringWithCString: info->driver
						   encoding: NSUTF8StringEncoding];
		client.properties = [self createDictionaryFromProplist: info->proplist];
		[clients addObject: client];
	}
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateClientsChanged)
				       withObject: nil
				    waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) moduleInfoCallback: (const pa_module_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PAModuleInfo *module = [[PAModuleInfo alloc] init];

		module.name = [NSString stringWithCString: info->name
						 encoding: NSUTF8StringEncoding];
		if (info->argument)
			module.argument = [NSString stringWithCString: info->argument
							     encoding: NSUTF8StringEncoding];
		module.index = info->index;
		module.useCount = info->n_used;
		module.properties = [self createDictionaryFromProplist: info->proplist];
		
		[modules addObject: module];
	}

	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateModulesChanged)
				       withObject: nil
				    waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sampleInfoCallback: (const pa_sample_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (info) {
		PASampleInfo *sample = [[PASampleInfo alloc] init];
		char tmp[100];
		
		sample.name = [NSString stringWithCString: info->name 
						 encoding: NSUTF8StringEncoding];
		sample.sampleSpec = [NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
						       encoding: NSUTF8StringEncoding];
		sample.channelMap = [NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
						       encoding: NSUTF8StringEncoding];
		sample.fileName = [NSString stringWithCString: info->filename
						     encoding: NSUTF8StringEncoding];
		sample.duration = info->duration;
		sample.bytes = info->bytes;
		sample.lazy = info->lazy;
		
		[samples addObject: sample];
	}

	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSamplesChanged)
				       withObject: nil
				    waitUntilDone: NO];				
	}
	
	[pool drain];
}

- (void) moduleLoadedCallback: (UInt32) index
{
	[modules removeAllObjects];
	pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
}

- (void) moduleUnloadedCallback: (BOOL) success
{
	[modules removeAllObjects];
	pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
}

@end


@implementation PAServerConnection

@synthesize delegate;
@synthesize serverInfo;
@synthesize cards;
@synthesize sinks;
@synthesize sources;
@synthesize clients;
@synthesize modules;
@synthesize samples;

- (id) init
{
	[super init];
	
	serverInfo = [[[PAServerInfo alloc] init] retain];

	cards = [[NSMutableArray arrayWithCapacity: 0] retain];
	sinks = [[NSMutableArray arrayWithCapacity: 0] retain];
	sources = [[NSMutableArray arrayWithCapacity: 0] retain];
	clients = [[NSMutableArray arrayWithCapacity: 0] retain];
	modules = [[NSMutableArray arrayWithCapacity: 0] retain];
	samples = [[NSMutableArray arrayWithCapacity: 0] retain];
	
	pa_get_binary_name(procName, sizeof(procName));

	return self;
}

- (void) dealloc
{
	if (audio)
		[audio release];
	
	[cards release];
	[sinks release];
	[sources release];
	[clients release];
	[modules release];
	[samples release];
	[serverInfo release];

	[super dealloc];
}

- (void) connectToHost: (NSString *) hostName
		  port: (int) port
{
	int ret;
	
	if (PAContext)
		return;

	if (!PAMainLoop) {
		PAMainLoop = pa_threaded_mainloop_new();
		if (!PAMainLoop)
			NSLog(@"pa_threaded_mainloop_new() failed");
		
		ret = pa_threaded_mainloop_start(PAMainLoop);
		if (ret)
			NSLog(@"pa_threaded_mainloop_start() failed");
	}
		
	pa_threaded_mainloop_lock(PAMainLoop);
	
	pa_mainloop_api *api = pa_threaded_mainloop_get_api(PAMainLoop);
	if (!api)
		NSLog(@"pa_threaded_mainloop_get_api() failed");
	
	PAContext = pa_context_new(api, procName);
	if (!PAContext)
		NSLog(@"pa_context_new() failed");
	
	pa_context_subscribe(PAContext, PA_SUBSCRIPTION_MASK_ALL, NULL, NULL);
	pa_context_set_state_callback(PAContext, staticContextStateCallback, self);
	pa_context_set_subscribe_callback(PAContext, staticContextSubscribeCallback, self);
	pa_context_set_event_callback(PAContext, staticContextEventCallback, self);

	ret = pa_context_connect(PAContext, NULL,
				 (pa_context_flags_t) (PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN),
				 NULL);
	
	pa_threaded_mainloop_unlock(PAMainLoop);
}

- (void) disconnect
{
	if (!PAMainLoop)
		return;

	if (PAContext) {
		pa_threaded_mainloop_lock(PAMainLoop);
		
		if (audio) {
			[audio release];
			audio = nil;
		}
	
		pa_context_disconnect(PAContext);
		//pa_context_unref(PAContext);
		PAContext = nil;
		pa_threaded_mainloop_unlock(PAMainLoop);
	}
	
	pa_threaded_mainloop_stop(PAMainLoop);
	pa_threaded_mainloop_free(PAMainLoop);		
	PAMainLoop = NULL;	
}	

- (BOOL) isConnected
{
	if (!PAMainLoop || !PAContext)
		return NO;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_state_t state = pa_context_get_state(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);

	return state == PA_CONTEXT_READY;
}

- (BOOL) addAudioStreams: (UInt32) nChannels
	      sampleRate: (Float32) sampleRate
	ioProcBufferSize: (UInt32) ioProcBufferSize
{
	if (![self isConnected])
		return NO;
	
	if (audio)
		return YES;

	pa_threaded_mainloop_lock(PAMainLoop);
	audio = [[PAServerConnectionAudio alloc] initWithPAServerConnection: self
								    context: PAContext
								  nChannels: nChannels
								 sampleRate: sampleRate
							   ioProcBufferSize: ioProcBufferSize];
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return audio != nil;
}

- (BOOL) loadModuleWithName: (NSString *) name
		  arguments: (NSString *) arguments
{
	if (![self isConnected])
		return NO;

	pa_context_load_module(PAContext,
			       [name cStringUsingEncoding: NSASCIIStringEncoding],
			       [arguments cStringUsingEncoding: NSASCIIStringEncoding],
			       staticModuleLoadedCallback, self);
	
	return YES;
}

- (BOOL) unloadModuleWithName: (NSString *) name
{
	if (![self isConnected])
		return NO;

	BOOL found = NO;
	
	for (PAModuleInfo *info in modules) {
		if ([info.name isEqualToString: name]) {
			pa_context_unload_module(PAContext, info.index,
						 staticModuleUnloadedCallback, self);
			found = YES;
		}
	}

	return found;
}

- (NSString *) clientName
{
	return [NSString stringWithCString: procName
				  encoding: NSASCIIStringEncoding];
}

- (void) shutdownServer
{
	pa_context_exit_daemon(PAContext, NULL, NULL);
}

+ (NSString *) libraryVersion
{
	return [NSString stringWithCString: pa_get_library_version()
				  encoding: NSASCIIStringEncoding];	
}

+ (NSString *) frameworkPath
{
	return frameworkPath;
}

+ (NSArray *) availableModules
{
	NSError *error = nil;
	NSString *path = [NSString stringWithFormat: @"%@/Resources/lib/modules/", frameworkPath];
	NSArray *list = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: path
									    error: &error];
	CFShow(list);

	if (error)
		return nil;

	NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
	
	for (NSString *entry in list)
		if ([entry hasSuffix: @".so"])
			[array addObject: [entry stringByDeletingPathExtension]];
	
	return array;
}


@end
