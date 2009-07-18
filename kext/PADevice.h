//
//	PADevice.h
//

#ifndef PADEVICE_H
#define PADEVICE_H

#include <IOKit/audio/IOAudioDevice.h>

#define PADevice org_pulseaudio_ioaudiodevice

class PADevice : public IOAudioDevice
{
	OSDeclareDefaultStructors(PADevice)

public:
	virtual bool	initHardware(IOService *provider);
	PAEngine		*audioEngine;
};

#endif // PADEVICE_H