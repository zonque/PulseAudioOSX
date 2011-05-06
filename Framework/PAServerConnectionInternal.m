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

- (void) setAudioStarted
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnectionAudioStarted:)])
                [delegate PAServerConnectionAudioStarted: self];
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

// server info

- (void) sendDelegateServerInfoChanged: (PAServerInfo *) serverInfo
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:serverInfoChanged:)])
                [delegate PAServerConnection: self
                           serverInfoChanged: serverInfo];
}

// card

- (void) sendDelegateCardInfoAdded: (PACardInfo *) card
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:cardInfoAdded:)])
                [delegate PAServerConnection: self
                               cardInfoAdded: card];
}

- (void) sendDelegateCardInfoRemoved: (PACardInfo *) card
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:cardInfoRemoved:)])
                [delegate PAServerConnection: self
                             cardInfoRemoved: card];
}

- (void) sendDelegateCardInfoChanged: (PACardInfo *) card
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:cardInfoChanged:)])
                [delegate PAServerConnection: self
                             cardInfoChanged: card];
}

// sink

- (void) sendDelegateSinkInfoAdded: (PASinkInfo *) sink
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInfoAdded:)])
                [delegate PAServerConnection: self
                               sinkInfoAdded: sink];
}

- (void) sendDelegateSinkInfoRemoved: (PASinkInfo *) sink
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInfoRemoved:)])
                [delegate PAServerConnection: self
                             sinkInfoRemoved: sink];
}

- (void) sendDelegateSinkInfoChanged: (PASinkInfo *) sink
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInfoChanged:)])
                [delegate PAServerConnection: self
                             sinkInfoChanged: sink];
}

// sink input

- (void) sendDelegateSinkInputInfoAdded: (PASinkInputInfo *) sinkInput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInputInfoAdded:)])
                [delegate PAServerConnection: self
                          sinkInputInfoAdded: sinkInput];
}

- (void) sendDelegateSinkInputInfoRemoved: (PASinkInputInfo *) sinkInput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInputInfoRemoved:)])
                [delegate PAServerConnection: self
                        sinkInputInfoRemoved: sinkInput];
}

- (void) sendDelegateSinkInputInfoChanged: (PASinkInputInfo *) sinkInput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sinkInputInfoChanged:)])
                [delegate PAServerConnection: self
                        sinkInputInfoChanged: sinkInput];
}

// source

- (void) sendDelegateSourceInfoAdded: (PASourceInfo *) source
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceInfoAdded:)])
                [delegate PAServerConnection: self
                             sourceInfoAdded: source];
}

- (void) sendDelegateSourceInfoRemoved: (PASourceInfo *) source
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceInfoRemoved:)])
                [delegate PAServerConnection: self
                           sourceInfoRemoved: source];
}

- (void) sendDelegateSourceInfoChanged: (PASourceInfo *) source
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceInfoChanged:)])
                [delegate PAServerConnection: self
                           sourceInfoChanged: source];
}

// source output

- (void) sendDelegateSourceOutputInfoAdded: (PASourceOutputInfo *) sourceOutput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceOutputInfoAdded:)])
                [delegate PAServerConnection: self
                       sourceOutputInfoAdded: sourceOutput];
}

- (void) sendDelegateSourceOutputInfoRemoved: (PASourceOutputInfo *) sourceOutput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceOutputInfoRemoved:)])
                [delegate PAServerConnection: self
                     sourceOutputInfoRemoved: sourceOutput];
}

- (void) sendDelegateSourceOutputInfoChanged: (PASourceOutputInfo *) sourceOutput
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sourceOutputInfoChanged:)])
                [delegate PAServerConnection: self
                     sourceOutputInfoChanged: sourceOutput];
}

// client

- (void) sendDelegateClientInfoAdded: (PAClientInfo *) client
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:clientInfoAdded:)])
                [delegate PAServerConnection: self
                             clientInfoAdded: client];
}

- (void) sendDelegateClientInfoRemoved: (PAClientInfo *) client
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:clientInfoRemoved:)])
                [delegate PAServerConnection: self
                           clientInfoRemoved: client];
}

- (void) sendDelegateClientInfoChanged: (PAClientInfo *) client
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:clientInfoChanged:)])
                [delegate PAServerConnection: self
                           clientInfoChanged: client];
}

// module

- (void) sendDelegateModuleInfoAdded: (PAModuleInfo *) module
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:moduleInfoAdded:)])
                [delegate PAServerConnection: self
                             moduleInfoAdded: module];
}

- (void) sendDelegateModuleInfoRemoved: (PAModuleInfo *) module
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:moduleInfoRemoved:)])
                [delegate PAServerConnection: self
                           moduleInfoRemoved: module];
}

- (void) sendDelegateModuleInfoChanged: (PAModuleInfo *) module
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:moduleInfoChanged:)])
                [delegate PAServerConnection: self
                           moduleInfoChanged: module];
}

- (BOOL) unloadModule: (PAModuleInfo *) module
{
        return [impl unloadModule: module];
}

// sample

- (void) sendDelegateSampleInfoAdded: (PASampleInfo *) sample
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sampleInfoAdded:)])
                [delegate PAServerConnection: self
                             sampleInfoAdded: sample];
}

- (void) sendDelegateSampleInfoRemoved: (PASampleInfo *) sample
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sampleInfoRemoved:)])
                [delegate PAServerConnection: self
                           sampleInfoRemoved: sample];
}

- (void) sendDelegateSampleInfoChanged: (PASampleInfo *) sample
{
        if (delegate && [delegate respondsToSelector: @selector(PAServerConnection:sampleInfoChanged:)])
                [delegate PAServerConnection: self
                           sampleInfoChanged: sample];
}

@end
