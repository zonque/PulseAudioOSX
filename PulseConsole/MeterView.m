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

#import "MeterView.h"


@implementation MeterView

- (id) initWithFrame: (NSRect) frame
{
    self = [super initWithFrame:frame];

    return self;
}

- (void) drawRect: (NSRect) dirtyRect
{
//	[[NSColor blackColor] set];
//	[NSBezierPath fillRect: dirtyRect];
}

- (void) setNumChannels: (NSInteger) _numChannels
{
	numChannels = _numChannels;

	if (currVal) {
		free(currVal);
		currVal = NULL;
	}
	
	if (lastPeak) {
		free(lastPeak);
		lastPeak = NULL;
	}

	lastPeak = malloc(sizeof(*lastPeak) * numChannels);
	currVal = malloc(sizeof(*currVal) * numChannels);
}

@end
