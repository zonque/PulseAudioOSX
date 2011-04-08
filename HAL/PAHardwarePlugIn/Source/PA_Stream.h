#ifndef PA_STREAM_H_
#define PA_STREAM_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardware.h>

#include "PA_Object.h"

class PA_Stream : public PA_Object
{
private:
	CFMutableArrayRef *controls;
	
public:
	PA_Stream();
	~PA_Stream();
	
	void Initialize();
	void Teardown();
	
	OSStatus GetPropertyInfo(UInt32 inChannel,
				 AudioDevicePropertyID inPropertyID,
				 UInt32 *outSize,
				 Boolean *outWritable);
	
	OSStatus GetProperty(UInt32 inChannel,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 *ioPropertyDataSize,
			     void *outPropertyData);
	
	OSStatus SetProperty(const AudioTimeStamp *inWhen,
			     UInt32 inChannel,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 inPropertyDataSize,
			     const void *inPropertyData);
	
	PA_Object *findObjectById(AudioObjectID searchID);
};

#endif // PA_STREAM_H_
