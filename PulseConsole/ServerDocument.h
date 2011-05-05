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
#import <PulseAudio/PAServiceDiscovery.h>
#import "Server.h"

@interface ServerDocument : NSDocument <PAServiceDiscoveryDelegate,
					NSTableViewDelegate,
					NSTableViewDataSource,
					ServerDelegate>
{
	IBOutlet NSTableView *serverTableView;
	IBOutlet NSWindow *connectPanel;
	IBOutlet Server *server;

	NSMutableArray *serverArray;
	PAServiceDiscovery *discovery;
}

- (IBAction) connect: (id) sender;
- (IBAction) cancel: (id) sender;
- (IBAction) noop: (id) sender;

@end
