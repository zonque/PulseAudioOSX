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

#import <Cocoa/Cocoa.h>

@interface StreamListView : NSView
{
	NSMutableArray *streamViewItems;
	UInt32 streamType;
}

@property (nonatomic, assign) UInt32 streamType;

- (void) removeAllStreams;
- (void) recalcLayout;

#pragma mark # sink

- (void) sinkInfoAdded: (PASinkInfo *) sink;
- (void) sinkInfoRemoved: (PASinkInfo *) sink;
- (void) sinkInfoChanged: (PASinkInfo *) sink;

#pragma mark # sink input

- (void) sinkInputInfoAdded: (PASinkInputInfo *) input;
- (void) sinkInputInfoRemoved: (PASinkInputInfo *) input;
- (void) sinkInputInfoChanged: (PASinkInputInfo *) input;

#pragma mark # source

- (void) sourceInfoAdded: (PASourceInfo *) source;
- (void) sourceInfoRemoved: (PASourceInfo *) source;
- (void) sourceInfoChanged: (PASourceInfo *) source;

#pragma mark # source output

- (void) sourceOutputInfoAdded: (PASourceOutputInfo *) output;
- (void) sourceOutputInfoRemoved: (PASourceOutputInfo *) output;
- (void) sourceOutputInfoChanged: (PASourceOutputInfo *) output;

@end
