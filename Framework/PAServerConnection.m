/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <pulse/pulseaudio.h>

#import "PulseAudio.h"

#pragma mark ### hidden interface ###

@interface PAServerConnection (hidden)

	char			 procName[100];
	MPSemaphoreID		 PAContextSemaphore;
	pa_buffer_attr		 bufAttr;
	pa_sample_spec		 sampleSpec;

	pa_threaded_mainloop	*PAMainLoop;
	pa_context		*PAContext;

	Float32			 sampleRate;
	UInt32			 ioBufferFrameSize;

	pa_stream		*PARecordStream;
	pa_stream		*PAPlaybackStream;

// context
- (void) contextStateCallback;
- (void) contextSubscribeCallback: (pa_subscription_event_type_t) type
			    index: (UInt32) index;
- (void) contextEventCallback: (const char *) name
		     propList: (pa_proplist *) propList;

// streams
- (void) streamStartedCallback: (pa_stream *) stream;
- (void) streamWriteCallback: (pa_stream *) stream
		      nBytes: (size_t) nBytes;
- (void) streamReadCallback: (pa_stream *) stream
		     nBytes: (size_t) nBytes;
- (void) streamOverflowCallback: (pa_stream *) stream;
- (void) streamUnderflowCallback: (pa_stream *) stream;
- (void) streamEventCallback: (pa_stream *) stream
			name: (const char *) name
		    propList: (pa_proplist *) propList;
- (void) streamBufferAttrCallback: (pa_stream *) stream;

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

// streams
static void staticStreamStartedCallback(pa_stream *stream, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamStartedCallback: stream];
}

static void staticStreamWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamWriteCallback: stream
			 nBytes: nbytes];
}

static void staticStreamReadCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamReadCallback: stream
			nBytes: nbytes];
}

static void staticStreamOverflowCallback(pa_stream *stream, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamOverflowCallback: stream];
}

static void staticStreamUnderflowCallback(pa_stream *stream, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamUnderflowCallback: stream];
}

static void staticStreamEventCallback(pa_stream *stream, const char *name, pa_proplist *pl, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamEventCallback: stream
			   name: name
		       propList: pl];
}

static void staticStreamBufferAttrCallback(pa_stream *stream, void *userdata)
{
	PAServerConnection *sc = userdata;
	[sc streamBufferAttrCallback: stream];
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

#pragma mark ### hidden implementation ###

@implementation PAServerConnection (hidden)

- (void) initHidden
{
	sampleSpec.format = PA_SAMPLE_FLOAT32;
	sampleSpec.rate = sampleRate;
	sampleSpec.channels = 2;

	bufAttr.tlength = ioBufferFrameSize * pa_frame_size(&sampleSpec);
	bufAttr.maxlength = -1;
	bufAttr.minreq = -1;
	bufAttr.prebuf = -1;

	OSStatus ret = MPCreateSemaphore(UINT_MAX, 0, &PAContextSemaphore);
	if (ret != 0)
		NSLog(@"MPCreateSemaphore() failed");

	pa_get_binary_name(procName, sizeof(procName));
	NSLog(@"binary name >%s<", procName);
}

- (void) deallocHidden
{
	if (PAMainLoop) {
		pa_threaded_mainloop_lock(PAMainLoop);
		pa_threaded_mainloop_unlock(PAMainLoop);
		pa_threaded_mainloop_stop(PAMainLoop);
		pa_threaded_mainloop_free(PAMainLoop);		
		PAMainLoop = NULL;
	}

	/*
	pa_xfree(inputDummyBuffer);
	inputDummyBuffer = NULL;
	
	pa_xfree(outputDummyBuffer);
	outputDummyBuffer = NULL;
	
	pa_xfree(connectHost);
	connectHost = NULL;
	*/

	MPDeleteSemaphore(PAContextSemaphore);
}

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
	int state = pa_context_get_state(PAContext);
	
	NSLog(@"Context state changed to %d", state);
	
	switch (state) {
		case PA_CONTEXT_READY:
		{
			NSLog(@"Connection ready.");
			[self performSelectorOnMainThread: @selector(sendDelegateConnectionEstablished)
					       withObject: nil
					    waitUntilDone: YES];

			pa_context_get_sink_info_list(PAContext, staticSinkInfoCallback, self);
			pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
			pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
			pa_context_get_client_info_list(PAContext, staticClientInfoCallback, self);
			pa_context_get_sample_info_list(PAContext, staticSampleInfoCallback, self);
			pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
			pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
			break;
		}
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
}

- (void) contextSubscribeCallback: (pa_subscription_event_type_t) type
			    index: (UInt32) index
{
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
			//[serverinfo removeAllObjects];
			pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
			break;
		case PA_SUBSCRIPTION_EVENT_CARD:
			[cards removeAllObjects];
			pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
			break;

		default:
			return;
	}
}

