/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAServerConnectionInternal.h"
#import "PAServerConnectionImplementation.h"

@implementation PAServerConnectionImplementation

@synthesize serverInfo;
@synthesize cards;
@synthesize sinks;
@synthesize sources;
@synthesize clients;
@synthesize modules;
@synthesize samples;

@synthesize PAContext;
@synthesize PAMainLoop;

#pragma mark ### static forwards ###

static void staticContextStateCallback(pa_context *context, void *userdata);
static void staticContextSubscribeCallback(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
static void staticContextEventCallback(pa_context *c, const char *name, pa_proplist *p, void *userdata);
static void staticContextStatInfoCallback(pa_context *c, const pa_stat_info *i, void *userdata);
static void staticContextStatInfoCallback(pa_context *c, const pa_stat_info *i, void *userdata);
static void staticServerInfoCallback(pa_context *c, const struct pa_server_info *i, void *userdata);
static void staticSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
static void staticSinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
static void staticCardInfoCallback(pa_context *c, const pa_card_info *i, int eol, void *userdata);
static void staticSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata);
static void staticSourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, int eol, void *userdata);
static void staticClientInfoCallback(pa_context *c, const struct pa_client_info *i, int eol, void *userdata);
static void staticModuleInfoCallback(pa_context *c, const struct pa_module_info *i, int eol, void *userdata);
static void staticSampleInfoCallback(pa_context *c, const struct pa_sample_info *i, int eol, void *userdata);

#pragma mark ### callback catcher ###

- (void) contextStateCallback
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	int state = pa_context_get_state(PAContext);
	
	NSLog(@"Context state changed to %d", state);
	
	switch (state) {
		case PA_CONTEXT_READY:
			NSLog(@"Connection ready.");
			pa_context_subscribe(PAContext, PA_SUBSCRIPTION_MASK_ALL, NULL, NULL);
			pa_context_set_subscribe_callback(PAContext, staticContextSubscribeCallback, self);
			pa_context_set_event_callback(PAContext, staticContextEventCallback, self);
			pa_context_get_sink_info_list(PAContext, staticSinkInfoCallback, self);
			pa_context_get_sink_input_info_list(PAContext, staticSinkInputInfoCallback, self);			
			pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
			pa_context_get_source_output_info_list(PAContext, staticSourceOutputInfoCallback, self);
			pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
			pa_context_get_client_info_list(PAContext, staticClientInfoCallback, self);
			pa_context_get_sample_info_list(PAContext, staticSampleInfoCallback, self);
			pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
			pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
			[server performSelectorOnMainThread: @selector(sendDelegateConnectionEstablished)
						 withObject: nil
					      waitUntilDone: NO];
			break;
		case PA_CONTEXT_TERMINATED:
			NSLog(@"Connection terminated.");
			[server performSelectorOnMainThread: @selector(sendDelegateConnectionEnded)
						 withObject: nil
					      waitUntilDone: YES];
			pa_context_unref(PAContext);
			PAContext = NULL;
			break;
		case PA_CONTEXT_FAILED:
			NSLog(@"Connection failed.");
			[server performSelectorOnMainThread: @selector(sendDelegateConnectionFailed)
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
		case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
			[sinkInputs removeAllObjects];
			pa_context_get_sink_input_info_list(PAContext, staticSinkInputInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SOURCE:
			[sources removeAllObjects];
			pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
			[sourceOutputs removeAllObjects];
			pa_context_get_source_output_info_list(PAContext, staticSourceOutputInfoCallback, self);
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
	[serverInfo setFromInfoStruct: info
			       server: server];
	
	[server performSelectorOnMainThread: @selector(sendDelegateServerInfoChanged:)
				 withObject: serverInfo
			      waitUntilDone: NO];
	
	[pool drain];
}

- (void) cardInfoCallback: (const pa_card_info *) info
		      eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PACardInfo *card = [PACardInfo createFromInfoStruct: info
							     server: server];
		[cards addObject: card];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateCardsChanged:)
					 withObject: cards
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sinkInfoCallback: (const pa_sink_info *) info
		      eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PASinkInfo *sink = [PASinkInfo createFromInfoStruct: info
							     server: server];
		[sinks addObject: sink];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateSinksChanged:)
					 withObject: sinks
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sinkInputInfoCallback: (const pa_sink_input_info *) info
			   eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PASinkInputInfo *input = [PASinkInputInfo createFromInfoStruct: info
									server: server];
		[sinkInputs addObject: input];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateSinkInputsChanged:)
					 withObject: sinkInputs
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sourceInfoCallback: (const pa_source_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PASourceInfo *source = [PASourceInfo createFromInfoStruct: info
								   server: server];
		[sources addObject: source];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateSourcesChanged:)
					 withObject: sources
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sourceOutputInfoCallback: (const pa_source_output_info *) info
			      eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PASourceOutputInfo *output = [PASourceOutputInfo createFromInfoStruct: info
									       server: server];
		[sourceOutputs addObject: output];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateSourceOutputsChanged:)
					 withObject: sourceOutputs
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) clientInfoCallback: (const pa_client_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PAClientInfo *client = [PAClientInfo createFromInfoStruct: info
								   server: server];
		[clients addObject: client];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateClientsChanged:)
					 withObject: clients
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) moduleInfoCallback: (const pa_module_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PAModuleInfo *module = [PAModuleInfo createFromInfoStruct: info
								   server: server];
		[modules addObject: module];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateModulesChanged:)
					 withObject: modules
				      waitUntilDone: NO];		
	}
	
	[pool drain];
}

