/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PASampleInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PASampleInfo

@synthesize server;
@synthesize name;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize fileName;
@synthesize duration;
@synthesize bytes;
@synthesize lazy;

@end


@implementation PASampleInfo (internal)

- (id) initWithInfoStruct: (const pa_sample_info *) info
		   server: (PAServerConnection *) s
{
	[super init];

	char tmp[100];
	
	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];
	sampleSpec = [[NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
					 encoding: NSUTF8StringEncoding] retain];
	channelMap = [[NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
					 encoding: NSUTF8StringEncoding] retain];
	fileName = [[NSString stringWithCString: info->filename
				       encoding: NSUTF8StringEncoding] retain];
	duration = info->duration;
	bytes = info->bytes;
	lazy = info->lazy;
	server = s;
	
	return self;
}

+ (PASampleInfo *) createFromInfoStruct: (const pa_sample_info *) info
			       server: (PAServerConnection *) s
{
	return [[PASampleInfo alloc] initWithInfoStruct: info
						 server: s];
}

@end

