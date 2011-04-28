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

#import "StreamView.h"
#import "StreamListView.h"

#define COLLAPSED_HEIGHT	(60.0f)
#define LEFT_MARGIN		(40.0f)
#define RIGHT_MARGIN		(20.0f)

@implementation StreamView

#pragma mark === NSView subclass ===

- (id) initWithFrame: (NSRect) rect
		type: (NSInteger) _type
		name: (NSString *) _name
		info: (const void *) _info
{
	rect.size.height = COLLAPSED_HEIGHT;
	type = _type;
	pa_info = _info;
	
	[super initWithFrame: rect];
	[self setFrame: rect];
  	
	masterSlider = [[NSSlider alloc] initWithFrame: NSMakeRect(LEFT_MARGIN,
								   10.0,
								   rect.size.width - LEFT_MARGIN - RIGHT_MARGIN,
								   20.0)];
	[masterSlider setContinuous: YES];
	[masterSlider setTag: -1];
	[masterSlider setAction: @selector(sliderMovedCallback:)];
	[masterSlider setTarget: self];
	[self addSubview: masterSlider];
	
	label = [[NSTextField alloc] initWithFrame: NSMakeRect(LEFT_MARGIN,
							       30.0,
							       rect.size.width - LEFT_MARGIN - RIGHT_MARGIN,
							       20.0)];
	[label setEditable: NO];
	[label setDrawsBackground: NO];
	[label setBordered: NO];
	[[label cell] setFont: [NSFont labelFontOfSize: 11.0]];
	[label setStringValue: _name];
	[label sizeToFit];
	[self addSubview: label];
	
	expandButton = [[NSButton alloc] initWithFrame: NSMakeRect(10.0, 10.0, 20.0, 20.0)];
	[expandButton setButtonType: NSOnOffButton];
	[expandButton setBezelStyle: NSDisclosureBezelStyle];
	[expandButton setTitle: @""];
	[expandButton setAction: @selector(expandCallback:)];
	[expandButton setTarget: self];
	[self addSubview: expandButton];
	
	meterView = [[MeterView alloc] initWithFrame: NSMakeRect(LEFT_MARGIN,
								 50.0,
								 rect.size.width - LEFT_MARGIN - RIGHT_MARGIN,
								 20.0)];
	[meterView setNumChannels: 2];
	[self addSubview: meterView];
	
	// use NSBox as separator line
	box = [[NSBox alloc] initWithFrame: NSMakeRect(0, rect.size.height - 1.0, rect.size.width, 1.0)];
	[box setBoxType: NSBoxSeparator];
	[self addSubview: box];
	
	expandedHeight = COLLAPSED_HEIGHT;
	expandedHeight += 20.0;
	
	lockButton = [[NSButton alloc] initWithFrame: NSMakeRect(LEFT_MARGIN,
								 70.0,
								 rect.size.width - LEFT_MARGIN - RIGHT_MARGIN,
								 20.0)];
	[lockButton setButtonType: NSSwitchButton];
	[lockButton setTitle: @"Lock"];
	[lockButton setAction: @selector(lockCallback:)];
	[lockButton setTarget: self];
	[[lockButton cell] setControlSize: NSSmallControlSize];
	[self addSubview: lockButton];
	
	channelSliders = [[NSMutableArray alloc] initWithCapacity: 0];
	channelLabels = [[NSMutableArray alloc] initWithCapacity: 0];
	int i;
	longestChannelLabel = 0.0f;
	
	/*
	pa_channel_map *map;
	
	switch (type) {
		case StreamTypeSink:
			map = &((pa_sink_info *) pa_info)->channel_map;
			break;
		case StreamTypeSource:
			map = &((pa_source_info *) pa_info)->channel_map;
			break;
		case StreamTypeRecording:
			break;
		case StreamTypePlayback:
			break;
	};
	
	for (i = 0; i < map->channels; i++) {
		NSTextField *l = [[NSTextField alloc] initWithFrame: NSMakeRect(LEFT_MARGIN,
										93.0 + (i * 20),
										rect.size.width - LEFT_MARGIN - RIGHT_MARGIN,
										20.0)];
		[l setEditable: NO];
		[l setBordered: NO];
		[l setDrawsBackground: NO];
//		[l setStringValue: [NSString stringWithCString: pa_channel_position_to_string(map->map[i])
//						      encoding: NSUTF8StringEncoding]];
		[[l cell] setFont: [NSFont labelFontOfSize: 10.0]];
		[l sizeToFit];
		[self addSubview: l];
		
		if (longestChannelLabel < [l frame].size.width)
			longestChannelLabel = [l frame].size.width;
	}

	for (i = 0; i < map->channels; i++) {
		NSSlider *slider = [[NSSlider alloc] initWithFrame: NSMakeRect(LEFT_MARGIN + longestChannelLabel + 10.0,
									       90.0 + (i * 20.0),
									       rect.size.width - LEFT_MARGIN - RIGHT_MARGIN - longestChannelLabel - 10.0,
									       20.0)];
		[[slider cell] setControlSize: NSSmallControlSize];
		[slider setContinuous: YES];
		[slider setTag: i];
		[slider setAction: @selector(sliderMovedCallback:)];
		[slider setTarget: self];
		[self addSubview: slider];
		[channelSliders addObject: slider];
		
		expandedHeight += 20.0;
	}
	 */

	switch (type) {
		case StreamTypeSink:
		case StreamTypePlayback:
		case StreamTypeRecording:
			expandedHeight += 20.0;
			endpointSelect = [[NSPopUpButton alloc] initWithFrame: NSMakeRect(rect.size.width - 100.0 - RIGHT_MARGIN,
											  expandedHeight,
											  100.0,
											  20.0)];
			[endpointSelect addItemWithTitle: @"schalali"];
			[endpointSelect addItemWithTitle: @"xxxalala"];
			[[endpointSelect cell] setControlSize: NSSmallControlSize];
			[self addSubview: endpointSelect];
			expandedHeight += 20.0;
			break;
	}
	
	expandedHeight += 20.0;
	
	return self;
}

