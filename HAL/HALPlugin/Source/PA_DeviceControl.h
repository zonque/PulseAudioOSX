#ifndef PA_DEVICE_CONTROL_H
#define PA_DEVICE_CONTROL_H

class PA_Device;

class PA_DeviceControl
{
private:
	PA_Device *device;
	
public:
	PA_DeviceControl(PA_Device *inDevice);
	~PA_DeviceControl();
	
	void Initialize();
	void Teardown();
	
	void AnnounceDevice();
	void UnannounceDevice();
	void SetConfig(CFDictionaryRef config);

	void StreamVolumeChanged(CFStringRef name, CFDictionaryRef userInfo);
	void StreamMuteChanged(CFStringRef name, CFDictionaryRef userInfo);
};

#endif // PA_DEVICE_CONTROL_H
