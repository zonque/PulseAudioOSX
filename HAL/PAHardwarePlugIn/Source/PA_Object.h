#ifndef PA_OBJECT_H_
#define PA_OBJECT_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardware.h>

class PA_Object
{
private:
	AudioObjectID objectID;

	CFMutableArrayRef properties;

public:
	
	PA_Object();
	~PA_Object();
	
	AudioObjectID	GetObjectID()			{ return objectID; };
	void		SetObjectID(AudioObjectID i)	{ objectID = i; };
	
	Boolean	HasProperty(const AudioObjectPropertyAddress *inAddress);

	OSStatus IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
				    Boolean *outIsSettable);
	
	OSStatus GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
				     UInt32 inQualifierDataSize,
				     const void *inQualifierData,
				     UInt32 *outDataSize);
	
	OSStatus GetPropertyData(const AudioObjectPropertyAddress *inAddress,
				 UInt32 inQualifierDataSize,
				 const void *inQualifierData,
				 UInt32 *ioDataSize,
				 void *outData);

	OSStatus SetPropertyData(const AudioObjectPropertyAddress *inAddress,
				 UInt32 inQualifierDataSize,
				 const void *inQualifierData,
				 UInt32 inDataSize,
				 const void *inData);
	
	void Show();
	
	virtual PA_Object *findObjectById(AudioObjectID searchID) = 0;
};

#endif // PA_OBJECT_H_
