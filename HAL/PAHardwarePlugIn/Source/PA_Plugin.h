#ifndef PA_PLUGIN_H_
#define PA_PLUGIN_H_

#include <CoreAudio/AudioHardwarePlugIn.h>
#include "PA_Device.h"
#include "PA_Stream.h"
#include "PA_Object.h"

class PA_Plugin : public PA_Object {

private:
	CFMutableArrayRef devices;
	AudioObjectID objectID;

	PA_Device *GetDeviceById(AudioObjectID inObjectID);
	PA_Stream *GetStreamById(AudioObjectID inObjectID);
	
	AudioHardwarePlugInRef plugin;

public:
	PA_Plugin(AudioHardwarePlugInRef inPlugin);
	~PA_Plugin();
	
	ULONG AddRef();
	ULONG Release();
	
	OSStatus Initialize();
	OSStatus InitializeWithObjectID(AudioObjectID inObjectID);
	
	OSStatus Teardown();
	
	void ObjectShow(AudioObjectID inObjectID);
	
	Boolean ObjectHasProperty(AudioObjectID inObjectID,
				  const AudioObjectPropertyAddress *inAddress);
	
	OSStatus ObjectIsPropertySettable(AudioObjectID inObjectID,
					  const AudioObjectPropertyAddress *inAddress,
					  Boolean *outIsSettable);
	
	OSStatus ObjectGetPropertyDataSize(AudioObjectID inObjectID,
					   const AudioObjectPropertyAddress *inAddress,
					   UInt32 inQualifierDataSize,
					   const void *inQualifierData,
					   UInt32 *outDataSize);
	
	OSStatus ObjectGetPropertyData(AudioObjectID inObjectID,
				       const AudioObjectPropertyAddress *inAddress,
				       UInt32 inQualifierDataSize,
				       const void *inQualifierData,
				       UInt32 *ioDataSize,
				       void *outData);
	
	OSStatus ObjectSetPropertyData(AudioObjectID inObjectID,
				       const AudioObjectPropertyAddress *inAddress,
				       UInt32 inQualifierDataSize,
				       const void *inQualifierData,
				       UInt32 inDataSize,
				       const void *inData);
	
	OSStatus DeviceCreateIOProcID(AudioDeviceID inDeviceID,
				      AudioDeviceIOProc inProc,
				      void *inClientData,
				      AudioDeviceIOProcID *outIOProcID);
	
	OSStatus DeviceDestroyIOProcID(AudioDeviceID inDeviceID,
				       AudioDeviceIOProcID inIOProcID);
	
	OSStatus DeviceAddIOProc(AudioDeviceID inDeviceID,
				 AudioDeviceIOProc inProc, 
				 void *inClientData);
	
	OSStatus DeviceRemoveIOProc(AudioDeviceID inDeviceID,
				    AudioDeviceIOProc inProc);
	
	OSStatus DeviceStart(AudioDeviceID inDeviceID,
			     AudioDeviceIOProc inProc);
	
	OSStatus DeviceStartAtTime(AudioDeviceID inDeviceID,
				   AudioDeviceIOProc inProc,
				   AudioTimeStamp *ioRequestedStartTime,
				   UInt32 inFlags);
	
	OSStatus DeviceStop(AudioDeviceID inDeviceID,
			    AudioDeviceIOProc inProc);
	
	OSStatus DeviceRead(AudioDeviceID inDeviceID,
			    const AudioTimeStamp *inStartTime,
			    AudioBufferList *outData);
	
	OSStatus DeviceGetCurrentTime(AudioDeviceID inDeviceID,
				      AudioTimeStamp* outTime);
	
	OSStatus DeviceTranslateTime(AudioDeviceID inDeviceID,
				     const AudioTimeStamp *inTime,
				     AudioTimeStamp *outTime);
	
	OSStatus DeviceGetNearestStartTime(AudioDeviceID inDeviceID,
					   AudioTimeStamp *ioRequestedStartTime,
					   UInt32 inFlags);
	
	OSStatus DeviceGetPropertyInfo(AudioDeviceID inDeviceID,
				       UInt32 inChannel,
				       Boolean isInput,
				       AudioDevicePropertyID
				       inPropertyID,
				       UInt32 *outSize,
				       Boolean *outWritable);
	
	OSStatus DeviceGetProperty(AudioDeviceID inDeviceID,
				   UInt32 inChannel,
				   Boolean isInput,
				   AudioDevicePropertyID inPropertyID,
				   UInt32* ioPropertyDataSize,
				   void* outPropertyData);
	
	OSStatus DeviceSetProperty(AudioDeviceID inDeviceID,
				   const AudioTimeStamp *inWhen,
				   UInt32 inChannel,
				   Boolean isInput,
				   AudioDevicePropertyID inPropertyID,
				   UInt32 inPropertyDataSize,
				   const void *inPropertyData);

	OSStatus StreamGetPropertyInfo(AudioStreamID inStreamID,
				       UInt32 inChannel,
				       AudioDevicePropertyID inPropertyID,
				       UInt32 *outSize,
				       Boolean *outWritable);
	
	OSStatus StreamGetProperty(AudioStreamID inStreamID,
				   UInt32 inChannel,
				   AudioDevicePropertyID inPropertyID,
				   UInt32 *ioPropertyDataSize,
				   void *outPropertyData);
	
	OSStatus StreamSetProperty(AudioStreamID inStreamID,
				   const AudioTimeStamp *inWhen,
				   UInt32 inChannel,
				   AudioDevicePropertyID inPropertyID,
				   UInt32 inPropertyDataSize,
				   const void *inPropertyData);
	
	PA_Object *FindObjectByID(AudioObjectID searchID);

};

#endif // PA_PLUGIN_H_