- (void) contextEventCallback: (const char *) name
		     propList: (pa_proplist *) propList
{
}

- (void) streamStartedCallback: (pa_stream *) stream
{
}

- (void) streamWriteCallback: (pa_stream *) stream
		      nBytes: (size_t) nBytes
{
}

- (void) streamReadCallback: (pa_stream *) stream
		     nBytes: (size_t) nBytes
{
}

- (void) streamOverflowCallback: (pa_stream *) stream
{
}

- (void) streamUnderflowCallback: (pa_stream *) stream
{
}

- (void) streamEventCallback: (pa_stream *) stream
			name: (const char *) name
		    propList: (pa_proplist *) propList
{
}

- (void) streamBufferAttrCallback: (pa_stream *) stream
{
}

// info

- (void) statInfoCallback: (const pa_stat_info *) i
{
}

- (void) serverInfoCallback: (const pa_server_info *) info
{
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
			    waitUntilDone: YES];
}

- (void) cardInfoCallback: (const pa_card_info *) info
		      eol: (BOOL) eol
{
	PACardInfo *card = [[PACardInfo alloc] init];	
	
	card.name = [NSString stringWithCString: info->name
				       encoding: NSUTF8StringEncoding];
	
	if (info->driver)
		card.driver = [NSString stringWithCString: info->driver
						 encoding: NSUTF8StringEncoding];

	card.properties = [self createDictionaryFromProplist: info->proplist];
	
	[cards addObject: card];

	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateCardsChanged)
				       withObject: nil
				    waitUntilDone: YES];		
	}
}

- (void) sinkInfoCallback: (const pa_sink_info *) info
		      eol: (BOOL) eol
{
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
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSinksChanged)
				       withObject: nil
				    waitUntilDone: YES];		
	}
}

- (void) sourceInfoCallback: (const pa_source_info *) info
			eol: (BOOL) eol
{
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
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSourcesChanged)
				       withObject: nil
				    waitUntilDone: YES];		
	}
	
}

- (void) clientInfoCallback: (const pa_client_info *) info
			eol: (BOOL) eol
{
	PAClientInfo *client = [[PAClientInfo alloc] init];

	client.name = [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding];
	client.driver = [NSString stringWithCString: info->driver
					   encoding: NSUTF8StringEncoding];
	client.properties = [self createDictionaryFromProplist: info->proplist];
	[clients addObject: client];
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateClientsChanged)
				       withObject: nil
				    waitUntilDone: YES];		
	}
}

- (void) moduleInfoCallback: (const pa_module_info *) info
			eol: (BOOL) eol
{
	PAModuleInfo *module = [[PAClientInfo alloc] init];

	module.name = [NSString stringWithCString: info->name
					 encoding: NSUTF8StringEncoding];
	if (info->argument)
		module.argument = [NSString stringWithCString: info->argument
						     encoding: NSUTF8StringEncoding];
	module.useCount = info->n_used;
	module.properties = [self createDictionaryFromProplist: info->proplist];
	
	[modules addObject: module];
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateModulesChanged)
				       withObject: nil
				    waitUntilDone: YES];		
	}
}

