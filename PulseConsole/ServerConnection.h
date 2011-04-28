/***
 This file is part of PulseConsole.
 
 Copyright 2009-2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudio is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 
 PulseAudio is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <Cocoa/Cocoa.h>
#import <PulseAudio/PulseAudio.h>

@interface ServerConnection : NSObject<PAServerConnectionDelegate>
{
	PAServerConnection *connection;
	
	NSMutableDictionary *statisticDict;    
	NSMutableDictionary *serverinfo;
	NSMutableArray *outlineToplevel;
	NSMutableArray *cards;
	NSMutableArray *sinks;
	NSMutableArray *sources;
	NSMutableArray *clients;
	NSMutableArray *modules;
	NSMutableArray *samplecache;
}

@property (nonatomic, retain) NSArray *outlineToplevel;
@property (nonatomic, retain) NSDictionary *statisticDict;
@property (nonatomic, retain) NSDictionary *serverinfo;
@property (nonatomic, retain) NSArray *sinks;

- (void) connectToServer: (NSString *) server;
- (void) reloadStatistics;
- (void) getServerInfo;

#pragma mark ### PAServerConnectionDelegate ###

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

@end
