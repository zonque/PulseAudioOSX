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

#include <CoreFoundation/CoreFoundation.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "PA_SocketConnection.h"

PA_SocketConnection::PA_SocketConnection()
{
	callbackArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
}

PA_SocketConnection::~PA_SocketConnection()
{
	CFRelease(callbackArray);
	
	if (source)
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);

	if (socket && CFSocketIsValid(socket))
		CFSocketInvalidate(socket);
}

void staticSocketCallback (CFSocketRef s,
			   CFSocketCallBackType callbackType,
			   CFDataRef address,
			   const void *data,
			   void *info)
{
	PA_SocketConnection *c = (PA_SocketConnection *) info;
	c->socketCallback(s, callbackType, address, data);
}

void
PA_SocketConnection::socketCallback(CFSocketRef s,
				    CFSocketCallBackType callbackType,
				    CFDataRef /* address */,
				    const void *data)
{
	if (s != socket)
		return;
	
	switch (callbackType) {
		case kCFSocketConnectCallBack:
			break;
		case kCFSocketDataCallBack: {
			CFDataRef xmlData = (CFDataRef) data;
			if (CFDataGetLength(xmlData) == 0) {
				CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),
								     kSocketConnectionDiedMessage,
								     this, NULL, true);
				return;
			}

			CFStringRef errorString;
			CFDictionaryRef userInfo = (CFDictionaryRef)
				CFPropertyListCreateFromXMLData(NULL,
								xmlData,
								kCFPropertyListImmutable,
								&errorString);
			if (!userInfo) {
				printf("%s(): CFPropertyListCreateFromXMLData failed\n", __func__);
				CFShow(errorString);
				return;
			}
			
			CFStringRef messageName = (CFStringRef) CFDictionaryGetValue(userInfo, CFSTR(PAOSX_MessageNameKey));
			
			CFShow(callbackArray);
			
			for (SInt32 i = 0; i < CFArrayGetCount(callbackArray); i++) {
				CFDictionaryRef dict = (CFDictionaryRef) CFArrayGetValueAtIndex(callbackArray, i);
				CFStringRef name = (CFStringRef) CFDictionaryGetValue(dict, CFSTR("messageName"));

				CFShow(dict);

				if (CFStringCompare(name, messageName, 0) == 0) {
					CFNumberRef number;
					PA_SocketConnectionCallback callback;
					void *info;
					
					number = (CFNumberRef) CFDictionaryGetValue(dict, "callback");
					CFNumberGetValue(number, kCFNumberLongType, &callback);
					CFRelease(number);

					number = (CFNumberRef) CFDictionaryGetValue(dict, "info");
					CFNumberGetValue(number, kCFNumberLongType, &info);
					CFRelease(number);
					
					callback(this, messageName, userInfo, info);
				}
			}

			CFRelease(userInfo);

			break;
		}
			
		default:
			printf("unhandled socket callback type %d\n", (int) callbackType);
			break;
	}
}

Boolean
PA_SocketConnection::sendMessage(CFStringRef name, CFDictionaryRef userInfo)
{
	if (!CFSocketIsValid(socket))
		return false;
	
	CFMutableDictionaryRef dict;

	if (userInfo)
		dict = CFDictionaryCreateMutableCopy(NULL, 0, userInfo);
	else
		dict = CFDictionaryCreateMutable(NULL, 0,
						 &kCFCopyStringDictionaryKeyCallBacks,
						 &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue(dict, CFSTR(PAOSX_MessageNameKey), name);

	CFErrorRef error = NULL;
	CFDataRef data = CFPropertyListCreateData(NULL, (CFPropertyListRef) dict,
						  kCFPropertyListXMLFormat_v1_0,
						  0, &error);
	
	CFRelease(dict);

	if (error) {
		printf("%s(): CFPropertyListCreateData() failed", __func__);
		return false;
	}

	CFSocketError ret = CFSocketSendData(socket, serverAddress, data, 1);
	if (ret != noErr)
		printf("%s(): CFSocketSendData returned %d\n", __func__, (int) ret);
	
	CFRelease(data);
	
	return ret == noErr;
}

void
PA_SocketConnection::RegisterCallback(CFStringRef messageName,
				      PA_SocketConnectionCallback callback,
				      void *info)
{
	CFNumberRef number;
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0,
								&kCFTypeDictionaryKeyCallBacks,
								&kCFTypeDictionaryValueCallBacks);
	
	CFDictionarySetValue(dict, CFSTR("messageName"), messageName);

	number = CFNumberCreate(NULL, kCFNumberLongType, &callback);
	CFDictionarySetValue(dict, CFSTR("callback"), number);
	CFRelease(number);

	number = CFNumberCreate(NULL, kCFNumberLongType, info);
	CFDictionarySetValue(dict, CFSTR("info"), number);
	CFRelease(number);
	
	CFArrayAppendValue(callbackArray, dict);
}

Boolean
PA_SocketConnection::Connect()
{
	CFSocketContext context;
	memset(&context, 0, sizeof(context));
	context.info = this;
	
	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	server.sun_len = sizeof(server);
	strlcpy(server.sun_path, PAOSX_HelperSocketName, sizeof(server.sun_path));	
	
	serverAddress = CFDataCreate(NULL, (UInt8 *) &server, sizeof(server));
	
	if (!serverAddress) {
		printf("CFDataCreate() failed\n");
		return false;
	}
	
	CFSocketSignature signature;
	signature.protocolFamily = AF_UNIX;
	signature.socketType = SOCK_STREAM;
	signature.protocol = 0;
	signature.address = serverAddress;

	socket = CFSocketCreateConnectedToSocketSignature(NULL, &signature, 
							  kCFSocketDataCallBack | kCFSocketConnectCallBack,
							  staticSocketCallback,
							  &context, 10);
	
	if (!socket) {
		printf("CFSocketCreateConnectedToSocketSignature() failed\n");
		return false;
	}
	
	
	source = CFSocketCreateRunLoopSource(NULL, socket, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	
	return true;
}

Boolean
PA_SocketConnection::IsConnected()
{
	return socket && CFSocketIsValid(socket);
}
