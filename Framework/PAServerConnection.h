/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

@class PACardInfo;
@class PAClientInfo;
@class PAModuleInfo;
@class PASampleInfo;
@class PAServerConnection;
@class PAServerInfo;
@class PASinkInfo;
@class PASinkInputInfo;
@class PASourceInfo;
@class PASourceOutputInfo;
@class PAServerConnectionImplementation;

@protocol PAServerConnectionDelegate
@optional

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection;
- (void) PAServerConnectionFailed: (PAServerConnection *) connection;
- (void) PAServerConnectionEnded: (PAServerConnection *) connection;

- (void) PAServerConnection: (PAServerConnection *) connection
	  serverInfoChanged: (PAServerInfo *) serverInfo;

#pragma mark # card

- (void) PAServerConnection: (PAServerConnection *) connection
	      cardInfoAdded: (PACardInfo *) card;
- (void) PAServerConnection: (PAServerConnection *) connection
	    cardInfoRemoved: (PACardInfo *) card;
- (void) PAServerConnection: (PAServerConnection *) connection
	    cardInfoChanged: (PACardInfo *) card;

#pragma mark # sink

- (void) PAServerConnection: (PAServerConnection *) connection
	      sinkInfoAdded: (PASinkInfo *) sink;
- (void) PAServerConnection: (PAServerConnection *) connection
	    sinkInfoRemoved: (PASinkInfo *) sink;
- (void) PAServerConnection: (PAServerConnection *) connection
	    sinkInfoChanged: (PASinkInfo *) sink;

#pragma mark # sink input

- (void) PAServerConnection: (PAServerConnection *) connection
	 sinkInputInfoAdded: (PASinkInputInfo *) input;
- (void) PAServerConnection: (PAServerConnection *) connection
       sinkInputInfoRemoved: (PASinkInputInfo *) input;
- (void) PAServerConnection: (PAServerConnection *) connection
       sinkInputInfoChanged: (PASinkInputInfo *) input;

#pragma mark # source

- (void) PAServerConnection: (PAServerConnection *) connection
	    sourceInfoAdded: (PASourceInfo *) source;
- (void) PAServerConnection: (PAServerConnection *) connection
	  sourceInfoRemoved: (PASourceInfo *) source;
- (void) PAServerConnection: (PAServerConnection *) connection
	  sourceInfoChanged: (PASourceInfo *) source;

#pragma mark # source output

- (void) PAServerConnection: (PAServerConnection *) connection	
      sourceOutputInfoAdded: (PASourceOutputInfo *) output;
- (void) PAServerConnection: (PAServerConnection *) connection
    sourceOutputInfoRemoved: (PASourceOutputInfo *) output;
- (void) PAServerConnection: (PAServerConnection *) connection
    sourceOutputInfoChanged: (PASourceOutputInfo *) output;

#pragma mark # client

- (void) PAServerConnection: (PAServerConnection *) connection
	    clientInfoAdded: (PAClientInfo *) client;
- (void) PAServerConnection: (PAServerConnection *) connection
	  clientInfoRemoved: (PAClientInfo *) client;
- (void) PAServerConnection: (PAServerConnection *) connection
	  clientInfoChanged: (PAClientInfo *) client;

#pragma mark # module

- (void) PAServerConnection: (PAServerConnection *) connection
	    moduleInfoAdded: (PAModuleInfo *) module;
- (void) PAServerConnection: (PAServerConnection *) connection
	  moduleInfoRemoved: (PAModuleInfo *) module;
- (void) PAServerConnection: (PAServerConnection *) connection
	  moduleInfoChanged: (PAModuleInfo *) module;

#pragma mark # sample

- (void) PAServerConnection: (PAServerConnection *) connection
	    sampleInfoAdded: (PASampleInfo *) sample;
- (void) PAServerConnection: (PAServerConnection *) connection
	  sampleInfoRemoved: (PASampleInfo *) sample;
- (void) PAServerConnection: (PAServerConnection *) connection
	  sampleInfoChanged: (PASampleInfo *) sample;

#pragma mark # audio

- (void) PAServerConnectionAudioStarted: (PAServerConnection *) connection;
- (UInt32) PAServerConnection: (PAServerConnection *) connection
	      hasPlaybackData: (Byte *) playbackData
		   recordData: (const Byte *) recordData
		     byteSize: (UInt32) byteSize;

@end

@interface PAServerConnection : NSObject
{
	PAServerConnectionImplementation *impl;
	NSObject <PAServerConnectionDelegate> *delegate;
}

@property (nonatomic, assign) NSObject <PAServerConnectionDelegate> *delegate;
@property (nonatomic, readonly) PAServerConnectionImplementation *impl;

- (NSArray *) presentCards;
- (NSArray *) presentSinks;
- (NSArray *) presentSinkInputs;
- (NSArray *) presentSources;
- (NSArray *) presentSourceOutputs;
- (NSArray *) presentClients;
- (NSArray *) presentModules;
- (NSArray *) presentSamples;
- (PAServerInfo *) serverInfo;

- (void) connectToHost: (NSString *) hostName
		  port: (int) port;
- (void) disconnect;
- (BOOL) isConnected;
- (BOOL) addAudioStreams: (UInt32) nChannels
	      sampleRate: (Float32) sampleRate
	ioProcBufferSize: (UInt32) ioProcBufferSize
	 sinkForPlayback: (NSString *) sinkForPlayback
	 sourceForRecord: (NSString *) sourceForRecord;

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

- (NSString *) lastError;

- (NSString *) connectedSink;
- (NSString *) connectedSource;

+ (NSString *) libraryVersion;
+ (NSString *) frameworkPath;
+ (NSArray *) availableModules;

@end
