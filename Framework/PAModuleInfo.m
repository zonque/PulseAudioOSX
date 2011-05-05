/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAModuleInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PAModuleInfo

@synthesize argument;
@synthesize useCount;
@synthesize properties;

- (BOOL) unload
{
	return [server unloadModule: self];
}

@end

@implementation PAModuleInfo (internal)

- (void) loadFromInfoStruct: (const pa_module_info *) info
{
	index = info->index;
	useCount = info->n_used;

	if (name)
		[name release];
	
	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];
	if (argument) {
		[argument release];
		argument = nil;
	}
	
	if (info->argument)
		argument = [[NSString stringWithCString: info->argument
					       encoding: NSUTF8StringEncoding] retain];
	if (properties)
		[properties release];
	
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
	
	if (initialized)
		[server performSelectorOnMainThread: @selector(sendDelegateModuleInfoChanged:)
					 withObject: self
				      waitUntilDone: NO];	
}

- (id) initWithInfoStruct: (const pa_module_info *) info
		   server: (PAServerConnection *) s
{
	[super initWithServer: s];
	[self loadFromInfoStruct: info];
	return self;
}

+ (PAModuleInfo *) createFromInfoStruct: (const pa_module_info *) info
			       server: (PAServerConnection *) s
{
	return [[PAModuleInfo alloc] initWithInfoStruct: info
						 server: s];
}


@end

