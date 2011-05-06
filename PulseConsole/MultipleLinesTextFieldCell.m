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

#import "MultipleLinesTextFieldCell.h"

@implementation MultipleLinesTextFieldCell

- (void) drawInteriorWithFrame: (NSRect) cellFrame
                        inView: (NSView *) controlView
{
        NSCharacterSet *set = [NSCharacterSet characterSetWithCharactersInString: @"|"];
        NSArray *array = [[self stringValue] componentsSeparatedByCharactersInSet: set];
        NSString *caption = [array objectAtIndex: 0];
        NSString *details = [array objectAtIndex: 1];
        NSMutableDictionary *attributes;

        attributes = [NSMutableDictionary dictionaryWithCapacity: 0];
        [attributes setObject: [NSFont boldSystemFontOfSize: 12.0]
                       forKey: NSFontAttributeName];

        [caption drawAtPoint: NSMakePoint(cellFrame.origin.x, cellFrame.origin.y + 2)
              withAttributes: attributes];

        attributes = [NSMutableDictionary dictionaryWithCapacity: 0];
        [attributes setObject: [NSFont systemFontOfSize: 11.0]
                       forKey: NSFontAttributeName];
        [attributes setObject: [NSColor grayColor]
                       forKey: NSForegroundColorAttributeName];

        [details drawAtPoint: NSMakePoint(cellFrame.origin.x, cellFrame.origin.y + 20)
              withAttributes: attributes];
}

@end
