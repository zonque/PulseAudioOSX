/***
  This file is part of PulseConsole

  Copyright 2009 Daniel Mack <daniel@caiaq.de>

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

#include <dns_sd.h>
#include <nameser.h>
#include <sys/socket.h>
#include <net/if.h>
#include <assert.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

@interface BonjourListener : NSObject {
    DNSServiceRef dnsService;
    CFSocketRef   dnsSocket;
    CFRunLoopSourceRef dnsRunLoopSource;
    NSMutableArray *known;
}

- (id) initForService: (const char *) service;
- (void) start;

@end
