/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import "PASourceInfoInternal.h"
#import "PAServerConnectionInternal.h"


@implementation PASourceInfo

@synthesize description;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize channelNames;
@synthesize driver;
@synthesize latency;
@synthesize configuredLatency;
@synthesize properties;

@end


@implementation PASourceInfo (internal)

- (void) loadFromInfoStruct: (const pa_source_info *) info
{
    char tmp[0x100];
    
    index = info->index;
    latency = info->latency;
    configuredLatency = info->configured_latency;
    
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
    
    if (properties)
        [properties release];
    properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
    
    if (initialized)
        [server performSelectorOnMainThread: @selector(sendDelegateSourceInfoChanged:)
                                 withObject: self
                              waitUntilDone: NO];
    
    initialized = YES;
}

- (id) initWithInfoStruct: (const pa_source_info *) info
                   server: (PAServerConnection *) s
{
    [super initWithServer: s];
    [self loadFromInfoStruct: info];
    return self;
}

+ (PASourceInfo *) createFromInfoStruct: (const pa_source_info *) info
                                 server: (PAServerConnection *) s
{
    return [[PASourceInfo alloc] initWithInfoStruct: info
                                             server: s];
}

@end

