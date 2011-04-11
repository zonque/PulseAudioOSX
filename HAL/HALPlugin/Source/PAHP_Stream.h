#ifndef _PAHP_STREAM_H_
#define _PAHP_STREAM_H_

#include "HP_Stream.h"
#include <IOKit/IOKitLib.h>
#include <pulse/pulseaudio.h>

class PAHP_Device;
class PAHP_PlugIn;

class PAHP_Stream : public HP_Stream
{
public:
				PAHP_Stream(AudioStreamID inAudioStreamID,
					    PAHP_PlugIn *inPlugIn,
					    PAHP_Device *inOwningDevice,
					    bool inIsInput,
					    UInt32 inStartingDeviceChannelNumber);
	virtual			~PAHP_Stream();

	virtual void		Initialize();
	virtual void		Teardown();
	virtual void		Finalize();

private:
	PAHP_PlugIn *		plugin;
	PAHP_Device *		owningDevice;

public:
	virtual bool		HasProperty(const AudioObjectPropertyAddress &inAddress) const;
	virtual bool		IsPropertySettable(const AudioObjectPropertyAddress &inAddress) const;
	virtual UInt32		GetPropertyDataSize(const AudioObjectPropertyAddress &inAddress,
						    UInt32 inQualifierDataSize,
						    const void *inQualifierData) const;
	virtual void		GetPropertyData(const AudioObjectPropertyAddress &inAddress,
						UInt32 inQualifierDataSize,
						const void *inQualifierData,
						UInt32 &ioDataSize,
						void *outData) const;
	virtual void		SetPropertyData(const AudioObjectPropertyAddress &inAddress,
						UInt32 inQualifierDataSize,
						const void *inQualifierData,
						UInt32 inDataSize,
						const void *inData,
						const AudioTimeStamp *inWhen);

public:
	virtual bool		TellHardwareToSetPhysicalFormat(const AudioStreamBasicDescription &inFormat);
	void			RefreshAvailablePhysicalFormats();

private:
	void			AddAvailablePhysicalFormats();
	bool			mNonMixableFormatSet;

};

#endif /* _PAHP_STREAM_H_ */
