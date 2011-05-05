/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <pulse/pulseaudio.h>

#import "PulseAudio.h"
#import "PAServerConnectionAudio.h"

#import "PACardInfoInternal.h"
#import "PAClientInfoInternal.h"
#import "PAModuleInfoInternal.h"
#import "PASampleInfoInternal.h"
#import "PAServerInfoInternal.h"
#import "PASinkInfoInternal.h"
#import "PASinkInputInfoInternal.h"
#import "PASourceInfoInternal.h"
#import "PASourceOutputInfoInternal.h"

@interface PAServerConnectionImplementation : NSObject
{
	PAServerConnection	*server;
	char			 procName[100];
	
	pa_threaded_mainloop	*PAMainLoop;
	pa_context		*PAContext;
	
	PAServerConnectionAudio *audio;	
	
	PAServerInfo *serverInfo;
	
	NSMutableArray *presentCards;
	NSMutableArray *presentSinks;
	NSMutableArray *presentSinkInputs;
	NSMutableArray *presentSources;
	NSMutableArray *presentSourceOutputs;
	NSMutableArray *presentClients;
	NSMutableArray *presentModules;
	NSMutableArray *presentSamples;

	NSMutableArray *publishedCards;
	NSMutableArray *publishedSinks;
	NSMutableArray *publishedSinkInputs;
	NSMutableArray *publishedSources;
	NSMutableArray *publishedSourceOutputs;
	NSMutableArray *publishedClients;
	NSMutableArray *publishedModules;
	NSMutableArray *publishedSamples;
	
	NSString *lastError;
}

@property (nonatomic, readonly) pa_threaded_mainloop *PAMainLoop;
@property (nonatomic, readonly) pa_context *PAContext;
@property (nonatomic, readonly) NSString *lastError;

@property (nonatomic, readonly) NSArray *presentCards;
@property (nonatomic, readonly) NSArray *presentSinks;
@property (nonatomic, readonly) NSArray *presentSinkInputs;
@property (nonatomic, readonly) NSArray *presentSources;
@property (nonatomic, readonly) NSArray *presentSourceOutputs;
@property (nonatomic, readonly) NSArray *presentClients;
@property (nonatomic, readonly) NSArray *presentModules;
@property (nonatomic, readonly) NSArray *presentSamples;

@property (nonatomic, readonly) PAServerInfo *serverInfo;


- (id) initForServer: (PAServerConnection *) s;

- (void) connectToHost: (NSString *) hostName
		  port: (int) port;
- (void) disconnect;
- (BOOL) isConnected;
- (BOOL) addAudioStreams: (UInt32) nChannels
	      sampleRate: (Float32) sampleRate
	ioProcBufferSize: (UInt32) ioProcBufferSize;

- (NSString *) clientName;
- (void) setClientName: (NSString *) name;

- (BOOL) loadModuleWithName: (NSString *) name
		  arguments: (NSString *) arguments;

- (BOOL) setDefaultSink: (NSString *) name;
- (BOOL) setDefaultSource: (NSString *) name;

- (UInt32) protocolVersion;
- (UInt32) serverProtocolVersion;

- (BOOL) isLocal;
- (NSString *) serverName;
- (void) shutdownServer;

@end
