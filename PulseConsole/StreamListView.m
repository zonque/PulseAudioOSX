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

- (void) itemAdded: (PAElementInfo *) info
	      name: (NSString *) name
{
	[self addStreamView: info
		       name: name];
	[self recalcLayout];
}

- (void) itemRemoved: (PAElementInfo *) info
{
	[self removeStreamView: info];
	[self recalcLayout];	
}

- (void) itemChanged: (PAElementInfo *) info
{
	for (StreamView *v in streamViewItems)
		if (v.info == info)
			[v update];
}


#pragma mark # sink

- (void) sinkInfoAdded: (PASinkInfo *) sink
{
	if (streamType == StreamTypeSink)
		[self itemAdded: sink
			   name: [NSString stringWithFormat: @"%@: %@",
				  [sink.properties objectForKey: @"device.string"],
				  sink.name]];
}

- (void) sinkInfoRemoved: (PASinkInfo *) sink
{
	if (streamType == StreamTypeSink)
		[self itemRemoved: sink];	
}

- (void) sinkInfoChanged: (PASinkInfo *) sink
{
	if (streamType == StreamTypeSink)
		[self itemChanged: sink];	
}

#pragma mark # sink input

- (void) sinkInputInfoAdded: (PASinkInputInfo *) input
{
	if (streamType == StreamTypePlayback)
		[self itemAdded: input
			   name: [NSString stringWithFormat: @"%@: %@",
				  [input.properties objectForKey: @"application.name"],
				  input.name]];
}

- (void) sinkInputInfoRemoved: (PASinkInputInfo *) input
{
	if (streamType == StreamTypePlayback)
		[self itemRemoved: input];	
}

- (void) sinkInputInfoChanged: (PASinkInputInfo *) input
{
	if (streamType == StreamTypePlayback)
		[self itemChanged: input];	
}


#pragma mark # source

- (void) sourceInfoAdded: (PASourceInfo *) source
{
	if (streamType == StreamTypeSource)
		[self itemAdded: source
			   name: [NSString stringWithFormat: @"%@: %@",
				  [source.properties objectForKey: @"device.string"],
				  source.name]];
}

- (void) sourceInfoRemoved: (PASourceInfo *) source
{
	if (streamType == StreamTypeSource)
		[self itemRemoved: source];
}

- (void) sourceInfoChanged: (PASourceInfo *) source
{
	if (streamType == StreamTypeSource)
		[self itemChanged: source];	
}


#pragma mark # source output

- (void) sourceOutputInfoAdded: (PASourceOutputInfo *) output
{
	if (streamType == StreamTypeRecording)
		[self itemAdded: output
			   name: [NSString stringWithFormat: @"%@: %@",
				  [output.properties objectForKey: @"application.name"],
				  output.name]];
}

- (void) sourceOutputInfoRemoved: (PASourceOutputInfo *) output
{
	if (streamType == StreamTypeRecording)
		[self itemRemoved: output];
}

- (void) sourceOutputInfoChanged: (PASourceOutputInfo *) output
{
	if (streamType == StreamTypeRecording)
		[self itemChanged: output];
}

@end
