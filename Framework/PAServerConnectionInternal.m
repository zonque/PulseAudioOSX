/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PACardInfoInternal.h"
#import "PAClientInfoInternal.h"
#import "PAModuleInfoInternal.h"
#import "PASampleInfoInternal.h"
#import "PAServerInfoInternal.h"
#import "PASinkInfoInternal.h"
#import "PASinkInputInfoInternal.h"
#import "PASourceInfoInternal.h"
#import "PASourceOutputInfoInternal.h"
#import "PAServerConnectionInternal.h"

@implementation PAServerConnection (internal)

+ (NSDictionary *) createDictionaryFromProplist: (pa_proplist *) plist
{	
	if (!plist)
		return nil;
	
	NSMutableDictionary * dict = [NSMutableDictionary dictionaryWithCapacity: 0];
	void *state = NULL;
	
	do {
		const char *key = pa_proplist_iterate(plist, &state);
		if (!key)
			break;
		
		[dict setValue: [NSString stringWithCString: pa_proplist_gets(plist, key)
						   encoding: NSUTF8StringEncoding]
			forKey: [NSString stringWithCString: key
						   encoding: NSUTF8StringEncoding]];
	} while (state);
	
	return dict;
}

+ (NSArray *) createChannelNamesArray: (const pa_channel_map *) map
{
	NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
	
	for (UInt32 i = 0; i < map->channels; i++)
		[array addObject: [NSString stringWithCString: pa_channel_position_to_string(map->map[i])
						     encoding: NSASCIIStringEncoding]];
	
	return array;
}

- (void) sendDelegateConnectionEstablished
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionEstablished:)])
		[delegate PAServerConnectionEstablished: self];
}

- (void) sendDelegateConnectionEnded
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionEnded:)])
		[delegate PAServerConnectionEnded: self];
}

- (void) sendDelegateConnectionFailed
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionFailed:)])
		[delegate PAServerConnectionFailed: self];
}

- (void) sendDelegateServerInfoChanged: (PAServerInfo *) serverInfo
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:serverInfoChanged:)])
		[delegate PAServerConnection: self
			   serverInfoChanged: serverInfo];
}

- (void) sendDelegateCardsChanged: (NSArray *) cards
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:cardsChanged:)])
		[delegate PAServerConnection: self
				cardsChanged: cards];	
}

- (void) sendDelegateSinksChanged: (NSArray *) sinks
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinksChanged:)])
		[delegate PAServerConnection: self
				sinksChanged: sinks];
}

- (void) sendDelegateSinkInputsChanged: (NSArray *) sinkInputs
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInputsChanged:)])
		[delegate PAServerConnection: self
			   sinkInputsChanged: sinkInputs];
}

- (void) sendDelegateSourcesChanged: (NSArray *) sources
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourcesChanged:)])
		[delegate PAServerConnection: self
			      sourcesChanged: sources];
}

- (void) sendDelegateSourceOutputsChanged: (NSArray *) sourceOutputs
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceOutputsChanged:)])
		[delegate PAServerConnection: self
			sourceOutputsChanged: sourceOutputs];
}

- (void) sendDelegateClientsChanged: (NSArray *) clients
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:clientsChanged:)])
		[delegate PAServerConnection: self
			      clientsChanged: clients];
}

- (void) sendDelegateModulesChanged: (NSArray *) modules
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:modulesChanged:)])
		[delegate PAServerConnection: self
			      modulesChanged: modules];
}

- (void) sendDelegateSamplesChanged: (NSArray *) samples
{
	if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:samplesChanged:)])
		[delegate PAServerConnection: self
			      samplesChanged: samples];
}

@end
