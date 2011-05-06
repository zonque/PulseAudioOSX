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

#import "ServerDocument.h"
#import "MultipleLinesTextFieldCell.h"

@implementation ServerDocument

#pragma mark ### IBAction ###

- (IBAction) connect: (id) sender
{
        [NSApp endSheet: connectPanel
             returnCode: 1];
}

- (IBAction) cancel: (id) sender
{
        [NSApp endSheet: connectPanel
             returnCode: 0];
}

- (IBAction) noop: (id) sender
{
}

- (void) doubleClicked
{
        [self connect: serverTableView];
}

- (void) sheetDidEnd: (NSWindow *) sheet
          returnCode: (NSInteger) returnCode
         contextInfo: (void *) contextInfo
{

        [connectPanel orderOut: nil];

        if (returnCode == 1) {
                NSInteger selectedRow = [serverTableView selectedRow];

                if (selectedRow < 0)
                        return;

                NSNetService *service = [serverArray objectAtIndex: selectedRow];
                NSString *host = [service hostName];
                [server connectToServer: host];
        } else {
                [[self windowForSheet] performClose: self];
        }
}

#pragma mark ### NSDocument ###

- (void) awakeFromNib
{
        server.delegate = self;
}

- (void) windowControllerDidLoadNib: (NSWindowController *) controller
{
        [super windowControllerDidLoadNib: controller];

        serverArray = [[NSMutableArray arrayWithCapacity: 0] retain];

        discovery = [[PAServiceDiscovery alloc] init];
        discovery.delegate = self;
        [discovery start];

        [serverTableView setDoubleAction: @selector(connect:)];

        [NSApp beginSheet: connectPanel
           modalForWindow: [self windowForSheet]
            modalDelegate: self
           didEndSelector: @selector(sheetDidEnd:returnCode:contextInfo:)
              contextInfo: nil];
}

- (void) dealloc
{
        [discovery release];
        [super dealloc];
}

- (NSString *) displayName
{
        return @"Select Server ...";
}

- (NSString *) windowNibName
{
        return @"Server";
}

#pragma mark ### NSTableViewSource protocol ###

- (void)tableView: (NSTableView *) aTableView
   setObjectValue: obj
   forTableColumn: (NSTableColumn *) col
              row: (NSInteger) rowIndex
{
}

- (id)tableView: (NSTableView *) tableView
objectValueForTableColumn: (NSTableColumn *) col
            row: (NSInteger) rowIndex
{
        if ([[col identifier] isEqualToString: @"image"])
                return [NSImage imageNamed: @"NSBonjour"];

        NSNetService *service = [serverArray objectAtIndex: rowIndex];

        return [NSString stringWithFormat: @"%@|%@", [service name], [service hostName]];
}

- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView
{
        return [serverArray count];
}

#pragma mark ### PAServiceDiscoveryDelegate ###

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
             serverAppeared: (NSNetService *) service
{
        [serverArray addObject: service];
        [serverTableView reloadData];
}

- (void) PAServiceDiscovery: (PAServiceDiscovery *) discovery
          serverDisappeared: (NSNetService *) service
{
        [serverArray removeObject: service];
        [serverTableView reloadData];
}

#pragma mark ### ServerDelegate ###

- (void) serverSignalledEnd: (Server *) server
{
        [[self windowForSheet] performClose: self];
}


@end
