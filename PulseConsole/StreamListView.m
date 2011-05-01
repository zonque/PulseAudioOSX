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
	NSEnumerator *enumerator = [streamViewItems objectEnumerator];
	NSView *object;
	float height = 0.0f;
	
	NSLog(@"DEBUG: %s name >%@<\n", __func__, name);
	
	while (object = [enumerator nextObject])
		height += [object frame].size.height;

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

- (void) recalcLayout
{
	NSEnumerator *enumerator = [streamViewItems objectEnumerator];
	NSView *object;
	float y = 0.0f;

	while (object = [enumerator nextObject]) {
		NSRect rect = [object frame];
		rect.origin.y = y;
		[object setFrameOrigin: rect.origin];
		y += rect.size.height;
	}
	
	[super setFrameSize: NSMakeSize([self frame].size.width, y)];
	[self setNeedsDisplay: YES];
}

- (void) setFrameSize: (NSSize) size
{
	NSEnumerator *enumerator = [streamViewItems objectEnumerator];
	NSView *object;
	
	while (object = [enumerator nextObject])
		[object setFrameSize: NSMakeSize(size.width, [object frame].size.height)];

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

- (void) sinksChanged: (NSArray *) sinks
{
	if (streamType != StreamTypeSink)
		return;

	for (PASinkInfo *sink in sinks)
		[self addStreamView: sink
			       name: sink.name];
}

- (void) sourcesChanged: (NSArray *) sources
{
	if (streamType != StreamTypeSource)
		return;
	
	for (PASourceInfo *source in sources)
		[self addStreamView: source
			       name: source.name];	
}


@end
