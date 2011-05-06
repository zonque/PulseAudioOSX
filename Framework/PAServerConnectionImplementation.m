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

@synthesize lastError;
@synthesize PAContext;
@synthesize PAMainLoop;

@synthesize presentCards;
@synthesize presentSinks;
@synthesize presentSinkInputs;
@synthesize presentSources;
@synthesize presentSourceOutputs;
@synthesize presentClients;
@synthesize presentModules;
@synthesize presentSamples;
@synthesize serverInfo;

@synthesize sinkForPlayback;
@synthesize sourceForRecord;

#pragma mark ### static forwards ###
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

#pragma mark ### error handling ###

- (void) updateError
{
        if (lastError) {
                [lastError release];
                lastError = nil;
        }

        if (!PAContext)
                return;

        const char *e = pa_strerror(pa_context_errno(PAContext));
        if (e)
                lastError = [NSString stringWithCString: e
                                               encoding: NSASCIIStringEncoding];
}

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
                        [self updateError];
                        [server performSelectorOnMainThread: @selector(sendDelegateConnectionEnded)
                                                 withObject: nil
                                              waitUntilDone: YES];
                        pa_context_unref(PAContext);
                        PAContext = NULL;
                        break;
                case PA_CONTEXT_FAILED:
                        NSLog(@"Connection failed.");
                        [self updateError];
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
                        [presentSinks removeAllObjects];
                        pa_context_get_sink_info_list(PAContext, staticSinkInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
                        [presentSinkInputs removeAllObjects];
                        pa_context_get_sink_input_info_list(PAContext, staticSinkInputInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_SOURCE:
                        [presentSources removeAllObjects];
                        pa_context_get_source_info_list(PAContext, staticSourceInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
                        [presentSourceOutputs removeAllObjects];
                        pa_context_get_source_output_info_list(PAContext, staticSourceOutputInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_MODULE:
                        [presentModules removeAllObjects];
                        pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_CLIENT:
                        [presentClients removeAllObjects];
                        pa_context_get_client_info_list(PAContext, staticClientInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_SAMPLE_CACHE:
                        [presentSamples removeAllObjects];
                        pa_context_get_sample_info_list(PAContext, staticSampleInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_CARD:
                        [presentCards removeAllObjects];
                        pa_context_get_card_info_list(PAContext, staticCardInfoCallback, self);
                        break;
                case PA_SUBSCRIPTION_EVENT_SERVER:
                        pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
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
                for (PACardInfo *card in publishedCards)
                        if (card.index == info->index) {
                                [presentCards addObject: card];
                                [card loadFromInfoStruct: info];
                                return;
                        }

                PACardInfo *card = [PACardInfo createFromInfoStruct: info
                                                             server: server];
                [publishedCards addObject: card];
                [presentCards addObject: card];
                [server performSelectorOnMainThread: @selector(sendDelegateCardInfoAdded:)
                                         withObject: card
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PACardInfo *card in publishedCards)
                        if (![presentCards containsObject: card])
                                [d addObject: card];

                for (PACardInfo *card in d) {
                        [publishedCards removeObject: card];
                        [server performSelectorOnMainThread: @selector(sendDelegateCardInfoRemoved:)
                                                 withObject: card
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) sinkInfoCallback: (const pa_sink_info *) info
                      eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PASinkInfo *sink in publishedSinks)
                        if (sink.index == info->index) {
                                [presentSinks addObject: sink];
                                [sink loadFromInfoStruct: info];
                                return;
                        }

                PASinkInfo *sink = [PASinkInfo createFromInfoStruct: info
                                                             server: server];
                [publishedSinks addObject: sink];
                [presentSinks addObject: sink];
                [server performSelectorOnMainThread: @selector(sendDelegateSinkInfoAdded:)
                                         withObject: sink
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PASinkInfo *sink in publishedSinks)
                        if (![presentSinks containsObject: sink])
                                [d addObject: sink];

                for (PASinkInfo *sink in d) {
                        [publishedSinks removeObject: sink];
                        [server performSelectorOnMainThread: @selector(sendDelegateSinkInfoRemoved:)
                                                 withObject: sink
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) sinkInputInfoCallback: (const pa_sink_input_info *) info
                           eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PASinkInputInfo *sinkInput in publishedSinkInputs)
                        if (sinkInput.index == info->index) {
                                [presentSinkInputs addObject: sinkInput];
                                [sinkInput loadFromInfoStruct: info];
                                return;
                        }

                PASinkInputInfo *sinkInput = [PASinkInputInfo createFromInfoStruct: info
                                                                            server: server];
                [publishedSinkInputs addObject: sinkInput];
                [presentSinkInputs addObject: sinkInput];
                [server performSelectorOnMainThread: @selector(sendDelegateSinkInputInfoAdded:)
                                         withObject: sinkInput
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                NSLog(@"publishedSinkInputs: %d presentSinkInputs: %d", [publishedSinkInputs count], [presentSinkInputs count]);

                for (PASinkInputInfo *sinkInput in publishedSinkInputs)
                        if (![presentSinkInputs containsObject: sinkInput])
                                [d addObject: sinkInput];

                for (PASinkInputInfo *sinkInput in d) {
                        [publishedSinkInputs removeObject: sinkInput];
                        [server performSelectorOnMainThread: @selector(sendDelegateSinkInputInfoRemoved:)
                                                 withObject: sinkInput
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) sourceInfoCallback: (const pa_source_info *) info
                        eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PASourceInfo *source in publishedSources)
                        if (source.index == info->index) {
                                [presentSources addObject: source];
                                [source loadFromInfoStruct: info];
                                return;
                        }

                PASourceInfo *source = [PASourceInfo createFromInfoStruct: info
                                                                   server: server];
                [publishedSources addObject: source];
                [presentSources addObject: source];
                [server performSelectorOnMainThread: @selector(sendDelegateSourceInfoAdded:)
                                         withObject: source
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PASourceInfo *source in publishedSources)
                        if (![presentSources containsObject: source])
                                [d addObject: source];

                for (PASourceInfo *source in d) {
                        [publishedSources removeObject: source];
                        [server performSelectorOnMainThread: @selector(sendDelegateSourceInfoRemoved:)
                                                 withObject: source
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) sourceOutputInfoCallback: (const pa_source_output_info *) info
                              eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PASourceOutputInfo *sourceOutput in publishedSourceOutputs)
                        if (sourceOutput.index == info->index) {
                                [presentSourceOutputs addObject: sourceOutput];
                                [sourceOutput loadFromInfoStruct: info];
                                return;
                        }

                PASourceOutputInfo *sourceOutput = [PASourceOutputInfo createFromInfoStruct: info
                                                                                     server: server];
                [publishedSourceOutputs addObject: sourceOutput];
                [presentSourceOutputs addObject: sourceOutput];
                [server performSelectorOnMainThread: @selector(sendDelegateSourceOutputInfoAdded:)
                                         withObject: sourceOutput
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PASourceOutputInfo *sourceOutput in publishedSourceOutputs)
                        if (![presentSourceOutputs containsObject: sourceOutput])
                                [d addObject: sourceOutput];

                for (PASourceOutputInfo *sourceOutput in d) {
                        [publishedSourceOutputs removeObject: sourceOutput];
                        [server performSelectorOnMainThread: @selector(sendDelegateSourceOutputInfoRemoved:)
                                                 withObject: sourceOutput
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) clientInfoCallback: (const pa_client_info *) info
                        eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PAClientInfo *client in publishedClients)
                        if (client.index == info->index) {
                                [presentClients addObject: client];
                                [client loadFromInfoStruct: info];
                                return;
                        }

                PAClientInfo *client = [PAClientInfo createFromInfoStruct: info
                                                                   server: server];
                [publishedClients addObject: client];
                [presentClients addObject: client];
                [server performSelectorOnMainThread: @selector(sendDelegateClientInfoAdded:)
                                         withObject: client
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PAClientInfo *client in publishedClients)
                        if (![presentClients containsObject: client])
                                [d addObject: client];

                for (PAClientInfo *client in d) {
                        [publishedClients removeObject: client];
                        [server performSelectorOnMainThread: @selector(sendDelegateClientInfoRemoved:)
                                                 withObject: client
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) moduleInfoCallback: (const pa_module_info *) info
                        eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PAModuleInfo *module in publishedModules)
                        if (module.index == info->index) {
                                [presentModules addObject: module];
                                [module loadFromInfoStruct: info];
                                return;
                        }

                PAModuleInfo *module = [PAModuleInfo createFromInfoStruct: info
                                                                   server: server];
                [publishedModules addObject: module];
                [presentModules addObject: module];
                [server performSelectorOnMainThread: @selector(sendDelegateModuleInfoAdded:)
                                         withObject: module
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PAModuleInfo *module in publishedModules)
                        if (![presentModules containsObject: module])
                                [d addObject: module];

                for (PAModuleInfo *module in d) {
                        [publishedModules removeObject: module];
                        [server performSelectorOnMainThread: @selector(sendDelegateModuleInfoRemoved:)
                                                 withObject: module
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) sampleInfoCallback: (const pa_sample_info *) info
                        eol: (BOOL) eol
{
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        if (info) {
                for (PASampleInfo *sample in publishedSamples)
                        if (sample.index == info->index) {
                                [presentSamples addObject: sample];
                                [sample loadFromInfoStruct: info];
                                return;
                        }

                PASampleInfo *sample = [PASampleInfo createFromInfoStruct: info
                                                                   server: server];
                [publishedSamples addObject: sample];
                [presentSamples addObject: sample];
                [server performSelectorOnMainThread: @selector(sendDelegateSampleInfoAdded:)
                                         withObject: sample
                                      waitUntilDone: NO];
        }

        if (eol) {
                NSMutableArray *d = [NSMutableArray arrayWithCapacity: 0];

                for (PASampleInfo *sample in publishedSamples)
                        if (![presentSamples containsObject: sample])
                                [d addObject: sample];

                for (PASampleInfo *sample in d) {
                        [publishedSamples removeObject: sample];
                        [server performSelectorOnMainThread: @selector(sendDelegateSampleInfoRemoved:)
                                                 withObject: sample
                                              waitUntilDone: NO];
                }
        }

        [pool drain];
}

- (void) moduleLoadedCallback: (UInt32) index
{
        [presentModules removeAllObjects];
        pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
}

- (void) moduleUnloadedCallback: (BOOL) success
{
        [presentModules removeAllObjects];
        pa_context_get_module_info_list(PAContext, staticModuleInfoCallback, self);
}

- (void) contextDefaultsSetCallback: (BOOL) success
{
        pa_context_get_server_info(PAContext, staticServerInfoCallback, self);
}

- (void) clientNameSetCallback: (BOOL) success
{
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

static void staticClientNameSetCallback(pa_context *c, int success, void *userdata)
{

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

        const char *host = hostName ? [hostName cStringUsingEncoding: NSASCIIStringEncoding] : NULL;

        NSLog(@"host >%s<", host);

        ret = pa_context_connect(PAContext, host,
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
         sinkForPlayback: (NSString *) sink
         sourceForRecord: (NSString *) source
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
                                                           ioProcBufferSize: ioProcBufferSize
                                                            sinkForPlayback: sink
                                                            sourceForRecord: source];
        pa_threaded_mainloop_unlock(PAMainLoop);

        return audio != nil;
}

- (NSString *) sinkForPlayback
{
        return audio ? audio.sinkForPlayback : nil;
}

- (NSString *) sourceForRecord
{
        return audio ? audio.sourceForRecord : nil;
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

- (BOOL) unloadModule: (PAModuleInfo *) module
{
        if (![self isConnected])
                return NO;

        pa_threaded_mainloop_lock(PAMainLoop);
        pa_context_unload_module(PAContext, module.index,
                                 staticModuleUnloadedCallback, self);
        pa_threaded_mainloop_unlock(PAMainLoop);

        return YES;
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

- (void) setClientName: (NSString *) name
{
        if (![self isConnected])
                return;

        pa_threaded_mainloop_lock(PAMainLoop);
        pa_context_set_name(PAContext, [name cStringUsingEncoding: NSASCIIStringEncoding],
                            staticClientNameSetCallback, self);
        pa_threaded_mainloop_unlock(PAMainLoop);
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

        presentCards = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentSinks = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentSinkInputs = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentSources = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentSourceOutputs = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentClients = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentModules = [[NSMutableArray arrayWithCapacity: 0] retain];
        presentSamples = [[NSMutableArray arrayWithCapacity: 0] retain];

        publishedCards = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedSinks = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedSinkInputs = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedSources = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedSourceOutputs = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedClients = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedModules = [[NSMutableArray arrayWithCapacity: 0] retain];
        publishedSamples = [[NSMutableArray arrayWithCapacity: 0] retain];

        return self;
}

- (void) dealloc
{
        if (audio)
                [audio release];

        [presentCards release];
        [presentSinks release];
        [presentSinkInputs release];
        [presentSources release];
        [presentSourceOutputs release];
        [presentClients release];
        [presentModules release];
        [presentSamples release];

        [publishedCards release];
        [publishedSinks release];
        [publishedSinkInputs release];
        [publishedSources release];
        [publishedSourceOutputs release];
        [publishedClients release];
        [publishedModules release];
        [publishedSamples release];

        [serverInfo release];

        [super dealloc];
}

@end
