/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAClientInfoInternal.h"
#import "PAServerConnectionInternal.h"


@implementation PAClientInfo

@synthesize server;
@synthesize name;
@synthesize driver;
@synthesize properties;

@end


@implementation PAClientInfo (internal)

- (id) initWithInfoStruct: (const pa_client_info *) info
		   server: (PAServerConnection *) s
{
	[super init];

	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];
	driver = [[NSString stringWithCString: info->driver
				     encoding: NSUTF8StringEncoding] retain];
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
	server = s;

	return self;
}

+ (PAClientInfo *) createFromInfoStruct: (const pa_client_info *) info
				 server: (PAServerConnection *) s
{
	return [[PAClientInfo alloc] initWithInfoStruct: info
						 server: s];
}

@end

