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

#ifndef PA_OBJECT_H_
#define PA_OBJECT_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardwarePlugIn.h>
#include <CoreAudio/AudioHardware.h>
#include <pthread.h>

#ifndef CLASS_NAME
#error CLASS_NAME undefined
#endif

#define DebugLog(format, args...) \
	printf("%s::%s(), line %d: " format "\n", \
		CLASS_NAME, __func__, __LINE__, ## args);

class PA_Object
{
private:
	AudioObjectID objectID;
	pthread_mutex_t mutex;

public:
	
	PA_Object();
	~PA_Object();
	
	AudioObjectID	GetObjectID();
	void		SetObjectID(AudioObjectID i);
	
#pragma mark ### plugin interface ###

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
	
	void Show();
	
#pragma mark ### mutex ###

	void Lock();
	void Unlock();

	virtual void ReportOwnedObjects(std::vector<AudioObjectID> &arr);
	virtual PA_Object *FindObjectByID(AudioObjectID searchID) = 0;
	virtual const char *ClassName();
};

#endif // PA_OBJECT_H_
