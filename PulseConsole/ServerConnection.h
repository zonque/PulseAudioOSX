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

#import <pulse/pulseaudio.h>
#import <pulse/mainloop.h>
#import <pulse/context.h>

#import <Cocoa/Cocoa.h>

@interface ServerConnection : NSObject {
	pa_mainloop *mainloop;
	pa_context *context;
	char *connectRequest;
	NSThread *thread;
	
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

- (void) connectToServer: (NSString *) server;
- (void) reloadStatistics;
- (void) getServerInfo;

@end
