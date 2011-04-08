#ifndef PA_DEVICE_H_
#define PA_DEVICE_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardware.h>

#include "PA_Object.h"
#include "PA_Stream.h"

typedef struct IOProcTracker
{
	AudioDeviceIOProc proc;
	AudioTimeStamp startTime;
	UInt32 startTimeFlags;
	void *clientData;
	Boolean enabled;
} IOProcTracker;

class PA_Device : public PA_Object
{
private:
	CFMutableArrayRef streams, ioProcList;
	
	CFStringRef deviceName, deviceManufacturer;
	UInt32 nInputStreams, nOutputStreams;
	
	CAMutex *ioProcListMutex;
	
public:
	PA_Device();
	~PA_Device();
	
	void Initialize();
	void Teardown();
	
#pragma mark ### plugin interface ###

	PA_Stream *GetStreamById(AudioObjectID inObjectID);
	IOProcTracker *FindIOProc(AudioDeviceIOProc inProc);
	IOProcTracker *FindIOProcByID(AudioDeviceIOProcID inProcID);
	
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
	
#pragma mark ### properties ###
	virtual Boolean	HasProperty(const AudioObjectPropertyAddress *inAddress);
	
	virtual OSStatus IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
					    Boolean *outIsSettable);
	
	virtual OSStatus GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
					     UInt32 inQualifierDataSize,
					     const void *inQualifierData,
					     UInt32 *outDataSize);
	
	virtual OSStatus GetPropertyData(const AudioObjectPropertyAddress *inAddress,
					 UInt32 inQualifierDataSize,
					 const void *inQualifierData,
					 UInt32 *ioDataSize,
					 void *outData);
	
	virtual OSStatus SetPropertyData(const AudioObjectPropertyAddress *inAddress,
					 UInt32 inQualifierDataSize,
					 const void *inQualifierData,
					 UInt32 inDataSize,
					 const void *inData);

#pragma mark ### internal stuff ###
	void	EnableAllIOProcs(Boolean enable);
	void	SetBufferSize(UInt32 size);
	
	PA_Object *findObjectById(AudioObjectID searchID);
};

#endif // PA_DEVICE_H_
