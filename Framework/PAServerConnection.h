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
@class PASourceInfo;

@protocol PAServerConnectionDelegate
@optional

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection;
- (void) PAServerConnectionFailed: (PAServerConnection *) connection;
- (void) PAServerConnectionEnded: (PAServerConnection *) connection;

- (void) PAServerConnection: (PAServerConnection *) connection
	  serverInfoChanged: (PAServerInfo *) serverInfo;
- (void) PAServerConnection: (PAServerConnection *) connection
	       cardsChanged: (NSArray *) cards;
- (void) PAServerConnection: (PAServerConnection *) connection
	       sinksChanged: (NSArray *) sinks;
- (void) PAServerConnection: (PAServerConnection *) connection
	     sourcesChanged: (NSArray *) sources;
- (void) PAServerConnection: (PAServerConnection *) connection
	     clientsChanged: (NSArray *) clients;
- (void) PAServerConnection: (PAServerConnection *) connection
	     modulesChanged: (NSArray *) modules;
- (void) PAServerConnection: (PAServerConnection *) connection
	     samplesChanged: (NSArray *) samples;

- (UInt32) PAServerConnection: (PAServerConnection *) connection
	      hasPlaybackData: (Byte *) playbackData
		   recordData: (const Byte *) recordData
		     byteSize: (UInt32) byteSize;

@end

@interface PAServerConnection : NSObject
{
	NSObject <PAServerConnectionDelegate> *delegate;
	
	PAServerInfo *serverInfo;
	
	NSMutableArray *cards;
	NSMutableArray *sinks;
	NSMutableArray *sources;
	NSMutableArray *clients;
	NSMutableArray *modules;
	NSMutableArray *samples;
}

@property (nonatomic, assign) NSObject <PAServerConnectionDelegate> *delegate;
@property (nonatomic, readonly) PAServerInfo *serverInfo;
@property (nonatomic, readonly) NSArray *cards;
@property (nonatomic, readonly) NSArray *sinks;
@property (nonatomic, readonly) NSArray *sources;
@property (nonatomic, readonly) NSArray *clients;
@property (nonatomic, readonly) NSArray *modules;
@property (nonatomic, readonly) NSArray *samples;

- (void) connectToHost: (NSString *) hostName
		  port: (int) port;
- (BOOL) isConnected;
- (BOOL) addAudioStreams: (UInt32) nChannels
	      sampleRate: (Float32) sampleRate
	ioProcBufferSize: (UInt32) ioProcBufferSize;

- (NSString *) clientName;

+ (NSString *) pulseAudioLibraryVersion;


@end
