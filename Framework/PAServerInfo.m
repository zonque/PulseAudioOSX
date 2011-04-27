/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import "PAServerInfo.h"


@implementation PAServerInfo

@synthesize userName;
@synthesize hostName;
@synthesize serverName;
@synthesize version;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize defaultSinkName;
@synthesize defaultSourceName;
@synthesize cookie;

- (NSDictionary *) dictionary
{
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[d setObject: userName
	      forKey: @"User Name"];
	[d setObject: hostName
	      forKey: @"Host Name"];
	[d setObject: version
	      forKey: @"Server Version"];
	[d setObject: sampleSpec
	      forKey: @"Sample Spec"];
	[d setObject: channelMap
	      forKey: @"Channel Map"];
	[d setObject: defaultSinkName
	      forKey: @"Default Sink Name"];
	[d setObject: defaultSourceName
	      forKey: @"Default Source Name"];

	return d;
}


@end
