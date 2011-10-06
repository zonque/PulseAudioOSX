/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>
#import "PASinkInfoInternal.h"
#import "PAServerConnectionInternal.h"
#import "PAServerConnectionImplementation.h"

@implementation PASinkInfo

@synthesize description;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize channelNames;
@synthesize driver;
@synthesize latency;
@synthesize configuredLatency;
@synthesize nVolumeSteps;
@synthesize properties;
@synthesize monitorSourceIndex;

- (UInt32) volume
{
    return self->volume;
}

- (void) setVolume: (UInt32) v
{
    pa_cvolume pav;
    pa_cvolume_init(&pav);
    pa_cvolume_set(&pav, [channelNames count], v);
    
    pa_threaded_mainloop_lock(server.impl.PAMainLoop);
    pa_context_set_sink_volume_by_index(server.impl.PAContext, index, &pav, NULL, NULL);
    pa_threaded_mainloop_unlock(server.impl.PAMainLoop);
    
    volume = v;
}

@end


@implementation PASinkInfo (internal)

- (void) loadFromInfoStruct: (const pa_sink_info *) info
{
    char tmp[0x100];
    
    index = info->index;
    
    latency = info->latency;
    configuredLatency = info->configured_latency;
    nVolumeSteps = info->n_volume_steps;
    volume = pa_cvolume_avg(&info->volume);
    monitorSourceIndex = info->monitor_source;
    
    if (name)
        [name release];
    
    name = [[NSString stringWithCString: info->name
                               encoding: NSUTF8StringEncoding] retain];
    
    if (description)
        [description release];
    
    description = [[NSString stringWithCString: info->description
                                      encoding: NSUTF8StringEncoding] retain];
    if (sampleSpec)
        [sampleSpec release];
    
    sampleSpec = [[NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
                                     encoding: NSUTF8StringEncoding] retain];
    
    if (channelMap)
        [channelMap release];
    
    channelMap = [[NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
                                     encoding: NSUTF8StringEncoding] retain];
    if (driver)
        [driver release];
    
    driver = [[NSString stringWithCString: info->driver
                                 encoding: NSUTF8StringEncoding] retain];
    if (channelNames)
        [channelNames release];
    
    channelNames = [[PAServerConnection createChannelNamesArray: &info->channel_map] retain];
    
    if (properties)
        [properties release];
    
    properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
    
    if (initialized)
        [server performSelectorOnMainThread: @selector(sendDelegateSinkInfoChanged:)
                                 withObject: self
                              waitUntilDone: NO];
    
    initialized = YES;
}

- (id) initWithInfoStruct: (const pa_sink_info *) info
                   server: (PAServerConnection *) s
{
    [super initWithServer: s];
    [self loadFromInfoStruct: info];
    return self;
}

+ (PASinkInfo *) createFromInfoStruct: (const pa_sink_info *) info
                               server: (PAServerConnection *) s
{
    return [[PASinkInfo alloc] initWithInfoStruct: info
                                           server: s];
}

@end

