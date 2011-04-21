/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#ifndef PA_SOCKET_CONNECTION_H_
#define PA_SOCKET_CONNECTION_H_

#include "SocketCommunicationDefs.h"

#define kSocketConnectionDiedMessage		CFSTR("kSocketConnectionDiedMessage")
#define kSocketConnectionMessageReceived	CFSTR("kSocketConnectionMessageReceived")

class PA_SocketConnection;

typedef void (*PA_SocketConnectionCallback) (PA_SocketConnection *connection,
					     CFStringRef messageName,
					     CFDictionaryRef userInfo,
					     void *info);

class PA_SocketConnection
{
private:
	CFSocketRef socket;
	CFDataRef serverAddress;
	CFRunLoopSourceRef source;
	CFMutableArrayRef callbackArray;

public:
	void socketCallback(CFSocketRef s,
			    CFSocketCallBackType callbackType,
			    CFDataRef address,
			    const void *data);
	
	PA_SocketConnection();
	~PA_SocketConnection();

	Boolean	Connect();
	Boolean	IsConnected();
	
	Boolean sendMessage(CFStringRef name, CFDictionaryRef userInfo);
	Boolean setClientType(CFStringRef type);
	void RegisterCallback(CFStringRef messageName,
			      PA_SocketConnectionCallback callback,
			      void *info);
};

#endif /* PA_SOCKET_CONNECTION_H_ */
