/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
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

#ifndef PA_DEVICE_CONTROL_H
#define PA_DEVICE_CONTROL_H

class PA_Device;

class PA_DeviceControl
{
private:
	PA_Device *			device;
	CFNotificationCenterRef		center;
	
public:
	PA_DeviceControl(PA_Device *inDevice);
	~PA_DeviceControl();
	
	void		Initialize();
	void		Teardown();
	
	PA_Device *	GetDevice()	{ return device; };
	void		AnnounceDevice();
	void		UnannounceDevice();
	void		SetConfig(CFDictionaryRef config);

	void		StreamVolumeChanged(CFStringRef name, CFDictionaryRef userInfo);
	void		StreamMuteChanged(CFStringRef name, CFDictionaryRef userInfo);
};

#endif // PA_DEVICE_CONTROL_H
