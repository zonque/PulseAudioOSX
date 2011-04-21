/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAOSX_SOCKET_COMMUNICATION_DEFS_H
#define PAOSX_SOCKET_COMMUNICATION_DEFS_H

#define PAOSX_HelperSocketName		"/tmp/pulseaudio-osx-helper.socket"
#define PAOSX_MessageNameKey		"__messageName"
#define PAOSX_MessageClientType		"__clientType"

#define PAOSX_SocketConnectionAnnounceMessage	"AnnounceClient"
#define PAOSX_SocketConnectionSignOffMessage	"SignOffClient"
#define PAOSX_SocketConnectionSetConfigMessage	"SetConfig"

#endif /* PAOSX_SOCKET_COMMUNICATION_DEFS_H */
