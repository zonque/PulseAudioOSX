/***
  This file is part of PulseConsole

  Copyright 2009-2011 Daniel Mack <pulseaudio@zonque.de>

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

#import "BonjourListener.h"

#define MAX_DOMAIN_LABEL 63
#define MAX_DOMAIN_NAME 255

@implementation BonjourListener

struct dnsContext {
    DNSServiceRef service;
    CFSocketRef   socket;
    CFRunLoopSourceRef runLoopSource;
    BonjourListener *listener;
};

static void bonjour_read_callback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
    BonjourListener *l = info;

    DNSServiceErrorType err= DNSServiceProcessResult(l->dnsService);

    if (err != kDNSServiceErr_NoError)
        printf("DNSServiceProcessResult returned %d\n", err);
}

static void resolve_read_callback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
    struct dnsContext *c = info;

    DNSServiceErrorType err= DNSServiceProcessResult(c->service);

    if (err != kDNSServiceErr_NoError)
        printf("DNSServiceProcessResult returned %d\n", err);
}

static void dns_service_resolve_reply (DNSServiceRef sdRef,
                                       DNSServiceFlags flags,
                                       uint32_t interfaceIndex,
                                       DNSServiceErrorType errorCode,
                                       const char *fullname,
                                       const char *hosttarget,
                                       uint16_t port,
                                       uint16_t txtLen,
                                       const unsigned char *txtRecord,
                                       void *context)
{
    BonjourListener *l = context;

    if (![l->known containsObject: [NSString stringWithCString: hosttarget
													  encoding: NSUTF8StringEncoding]]) {
         [l->known addObject: [NSString stringWithCString: hosttarget
												 encoding: NSUTF8StringEncoding]];

        [[NSNotificationCenter defaultCenter]
				postNotificationName:@"serviceAdded" object: l
                userInfo: [NSDictionary dictionaryWithObject: [NSString stringWithFormat: @"%s", hosttarget] forKey: @"name"]];
    }
}

- (void) browseCallback: (DNSServiceFlags) flags
            serviceName: (const char *) serviceName
                regtype: (const char *) regtype
            replyDomain: (const char *) replyDomain
{
    if (![known containsObject: [NSString stringWithCString: serviceName
												   encoding: NSUTF8StringEncoding]]) {
         [known addObject: [NSString stringWithCString: serviceName
											  encoding: NSUTF8StringEncoding]];

        [[NSNotificationCenter defaultCenter]
				postNotificationName:@"serviceAdded" object: self
                userInfo: [NSDictionary dictionaryWithObject: [NSString stringWithFormat: @"%s", serviceName, replyDomain] forKey: @"name"]];
    }
}

static void browse_callback (DNSServiceRef sdRef,
                             DNSServiceFlags flags,
                             uint32_t interfaceIndex,
                             DNSServiceErrorType errorCode,
                             const char *serviceName,
                             const char *regtype,
                             const char *replyDomain,
                             void *context)
{
#if 1
    struct dnsContext *c = malloc(sizeof(struct dnsContext));

    c->listener = context;

    DNSServiceResolve (&c->service, 
                        0, 
                        0, 
                        serviceName,
                        regtype,
                        replyDomain,
                        dns_service_resolve_reply,
                        context);

    CFSocketNativeHandle sock;
    CFOptionFlags        sockFlags;
    CFSocketContext      dcontext = { 0, c, NULL, NULL, NULL };
    
    sock = DNSServiceRefSockFD(c->service);
    
    c->socket = CFSocketCreateWithNative(NULL, sock, kCFSocketReadCallBack, resolve_read_callback, &dcontext);

    sockFlags = CFSocketGetSocketFlags(c->socket);
    CFSocketSetSocketFlags(c->socket, sockFlags & (~kCFSocketCloseOnInvalidate));
    
    c->runLoopSource = CFSocketCreateRunLoopSource(NULL, c->socket, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), c->runLoopSource, kCFRunLoopCommonModes);


    return;
#else
    BonjourListener *l = context;
    [l browseCallback: flags
          serviceName: serviceName
              regtype: regtype
          replyDomain: replyDomain];
#endif
}

- (id) initForService: (const char *) service
{
    [super init];

    DNSServiceBrowse (&dnsService,
                      0,
                      0,
                      service,
                      NULL,
                      browse_callback,
                      self);

    known = [NSMutableArray arrayWithCapacity: 0];
    [known retain];

    CFSocketNativeHandle sock;
    CFOptionFlags        sockFlags;
    CFSocketContext      dcontext = { 0, self, NULL, NULL, NULL };
    
    sock = DNSServiceRefSockFD(dnsService);
    
    dnsSocket = CFSocketCreateWithNative(NULL, sock, kCFSocketReadCallBack, bonjour_read_callback, &dcontext);

    sockFlags = CFSocketGetSocketFlags(dnsSocket);
    CFSocketSetSocketFlags(dnsSocket, sockFlags & (~kCFSocketCloseOnInvalidate));
    
    dnsRunLoopSource = CFSocketCreateRunLoopSource(NULL, dnsSocket, 0);

    return self;
}

- (void) start
{
    CFRunLoopAddSource(CFRunLoopGetCurrent(), dnsRunLoopSource, kCFRunLoopCommonModes);
}

- (void) dealloc
{
    if (dnsService)
        DNSServiceRefDeallocate(dnsService);

    [super dealloc];
}

@end