- (void) sampleInfoCallback: (const pa_sample_info *) info
			eol: (BOOL) eol
{
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
	
	if (eol) {
		[self performSelectorOnMainThread: @selector(sendDelegateSamplesChanged)
				       withObject: nil
				    waitUntilDone: YES];				
	}
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
	[self initHidden];
	
	serverInfo = [[PAServerInfo alloc] init];

	cards = [NSMutableArray arrayWithCapacity: 0];
	sinks = [NSMutableArray arrayWithCapacity: 0];
	sources = [NSMutableArray arrayWithCapacity: 0];
	clients = [NSMutableArray arrayWithCapacity: 0];
	modules = [NSMutableArray arrayWithCapacity: 0];
	samples = [NSMutableArray arrayWithCapacity: 0];
	
	return self;
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

- (BOOL) isConnected
{
	if (!PAMainLoop || !PAContext)
		return NO;
	
	pa_threaded_mainloop_lock(PAMainLoop);
	pa_context_state_t state = pa_context_get_state(PAContext);
	pa_threaded_mainloop_unlock(PAMainLoop);

	return state == PA_CONTEXT_READY;
}

- (BOOL) addAudioStreams
{
	if (![self isConnected])
		return NO;

	char tmp[sizeof(procName) + 10];
	int ret;
	
	snprintf(tmp, sizeof(tmp), "%s playback", procName);
	
	pa_threaded_mainloop_lock(PAMainLoop);
	PAPlaybackStream = pa_stream_new(PAContext, tmp, &sampleSpec, NULL);
	
	if (!PAPlaybackStream) {
		pa_threaded_mainloop_unlock(PAMainLoop);
		return NO;
	}

	pa_stream_set_event_callback(PAPlaybackStream, staticStreamEventCallback, self);
	pa_stream_set_write_callback(PAPlaybackStream, staticStreamWriteCallback, self);
	pa_stream_set_started_callback(PAPlaybackStream, staticStreamStartedCallback, self);
	pa_stream_set_overflow_callback(PAPlaybackStream, staticStreamOverflowCallback, self);
	pa_stream_set_underflow_callback(PAPlaybackStream, staticStreamUnderflowCallback, self);
	pa_stream_set_buffer_attr_callback(PAPlaybackStream, staticStreamBufferAttrCallback, self);
	ret = pa_stream_connect_playback(PAPlaybackStream, NULL, &bufAttr,
					 (pa_stream_flags_t)  (PA_STREAM_INTERPOLATE_TIMING |
							       PA_STREAM_AUTO_TIMING_UPDATE),
					 NULL, NULL);
	pa_threaded_mainloop_unlock(PAMainLoop);

	if (ret != 0)
		return NO;
	
	/*
	 snprintf(tmp, sizeof(tmp), "%s record", procName);
	 pa_threaded_mainloop_lock(PAMainLoop);
	 PARecordStream = pa_stream_new(PAContext, tmp, &sampleSpec, NULL);
	 //pa_stream_set_read_callback(PARecordStream, staticStreamReadCallback, self);
	 pa_stream_set_event_callback(PARecordStream, staticStreamEventCallback, self);
	 pa_stream_set_overflow_callback(PARecordStream, staticStreamOverflowCallback, self);
	 pa_stream_set_underflow_callback(PARecordStream, staticStreamUnderflowCallback, self);
	 pa_stream_set_buffer_attr_callback(PARecordStream, staticStreamBufferAttrCallback, self);
	 pa_stream_connect_record(PARecordStream, NULL, &bufAttr, (pa_stream_flags_t) 0);
	 pa_threaded_mainloop_unlock(PAMainLoop);
	 */
	
	return YES;
}

@end
