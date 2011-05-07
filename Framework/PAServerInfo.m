/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import "PAServerInfoInternal.h"
#import "PAServerConnectionInternal.h"


@implementation PAServerInfo

@synthesize userName;
@synthesize hostName;
@synthesize serverName;
@synthesize version;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize defaultSinkName;
@synthesize defaultSourceName;
@synthesize cookie;

@end


@implementation PAServerInfo (internal)

- (void) setFromInfoStruct: (const pa_server_info *) info
                    server: (PAServerConnection *) s
{
        char tmp[0x100];

        userName = [[NSString stringWithCString: info->user_name
                                       encoding: NSUTF8StringEncoding] retain];
        hostName = [[NSString stringWithCString: info->host_name
                                       encoding: NSUTF8StringEncoding] retain];
        serverName = [[NSString stringWithCString: info->server_name
                                         encoding: NSUTF8StringEncoding] retain];
        version = [[NSString stringWithCString: info->server_version
                                      encoding: NSUTF8StringEncoding] retain];
        sampleSpec = [[NSString stringWithCString: pa_sample_spec_snprint(tmp, sizeof(tmp), &info->sample_spec)
                                         encoding: NSUTF8StringEncoding] retain];
        channelMap = [[NSString stringWithCString: pa_channel_map_snprint(tmp, sizeof(tmp), &info->channel_map)
                                         encoding: NSUTF8StringEncoding] retain];
        defaultSinkName = [[NSString stringWithCString: info->default_sink_name
                                              encoding: NSUTF8StringEncoding] retain];
        defaultSourceName = [[NSString stringWithCString: info->default_source_name
                                                encoding: NSUTF8StringEncoding] retain];
        cookie = info->cookie;

        server = s;

        name = @"Server Information";
}

@end