- (void) setFrameSize: (NSSize) size
{
	[box setFrameSize: NSMakeSize(size.width, [box frame].size.height)];
	[masterSlider setFrameSize: NSMakeSize(size.width - LEFT_MARGIN - RIGHT_MARGIN,
					       [masterSlider frame].size.height)];
	[label setFrameSize: NSMakeSize(size.width - LEFT_MARGIN - RIGHT_MARGIN,
					[label frame].size.height)];
	[meterView setFrameSize: NSMakeSize(size.width - LEFT_MARGIN - RIGHT_MARGIN,
					    [meterView frame].size.height)];
	
	NSEnumerator *enumerator = [channelSliders objectEnumerator];
	NSView *object;
	
	while (object = [enumerator nextObject])
		[object setFrameSize: NSMakeSize(size.width - LEFT_MARGIN - RIGHT_MARGIN - longestChannelLabel - 10.0f,
						 [object frame].size.height)];
	
	[endpointSelect setFrameOrigin: NSMakePoint(size.width - [endpointSelect frame].size.width - RIGHT_MARGIN,
						    [endpointSelect frame].origin.y)];
	
	[super setFrameSize: size];
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) drawRect: (NSRect) rect
{
}

#pragma mark === local GUI callbacks ===

- (void) lockCallback: (id) sender
{
	NSEnumerator *enumerator = [channelSliders objectEnumerator];
	id object;
	
	while (object = [enumerator nextObject])
		[object setEnabled: [sender state] == NSOffState];
}

- (void) expandCallback: (id) sender
{
	NSSize size = [self frame].size;
	StreamListView *listView = (StreamListView *) [self superview];
	
	if ([sender state] == NSOnState)
		size.height = expandedHeight;
	else
		size.height = COLLAPSED_HEIGHT;		
	
	[box setFrameOrigin: NSMakePoint(0.0, size.height - 1.0)];
	
	[self setFrameSize: size];
	[listView recalcLayout];
}

- (void) sliderMovedCallback: (id) sender
{
}

@end
