/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>
#import "PASinkInputInfoInternal.h"
#import "PAServerConnectionInternal.h"
#import "PAServerConnectionImplementation.h"

@implementation PASinkInputInfo

@synthesize index;
@synthesize bufferUsec;
@synthesize sinkUsec;

@synthesize name;
@synthesize resampleMethod;
@synthesize driver;

@synthesize channelNames;
@synthesize properties;

@synthesize volumeWriteable;

/*
static void staticMuteSetCallback(pa_context *context, int success, void *userdata)
{
}
 */

- (BOOL) muted
{
	return self->muted;
}

- (void) setMuted: (BOOL) m
{
	pa_threaded_mainloop_lock(server.impl.PAMainLoop);
	pa_context_set_sink_input_mute(server.impl.PAContext, index, m, NULL, NULL);
	pa_threaded_mainloop_unlock(server.impl.PAMainLoop);

	muted = m;
}

- (UInt32) volume
{
	return self->volume;
}

- (void) setVolume: (UInt32) v
{
	NSLog(@"%s() v = %d", __func__, v);
	
	pa_cvolume pav;
	pa_cvolume_init(&pav);
	pa_cvolume_set(&pav, [channelNames count], v);
	
	pa_threaded_mainloop_lock(server.impl.PAMainLoop);
	pa_context_set_sink_input_volume(server.impl.PAContext, index, &pav, NULL, NULL);
	pa_threaded_mainloop_unlock(server.impl.PAMainLoop);

	volume = v;
}

@end


@implementation PASinkInputInfo (internal)

- (id) initWithInfoStruct: (const pa_sink_input_info *) info
		   server: (PAServerConnection *) s
{
	[super init];
	
	index = info->index;
	bufferUsec = info->buffer_usec;
	sinkUsec = info->sink_usec;
	muted = !!info->mute;
	volumeWriteable = !!info->volume_writable;
	
	if (info->name)
		name = [[NSString stringWithCString: info->name
					   encoding: NSUTF8StringEncoding] retain];
	
	channelNames = [[PAServerConnection createChannelNamesArray: &info->channel_map] retain];
	driver = [[NSString stringWithCString: info->driver
				     encoding: NSUTF8StringEncoding] retain];
	if (info->resample_method)
		resampleMethod = [[NSString stringWithCString: info->resample_method
						     encoding: NSUTF8StringEncoding] retain];
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];	
	server = s;
	
	return self;
}

+ (PASinkInputInfo *) createFromInfoStruct: (const pa_sink_input_info *) info
				    server: (PAServerConnection *) s
{
	return [[PASinkInputInfo alloc] initWithInfoStruct: info
						    server: s];
}

@end