- (void) sampleInfoCallback: (const pa_sample_info *) info
			eol: (BOOL) eol
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (info) {
		PASampleInfo *sample = [PASampleInfo createFromInfoStruct: info
								   server: server];
		[samples addObject: sample];
	}
	
	if (eol) {
		[server performSelectorOnMainThread: @selector(sendDelegateSamplesChanged:)
					 withObject: samples
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

- (void) contextDefaultsSetCallback: (BOOL) success
{
	pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
}

#pragma mark ### static wrappers ###

// context
static void staticContextStateCallback(pa_context *context, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc contextStateCallback];
}

static void staticContextSubscribeCallback(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc contextSubscribeCallback: t
			       index: index];
}

static void staticContextEventCallback(pa_context *c, const char *name, pa_proplist *p, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc contextEventCallback: name
			propList: p];	 
}

// info callbacks

static void staticContextStatInfoCallback(pa_context *c, const pa_stat_info *i, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc statInfoCallback: i];
}

static void staticServerInfoCallback(pa_context *c, const struct pa_server_info *i, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc serverInfoCallback: i];
}

static void staticSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc sinkInfoCallback: i
			 eol: !!eol];
}

static void staticSinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc sinkInputInfoCallback: i
			      eol: !!eol];
}

static void staticCardInfoCallback(pa_context *c, const pa_card_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc cardInfoCallback: i
			 eol: !!eol];	
}

static void staticSourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc sourceInfoCallback: i
			   eol: !!eol];
}

static void staticSourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc sourceOutputInfoCallback: i
				 eol: !!eol];
}

static void staticClientInfoCallback(pa_context *c, const struct pa_client_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc clientInfoCallback: i
			   eol: !!eol];
}

static void staticModuleInfoCallback(pa_context *c, const struct pa_module_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc moduleInfoCallback: i
			   eol: !!eol];
}

static void staticSampleInfoCallback(pa_context *c, const struct pa_sample_info *i, int eol, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc sampleInfoCallback: i
			   eol: !!eol];
}

// module load/unload

static void staticModuleLoadedCallback(pa_context *c, uint32_t index, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc moduleLoadedCallback: index];
}

static void staticModuleUnloadedCallback(pa_context *c, int success, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc moduleUnloadedCallback: !!success];
}

