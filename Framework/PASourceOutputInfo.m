/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PASourceOutputInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PASourceOutputInfo

@synthesize server;
@synthesize index;
@synthesize bufferUsec;
@synthesize sourceUsec;

@synthesize name;
@synthesize resampleMethod;
@synthesize driver;

@synthesize channelNames;
@synthesize properties;

@synthesize corked;

@end


@implementation PASourceOutputInfo (internal)

- (id) initWithInfoStruct: (const pa_source_output_info *) info
		   server: (PAServerConnection *) s
{
	[super init];

	index = info->index;
	bufferUsec = info->buffer_usec;
	sourceUsec = info->source_usec;
	corked = !!info->corked;
	
	if (info->name)
		name = [[NSString stringWithCString: info->name
					  encoding: NSUTF8StringEncoding] retain];
	
	channelNames = [[PAServerConnection createChannelNamesArray: &info->channel_map] retain];
	driver = [NSString stringWithCString: info->driver
				    encoding: NSUTF8StringEncoding];
	if (info->resample_method)
		resampleMethod = [[NSString stringWithCString: info->resample_method
						    encoding: NSUTF8StringEncoding] retain];
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
	server = s;
	
	return self;
}

+ (PASourceOutputInfo *) createFromInfoStruct: (const pa_source_output_info *) info
			       server: (PAServerConnection *) s
{
	return [[PASourceOutputInfo alloc] initWithInfoStruct: info
						       server: s];
}

@end

