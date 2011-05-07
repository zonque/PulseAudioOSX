/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>

#import "PulseAudio.h"
#import "PAServerConnectionAudio.h"
#import "PAServerConnectionImplementation.h"

static NSString *frameworkPath = @"/Library/Frameworks/PulseAudio.framework/";

@implementation PAServerConnection

@synthesize delegate;
@synthesize impl;

- (id) init
{
        [super init];

        impl = [[PAServerConnectionImplementation alloc] initForServer: self];

        return self;
}

- (void) dealloc
{
        [impl release];
        [super dealloc];
}

- (void) connectToHost: (NSString *) hostName
                  port: (int) port
{
        [impl connectToHost: hostName
                           port: port];
}

- (void) disconnect
{
        [impl disconnect];
}

- (BOOL) isConnected
{
        return [impl isConnected];
}

- (BOOL) addAudioStreams: (UInt32) nChannels
              sampleRate: (Float32) sampleRate
        ioProcBufferSize: (UInt32) ioProcBufferSize
         sinkForPlayback: (NSString *) sinkForPlayback
         sourceForRecord: (NSString *) sourceForRecord
{
        return [impl addAudioStreams: nChannels
                          sampleRate: sampleRate
                    ioProcBufferSize: ioProcBufferSize
                     sinkForPlayback: sinkForPlayback
                     sourceForRecord: sourceForRecord];
}

- (NSString *) clientName
{
        return [impl clientName];
}

- (void) setClientName: (NSString *) name
{
        [impl setClientName: name];
}

- (BOOL) loadModuleWithName: (NSString *) name
                  arguments: (NSString *) arguments
{
        return [impl loadModuleWithName: name
                              arguments: arguments];
}

- (BOOL) setDefaultSink: (NSString *) name
{
        return [impl setDefaultSink: name];
}

- (BOOL) setDefaultSource: (NSString *) name
{
        return [impl setDefaultSource: name];
}

- (UInt32) protocolVersion
{
        return [impl protocolVersion];
}

- (UInt32) serverProtocolVersion
{
        return [impl serverProtocolVersion];
}

- (BOOL) isLocal
{
        return [impl isLocal];
}

- (NSString *) serverName
{
        return [impl serverName];
}

- (NSString *) lastError
{
        return impl.lastError;
}

- (NSString *) connectedSink
{
        return impl.sinkForPlayback;
}

- (NSString *) connectedSource
{
        return impl.sourceForRecord;
}

- (void) shutdownServer
{
        [impl shutdownServer];
}

- (NSArray *) presentCards
{
        return impl.presentCards;
}

- (NSArray *) presentSinks
{
        return impl.presentSinks;
}

- (NSArray *) presentSinkInputs
{
        return impl.presentSinkInputs;
}

- (NSArray *) presentSources
{
        return impl.presentSources;
}

- (NSArray *) presentSourceOutputs
{
        return impl.presentSourceOutputs;
}

- (NSArray *) presentClients
{
        return impl.presentClients;
}

- (NSArray *) presentModules
{
        return impl.presentModules;
}

- (NSArray *) presentSamples
{
        return impl.presentSamples;
}

- (PAServerInfo *) serverInfo
{
        return impl.serverInfo;
}

+ (NSString *) libraryVersion
{
        return [NSString stringWithCString: pa_get_library_version()
                                  encoding: NSASCIIStringEncoding];
}

+ (NSString *) frameworkPath
{
        return frameworkPath;
}

+ (NSArray *) availableModules
{
        NSError *error = nil;
        NSString *path = [NSString stringWithFormat: @"%@/Resources/lib/modules/", frameworkPath];
        NSArray *list = [[NSFileManager defaultManager] contentsOfDirectoryAtPath: path
                                                                            error: &error];
        CFShow(list);

        if (error)
                return nil;

        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];

        for (NSString *entry in list)
                if ([entry hasSuffix: @".so"])
                        [array addObject: [entry stringByDeletingPathExtension]];

        return array;
}


@end
