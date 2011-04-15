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

#define CLASS_NAME "PA_DeviceControl"

#include <IOKit/IOKitLib.h>
#include "PA_DeviceControl.h"
#include "PA_DeviceBackend.h"
#include "PA_Device.h"

#pragma mark ### static wrappers ###

static void staticScanDevices(CFNotificationCenterRef /* center */,
			      void *observer,
			      CFStringRef /* name */,
			      const void * /* object */,
			      CFDictionaryRef /* userInfo */)
{
	PA_DeviceControl *control = static_cast<PA_DeviceControl *>(observer);
	PA_Device *dev = control->GetDevice();
	
	if (dev->IsRunning())
		control->AnnounceDevice();
}

static void staticSetConfig(CFNotificationCenterRef /* center */,
			    void *observer,
			    CFStringRef /* name */,
			    const void * /* object */,
			    CFDictionaryRef userInfo)
{
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("pid"));
	pid_t pid;

	CFNumberGetValue(number, kCFNumberIntType, &pid);
	if (pid == getpid()) {
		PA_DeviceControl *control = static_cast<PA_DeviceControl *>(observer);
		control->SetConfig(userInfo);
	}
}

static void staticStreamVolumeChanged(CFNotificationCenterRef /* center */,
				      void *observer,
				      CFStringRef name,
				      const void * /* object */,
				      CFDictionaryRef userInfo)
{
	PA_DeviceControl *control = static_cast<PA_DeviceControl *>(observer);
	control->StreamVolumeChanged(name, userInfo);
}

static void staticStreamMuteChanged(CFNotificationCenterRef /* center */,
				    void *observer,
				    CFStringRef name,
				    const void * /* object */,
				    CFDictionaryRef userInfo)
{
	PA_DeviceControl *control = static_cast<PA_DeviceControl *>(observer);
	control->StreamMuteChanged(name, userInfo);
}

#pragma mark ### PA_DeviceControl ###

void
PA_DeviceControl::AnnounceDevice()
{
	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef number;
	CFStringRef str;
	PA_DeviceBackend *be = device->GetBackend();
	
	UInt32 n = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);
	
	CFDictionarySetValue(userInfo, CFSTR("procname"), be->GetProcessName());

	str = device->CopyDeviceName();
	CFDictionarySetValue(userInfo, CFSTR("audioDevice"), str);
	CFRelease(str);
	
	n = device->GetIOBufferFrameSize();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("IOBufferFrameSize"), number);
	CFRelease(number);

	CFStringRef serverName = be->GetHostName();
	CFDictionarySetValue(userInfo, CFSTR("serverName"), serverName);
	CFRelease(serverName);
	
	n = be->GetConnectionStatus();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("connectionStatus"), number);
	CFRelease(number);
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("announceDevice"),
					     CFSTR("PAHP_Device"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

void
PA_DeviceControl::SignOffDevice()
{
	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef number;
	
	UInt32 pid = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &pid);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("signOffDevice"),
					     CFSTR("PAHP_Device"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

void
PA_DeviceControl::SetConfig(CFDictionaryRef config)
{
	PA_DeviceBackend *be = device->GetBackend();
	CFStringRef server = (CFStringRef) CFDictionaryGetValue(config, CFSTR("serverName"));
	be->SetHostName(server, NULL);
	be->Reconnect();
}

void
PA_DeviceControl::StreamVolumeChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	Float32 value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberFloatType, &value);
	device->GetBackend()->ChangeStreamVolume(0, value);
}

void
PA_DeviceControl::StreamMuteChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	bool value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberIntType, &value);
	device->GetBackend()->ChangeStreamMute(0, value);
}

#pragma mark ### Construct/Desconstruct

PA_DeviceControl::PA_DeviceControl(PA_Device *inDevice) :
	device(inDevice)
{
}

PA_DeviceControl::~PA_DeviceControl()
{
}

void
PA_DeviceControl::Initialize()
{
	center = CFNotificationCenterGetDistributedCenter();
	
	CFNotificationCenterAddObserver(center, this,
					staticScanDevices,
					CFSTR("scanDevices"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(center, this,
					staticSetConfig,
					CFSTR("setConfiguration"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(center, this,
					staticStreamVolumeChanged,
					CFSTR("updateStreamVolume"),
					CFSTR("PAHP_LevelControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(center, this,
					staticStreamMuteChanged,
					CFSTR("updateMuteVolume"),
					CFSTR("PAHP_BooleanControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
}

void
PA_DeviceControl::Teardown()
{
	CFNotificationCenterRemoveObserver(center, this,
					   CFSTR("scanDevices"),
					   CFSTR("PAHP_Device"));

	CFNotificationCenterRemoveObserver(center, this,
					   CFSTR("setConfiguration"),
					   CFSTR("PAHP_Device"));

	CFNotificationCenterRemoveObserver(center, this,
					   CFSTR("updateStreamVolume"),
					   CFSTR("PAHP_LevelControl"));

	CFNotificationCenterRemoveObserver(center, this,
					   CFSTR("updateMuteVolume"),
					   CFSTR("PAHP_LevelControl"));
}
