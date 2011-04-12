/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

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
	CFAllocatorRef allocator;

public:
	PA_Plugin(CFAllocatorRef inAllocator, AudioHardwarePlugInRef inPlugin);
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

	AudioHardwarePlugInRef GetInterface()	{ return plugin; };
	CFAllocatorRef GetAllocator()		{ return allocator; };
	virtual const char *ClassName();
};

#endif // PA_PLUGIN_H_
