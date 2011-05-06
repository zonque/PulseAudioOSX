/***
 This file is part of PulseConsole

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <PulseAudio/PulseAudio.h>
#import "StreamView.h"
#import "StreamListView.h"

@implementation StreamListView

@synthesize streamType;

- (id) initWithFrame: (NSRect) rect
{
        [super initWithFrame: rect];

        streamViewItems = [[NSMutableArray alloc] initWithCapacity: 0];

        return self;
}

- (void) addStreamView: (id) info
                  name: (NSString *) name;
{
        NSRect frame = [self frame];
        NSScrollView *enclosingScrollView = [self enclosingScrollView];
        float height = 0.0f;

        NSLog(@"DEBUG: %s name >%@<\n", __func__, name);

        for (NSView *v in streamViewItems)
                height += [v frame].size.height;

        StreamView *view = [[StreamView alloc] initWithFrame: NSMakeRect(0.0, height, frame.size.width, 70.0)
                                                        type: streamType
                                                        name: name
                                                        info: info];

        // need to encapsulate in a NSDictionary
        [self addSubview: view];
        [streamViewItems addObject: view];

        height += [view frame].size.height;

        [self setFrameSize: NSMakeSize(frame.size.width, height)];

        [enclosingScrollView setDocumentView: self];
        [enclosingScrollView setNeedsDisplay: YES];
}

- (void) removeStreamView: (id) info
{
        for (StreamView *v in streamViewItems)
                if (v.info == info) {
                        [streamViewItems removeObject: v];
                        break;
                }

        [self recalcLayout];
}

- (void) recalcLayout
{
        float y = 0.0f;

        for (NSView *v in streamViewItems) {
                NSRect rect = [v frame];
                rect.origin.y = y;
                [v setFrameOrigin: rect.origin];
                y += rect.size.height;
        }

        [super setFrameSize: NSMakeSize([self frame].size.width, y)];
        [self setNeedsDisplay: YES];
}

- (void) setFrameSize: (NSSize) size
{
        for (NSView *v in streamViewItems)
                [v setFrameSize: NSMakeSize(size.width, [v frame].size.height)];

        [self recalcLayout];
}

- (BOOL) isFlipped
{
        return YES;
}

- (void) removeAllStreams
{
        [streamViewItems removeAllObjects];
        [self recalcLayout];
}

- (void) addItem: (PAElementInfo *) info
            name: (NSString *) name
{
        [self addStreamView: info
                       name: name];
        [self recalcLayout];
}

- (void) removeItem: (PAElementInfo *) info
{
        [self removeStreamView: info];
        [self recalcLayout];
}

- (void) updateItem: (PAElementInfo *) info
{
        for (StreamView *v in streamViewItems)
                if (v.info == info)
                        [v update];
}

@end