static void staticContextDefaultsSetCallback(pa_context *c, int success, void *userdata)
{
	PAServerConnectionImplementation *sc = userdata;
	[sc contextDefaultsSetCallback: !!success];
}



#pragma mark  ################

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
	
	pa_context_set_state_callback(PAContext, staticContextStateCallback, self);
	
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
	audio = [[PAServerConnectionAudio alloc] initWithPAServerConnection: server
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
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_load_module(PAContext,
			       [name cStringUsingEncoding: NSASCIIStringEncoding],
			       [arguments cStringUsingEncoding: NSASCIIStringEncoding],
			       staticModuleLoadedCallback, self);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return YES;
}

- (BOOL) unloadModuleWithName: (NSString *) name
{
	if (![self isConnected])
		return NO;
	
	BOOL found = NO;
	
	for (PAModuleInfo *info in modules) {
		if ([info.name isEqualToString: name]) {
			pa_threaded_mainloop_lock(PAMainLoop);
			pa_context_unload_module(PAContext, info.index,
						 staticModuleUnloadedCallback, self);
			pa_threaded_mainloop_unlock(PAMainLoop);
			found = YES;
		}
	}
	
	return found;
}

- (BOOL) setDefaultSink: (NSString *) name
{
	if (![self isConnected])
		return NO;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_default_sink(PAContext, [name cStringUsingEncoding: NSASCIIStringEncoding],
				    staticContextDefaultsSetCallback, self);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return YES;
}

- (BOOL) setDefaultSource: (NSString *) name
{
	if (![self isConnected])
		return NO;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_set_default_source(PAContext, [name cStringUsingEncoding: NSASCIIStringEncoding],
				      staticContextDefaultsSetCallback, self);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return YES;
}

- (UInt32) protocolVersion
{
	if (![self isConnected])
		return 0;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	UInt32 version = pa_context_get_protocol_version(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return version;
}

- (UInt32) serverProtocolVersion
{
	if (![self isConnected])
		return 0;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	UInt32 version = pa_context_get_server_protocol_version(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return version;	
}

- (NSString *) clientName
{
	return [NSString stringWithCString: procName
				  encoding: NSASCIIStringEncoding];
}

- (BOOL) isLocal
{
	if (![self isConnected])
		return NO;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	BOOL ret = (pa_context_is_local(PAContext) == 1);
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return ret;
}

- (NSString *) serverName
{
	if (![self isConnected])
		return nil;
	
	if ([self isLocal])
		return @"Local server";
	
	pa_threaded_mainloop_lock(PAMainLoop);
	NSString *name = [NSString stringWithCString: pa_context_get_server(PAContext)
					    encoding: NSASCIIStringEncoding];
	pa_threaded_mainloop_unlock(PAMainLoop);
	
	return name;
}

- (void) shutdownServer
{
	pa_context_exit_daemon(PAContext, NULL, NULL);
}

- (id) initForServer: (PAServerConnection *) s;
{
	[super init];

	server = s;
	pa_get_binary_name(procName, sizeof(procName));

	serverInfo = [[[PAServerInfo alloc] init] retain];
	
	cards = [[NSMutableArray arrayWithCapacity: 0] retain];
	sinks = [[NSMutableArray arrayWithCapacity: 0] retain];
	sinkInputs = [[NSMutableArray arrayWithCapacity: 0] retain];
	sources = [[NSMutableArray arrayWithCapacity: 0] retain];
	sourceOutputs = [[NSMutableArray arrayWithCapacity: 0] retain];
	clients = [[NSMutableArray arrayWithCapacity: 0] retain];
	modules = [[NSMutableArray arrayWithCapacity: 0] retain];
	samples = [[NSMutableArray arrayWithCapacity: 0] retain];
		
	return self;
}

- (void) dealloc
{
	if (audio)
		[audio release];
	
	[cards release];
	[sinks release];
	[sinkInputs release];
	[sources release];
	[sourceOutputs release];
	[clients release];
	[modules release];
	[samples release];
	[serverInfo release];
	
	[super dealloc];
}

@end
