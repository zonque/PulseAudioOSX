#ifndef PA_DEVICE_H_
#define PA_DEVICE_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardware.h>

#include "PA_Object.h"
#include "PA_Stream.h"

class PA_Device : public PA_Object
{
private:
	CFMutableArrayRef streams;
	
public:
	PA_Device();
	~PA_Device();
	
	void Initialize();
	void Teardown();
	
	PA_Stream *GetStreamById(AudioObjectID inObjectID);
	
	OSStatus CreateIOProcID(AudioDeviceIOProc inProc,
				void *inClientData,
				AudioDeviceIOProcID *outIOProcID);
	
	OSStatus DestroyIOProcID(AudioDeviceIOProcID inIOProcID);
	
	OSStatus AddIOProc(AudioDeviceIOProc inProc, 
			   void *inClientData);
	
	OSStatus RemoveIOProc(AudioDeviceIOProc inProc);
	
	OSStatus Start(AudioDeviceIOProc inProc);
	
	OSStatus StartAtTime(AudioDeviceIOProc inProc,
			     AudioTimeStamp *ioRequestedStartTime,
			     UInt32 inFlags);
	
	OSStatus Stop(AudioDeviceIOProc inProc);
	
	OSStatus Read(const AudioTimeStamp *inStartTime,
		      AudioBufferList *outData);
	
	OSStatus GetCurrentTime(AudioTimeStamp* outTime);
	
	OSStatus TranslateTime(const AudioTimeStamp *inTime,
			       AudioTimeStamp *outTime);
	
	OSStatus GetNearestStartTime(AudioTimeStamp *ioRequestedStartTime,
				     UInt32 inFlags);
	
	OSStatus GetPropertyInfo(UInt32 inChannel,
				 Boolean isInput,
				 AudioDevicePropertyID
				 inPropertyID,
				 UInt32 *outSize,
				 Boolean *outWritable);
	
	OSStatus GetProperty(UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32* ioPropertyDataSize,
			     void* outPropertyData);

	OSStatus SetProperty(const AudioTimeStamp *inWhen,
			     UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 inPropertyDataSize,
			     const void *inPropertyData);
	
	PA_Object *findObjectById(AudioObjectID searchID);
};

#endif // PA_DEVICE_H_
