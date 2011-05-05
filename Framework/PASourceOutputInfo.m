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

@synthesize bufferUsec;
@synthesize sourceUsec;

@synthesize resampleMethod;
@synthesize driver;

@synthesize channelNames;
@synthesize properties;

@synthesize corked;

@end


@implementation PASourceOutputInfo (internal)

- (void) loadFromInfoStruct: (const pa_source_output_info *) info
{
	index = info->index;
	bufferUsec = info->buffer_usec;
	sourceUsec = info->source_usec;
	corked = !!info->corked;

	if (name)
		[name release];

	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];

	if (driver)
		[driver release];

	driver = [[NSString stringWithCString: info->driver
				     encoding: NSUTF8StringEncoding] retain];

	if (channelNames)
		[channelNames release];

	channelNames = [[PAServerConnection createChannelNamesArray: &info->channel_map] retain];

	if (resampleMethod) {
		[resampleMethod release];
		resampleMethod = nil;
	}
	
	if (info->resample_method)
		resampleMethod = [[NSString stringWithCString: info->resample_method
						     encoding: NSUTF8StringEncoding] retain];
	if (properties)
		[properties release];

	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
		       
	if (initialized)
		[server performSelectorOnMainThread: @selector(sendDelegateSourceOutputInfoChanged:)
					 withObject: self
				      waitUntilDone: YES];
	
	initialized = YES;
}

- (id) initWithInfoStruct: (const pa_source_output_info *) info
		   server: (PAServerConnection *) s
{
	[super initWithServer: s];
	[self loadFromInfoStruct: info];
	
	return self;
}

+ (PASourceOutputInfo *) createFromInfoStruct: (const pa_source_output_info *) info
			       server: (PAServerConnection *) s
{
	return [[PASourceOutputInfo alloc] initWithInfoStruct: info
						       server: s];
}

@end

