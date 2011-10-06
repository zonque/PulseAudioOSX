/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>

#define PAElementInfoChangedNotification @"PAElementInfoChangedNotification"

@class PAServerConnection;

@interface PAElementInfo : NSObject
{
    UInt32 index;
    PAServerConnection *server;
    NSString *name;
    
    BOOL initialized;
}

- (id) initWithServer: (PAServerConnection *) s;

@property (nonatomic, readonly) UInt32 index;
@property (nonatomic, readonly) PAServerConnection *server;
@property (nonatomic, readonly) NSString *name;

@end
