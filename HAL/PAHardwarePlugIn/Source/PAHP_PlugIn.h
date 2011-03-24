#ifndef _PAHP_PLUGIN_H_
#define _PAHP_PLUGIN_H_

#include "HP_HardwarePlugIn.h"

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

class PAHP_Device;

class PAHP_PlugIn : public HP_HardwarePlugIn
{
public:
				PAHP_PlugIn(CFUUIDRef inFactoryUUID);
	virtual			~PAHP_PlugIn();

	virtual void		InitializeWithObjectID(AudioObjectID inObjectID);
	virtual void		Teardown();

public:
	virtual bool		HasProperty(const AudioObjectPropertyAddress &inAddress) const;
	virtual bool		IsPropertySettable(const AudioObjectPropertyAddress &inAddress) const;
	virtual UInt32		GetPropertyDataSize(const AudioObjectPropertyAddress &inAddress,
						    UInt32 inQualifierDataSize,
						    const void* inQualifierData) const;
	virtual void		GetPropertyData(const AudioObjectPropertyAddress& inAddress,
						UInt32 inQualifierDataSize,
						const void *inQualifierData,
						UInt32 &ioDataSize,
						void *outData) const;
	virtual void		SetPropertyData(const AudioObjectPropertyAddress& inAddress,
						UInt32 inQualifierDataSize,
						const void *inQualifierData,
						UInt32 inDataSize,
						const void *inData,
						const AudioTimeStamp *inWhen);
	
	pa_mainloop_api *			GetPulseAudioAPI(void);

private:
	PAHP_Device		*device;
	pa_threaded_mainloop	*loop;
};

#endif /* _PAHP_PLUGIN_H_ */
