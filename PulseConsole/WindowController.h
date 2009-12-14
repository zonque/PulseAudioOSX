/***
  This file is part of PulseConsole.

  Copyright 2009 Daniel Mack <daniel@caiaq.de>

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

#import "BonjourListener.h"

@interface WindowController : NSObject {

    IBOutlet NSWindow               *window;
    IBOutlet NSOutlineView          *outlineView;
    IBOutlet NSTableView            *parameterTableView;
    IBOutlet NSTableView            *propertyTableView;
    IBOutlet NSButton               *activeButton;
    IBOutlet NSProgressIndicator    *connectionProgressIndicator;
    IBOutlet NSPopUpButton          *serverSelector;
    IBOutlet NSTextField            *connectStatus;
    IBOutlet NSTabView              *tabView;
    IBOutlet NSTableView            *statisticsTableView;

    pa_mainloop *mainloop;
    pa_context *context;
    char *connectRequest;

    NSMutableDictionary *statisticDict;    
    NSMutableDictionary *serverinfo;
    NSMutableArray *outlineToplevel;
    NSMutableArray *cards;
    NSMutableArray *sinks;
    NSMutableArray *sources;
    NSMutableArray *clients;
    NSMutableArray *modules;
    NSMutableArray *samplecache;

    NSDictionary *activeItem;
    
    BonjourListener *listener;
}

- (void) awakeFromNib;
- (void) contextChangeCallback;
- (void) getServerInfo;
- (void) enableGUI;
- (void) stopProgressIndicator;
- (void) connectToServer: (NSString *) server;
- (NSDictionary *) createDictionaryFromProplist: (pa_proplist *) plist;

- (void) statCallback: (const pa_stat_info *) info;
- (void) contextSubscriptionEventCallback: (enum pa_subscription_event_type) type index: (UInt) index;
- (void) contextEventCallback: (const char *) name propList: (pa_proplist *) propList;
- (void) addServerInfo: (const pa_server_info *) info;
- (void) addCardInfo: (const pa_card_info *) info;
- (void) addSinkInfo: (const pa_sink_info *) info;
- (void) addSourceInfo: (const pa_source_info *) info;
- (void) addModuleInfo: (const pa_module_info *) info;
- (void) addClientInfo: (const pa_client_info *) info;
- (void) addSampleInfo: (const pa_sample_info *) info;

- (IBAction) connectToServerAction: (id) sender;
- (IBAction) displayAbout: (id) sender;
- (IBAction) reloadStatistics: (id) sender;

@end
