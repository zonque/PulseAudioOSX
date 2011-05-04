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
#import <PulseAudio/PAServiceDiscovery.h>

#import "StreamListView.h"
#import "StreamView.h"
#import "Introspect.h"

@interface Server : NSObject <PAServerConnectionDelegate>
{	
	IBOutlet NSWindow               *window;
	IBOutlet NSTabView              *tabView;
	IBOutlet NSTableView            *statisticsTableView;
	IBOutlet NSTextView             *commandShell;
	IBOutlet NSProgressIndicator    *connectionProgressIndicator;
	IBOutlet NSPopUpButton          *serverSelector;
	IBOutlet NSTextField            *connectStatus;
	
	IBOutlet StreamListView		*playbackStreamListView;
	IBOutlet StreamListView		*recordStreamListView;
	IBOutlet StreamListView		*sinkStreamListView;
	IBOutlet StreamListView		*sourceStreamListView;
	
	IBOutlet Introspect		*introspect;
	
		
	PAServerConnection *connection;
	
	NSMutableDictionary *statisticDict;    
}

- (void) enableGUI: (BOOL) enabled;
- (void) connectToServer: (NSString *) server;

- (IBAction) connectToServerAction: (id) sender;
- (IBAction) reloadStatistics: (id) sender;

@end
