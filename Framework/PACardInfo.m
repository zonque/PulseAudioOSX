/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PACardInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PACardInfo

@synthesize driver;
@synthesize properties;

@end

@implementation PACardInfo (internal)

- (void) loadFromInfoStruct: (const pa_card_info *) info
{
	index = info->index;

	if (name)
		[name release];
	name = [[NSString stringWithCString: info->name
				   encoding: NSUTF8StringEncoding] retain];
	if (driver)
		[driver release];
	driver = [[NSString stringWithCString: info->driver
				      encoding: NSUTF8StringEncoding] retain];
	if (properties)
		[properties release];
	properties = [[PAServerConnection createDictionaryFromProplist: info->proplist] retain];
	
	if (initialized)
		[server performSelectorOnMainThread: @selector(sendDelegateCardInfoChanged:)
					 withObject: self
				      waitUntilDone: YES];
	
	initialized = YES;
}

- (id) initWithInfoStruct: (const pa_card_info *) info
		   server: (PAServerConnection *) s
{
	[super initWithServer: s];
	[self loadFromInfoStruct: info];
	return self;
}

+ (PACardInfo *) createFromInfoStruct: (const pa_card_info *) info
			       server: (PAServerConnection *) s
{
	return [[PACardInfo alloc] initWithInfoStruct: info
					       server: s];
}

@end

