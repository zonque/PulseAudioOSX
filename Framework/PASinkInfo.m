/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PASinkInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PASinkInfo

@synthesize index;
@synthesize server;
@synthesize name;
@synthesize description;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize channelNames;
@synthesize driver;
@synthesize latency;
@synthesize configuredLatency;
@synthesize nVolumeSteps;
@synthesize volume;
@synthesize properties;
@synthesize monitorSourceIndex;

@end


@implementation PASinkInfo (internal)

- (id) initWithInfoStruct: (const pa_sink_info *) info
		   server: (PAServerConnection *) s
{
	[super init];

	char tmp[0x100];
	
	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];
	description = [[NSString stringWithCString: info->description
					  encoding: NSUTF8StringEncoding] retain];
	sampleSpec = [[NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
					 encoding: NSUTF8StringEncoding] retain];
	channelMap = [[NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
					 encoding: NSUTF8StringEncoding] retain];
	channelNames = [[PAServerConnection createChannelNamesArray: &info->channel_map] retain];
	driver = [[NSString stringWithCString: info->driver
				     encoding: NSUTF8StringEncoding] retain];
	
	index = info->index;
	latency = info->latency;
	configuredLatency = info->configured_latency;
	nVolumeSteps = info->n_volume_steps;
	volume = pa_cvolume_avg(&info->volume);	
	monitorSourceIndex = info->monitor_source;
	
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
	server = s;
		
	return self;
}

+ (PASinkInfo *) createFromInfoStruct: (const pa_sink_info *) info
			       server: (PAServerConnection *) s
{
	return [[PASinkInfo alloc] initWithInfoStruct: info
					       server: s];
}

@end

