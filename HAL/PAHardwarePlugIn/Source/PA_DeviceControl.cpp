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
	PA_DeviceControl *dev = static_cast<PA_DeviceControl *>(observer);
	dev->AnnounceDevice();
}

static void staticSetConfig(CFNotificationCenterRef /* center */,
			    void *observer,
			    CFStringRef /* name */,
			    const void * /* object */,
			    CFDictionaryRef userInfo)
{
	CFNumberRef n = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("pid"));
	pid_t pid;
	
	CFNumberGetValue(n, kCFNumberIntType, &pid);
	if (pid == getpid()) {
		PA_DeviceControl *dev = static_cast<PA_DeviceControl *>(observer);
		dev->SetConfig(userInfo);
	}
}

static void staticStreamVolumeChanged(CFNotificationCenterRef /* center */,
				      void *observer,
				      CFStringRef name,
				      const void * /* object */,
				      CFDictionaryRef userInfo)
{
	PA_DeviceControl *dev = static_cast<PA_DeviceControl *>(observer);
	dev->StreamVolumeChanged(name, userInfo);
}

static void staticStreamMuteChanged(CFNotificationCenterRef /* center */,
				    void *observer,
				    CFStringRef name,
				    const void * /* object */,
				    CFDictionaryRef userInfo)
{
	PA_DeviceControl *dev = static_cast<PA_DeviceControl *>(observer);
	dev->StreamMuteChanged(name, userInfo);
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
	PA_DeviceBackend *be = device->getBackend();
	
	UInt32 n = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);
	
	CFDictionarySetValue(userInfo, CFSTR("procname"), be->GetProcessName());

	//str = CopyDeviceName();
	//CFDictionarySetValue(userInfo, CFSTR("audioDevice"), str);
	//CFRelease(str);
	
	/*
	n = GetIOBufferFrameSize();
	number = CFNumberCreate(NULL, kCFNumberIntType, &n);
	CFDictionarySetValue(userInfo, CFSTR("IOBufferFrameSize"), number);
	CFRelease(number);
	*/

	CFDictionarySetValue(userInfo, CFSTR("serverName"), CFSTR("localhost"));
	
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
PA_DeviceControl::UnannounceDevice()
{
	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef number;
	CFStringRef str;
	
	UInt32 pid = getpid();
	number = CFNumberCreate(NULL, kCFNumberIntType, &pid);
	CFDictionarySetValue(userInfo, CFSTR("pid"), number);
	CFRelease(number);
	
	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("unannounceDevice"),
					     CFSTR("PAHP_Device"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

void
PA_DeviceControl::SetConfig(CFDictionaryRef config)
{
	CFStringRef server = (CFStringRef) CFDictionaryGetValue(config, CFSTR("serverName"));
	char buf[128];
	
	CFStringGetCString(server, buf, sizeof(buf), kCFStringEncodingASCII);	
	printf("REQUEST to connect to %s\n", buf);
}

void
PA_DeviceControl::StreamVolumeChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	Float32 value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberFloatType, &value);
	device->getBackend()->ChangeStreamVolume(0, value);
}

void
PA_DeviceControl::StreamMuteChanged(CFStringRef /* name */, CFDictionaryRef userInfo)
{
	bool value;
	CFNumberRef number = (CFNumberRef) CFDictionaryGetValue(userInfo, CFSTR("value"));
	
	CFNumberGetValue(number, kCFNumberIntType, &value);
	device->getBackend()->ChangeStreamMute(0, value);
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
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticScanDevices,
					CFSTR("scanDevices"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticSetConfig,
					CFSTR("setConfiguration"),
					CFSTR("PAHP_Device"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticStreamVolumeChanged,
					CFSTR("updateStreamVolume"),
					CFSTR("PAHP_LevelControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
	
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(),
					this,
					staticStreamMuteChanged,
					CFSTR("updateMuteVolume"),
					CFSTR("PAHP_BooleanControl"),
					CFNotificationSuspensionBehaviorDeliverImmediately);
}

void
PA_DeviceControl::Teardown()
{
	// FIXME
}
