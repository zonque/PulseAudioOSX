#include "PAHP_PlugIn.h"
#include "PAHP_Device.h"
#include "HP_DeviceSettings.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAPropertyAddress.h"

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

PAHP_PlugIn::PAHP_PlugIn(CFUUIDRef inFactoryUUID) :
	HP_HardwarePlugIn(inFactoryUUID),
	device(NULL)
{
}

PAHP_PlugIn::~PAHP_PlugIn()
{
}

void
PAHP_PlugIn::InitializeWithObjectID(AudioObjectID inObjectID)
{
	HP_HardwarePlugIn::InitializeWithObjectID(inObjectID);
	AudioDeviceID newDeviceID = 0;
	OSStatus ret = AudioObjectCreate(GetInterface(),
					 kAudioObjectSystemObject,
					 kAudioDeviceClassID,
					 &newDeviceID);
	ThrowIfError(ret, CAException(ret),
		     "PAHP_PlugIn::InitializeWithObjectID: AudioObjectCreate() failed");

	device = new PAHP_Device(newDeviceID, this);
	device->Initialize();

	UInt32 isMaster = 0;
	UInt32 size = sizeof(UInt32);
	AudioHardwareGetProperty(kAudioHardwarePropertyProcessIsMaster, &size, &isMaster);
	if (isMaster)
		HP_DeviceSettings::RestoreFromPrefs(*device,
						    HP_DeviceSettings::sStandardControlsToSave,
						    HP_DeviceSettings::kStandardNumberControlsToSave);

	HP_Object::SetObjectStateMutexForID(newDeviceID, device->GetObjectStateMutex());
	ret = AudioObjectsPublishedAndDied(GetInterface(),
					   kAudioObjectSystemObject,
					   1, &newDeviceID, 0, NULL);
	AssertNoError(ret, "PAHP_PlugIn::InitializeWithObjectID: AudioObjectsPublishedAndDied() failed");
}

void
PAHP_PlugIn::Teardown()
{
	//  first figure out if this is being done as part of the process being torn down
	UInt32 isInitingOrExiting = 0;
	UInt32 size = sizeof(UInt32);
	AudioHardwareGetProperty(kAudioHardwarePropertyIsInitingOrExiting,
				 &size, &isInitingOrExiting);

	//  next figure out if this is the master process
	UInt32 isMaster = 0;
	size = sizeof(UInt32);
	AudioHardwareGetProperty(kAudioHardwarePropertyProcessIsMaster, &size, &isMaster);

	//  do the full teardown if this is outside of the process being torn down or this is the master process
	if ((isInitingOrExiting == 0) || (isMaster != 0)) {
		device->Do_StopAllIOProcs();
		CAPropertyAddress isAliveAddress(kAudioDevicePropertyDeviceIsAlive);
		device->PropertiesChanged(1, &isAliveAddress);

		if (isMaster)
			HP_DeviceSettings::SaveToPrefs(*device,
						       HP_DeviceSettings::sStandardControlsToSave,
						       HP_DeviceSettings::kStandardNumberControlsToSave);

		//	tell the HAL that the device has gone away
		AudioObjectID objectID = device->GetObjectID();
		OSStatus ret = AudioObjectsPublishedAndDied(GetInterface(),
							    kAudioObjectSystemObject,
							    0, NULL, 1, &objectID);
		AssertNoError(ret, "PAHP_PlugIn::Teardown: AudioObjectsPublishedAndDied() failed");

		//	remove the object state mutex
		HP_Object::SetObjectStateMutexForID(objectID, NULL);

		device->Teardown();
		delete device;
		device = NULL;

		HP_HardwarePlugIn::Teardown();
	} else {
		device->Do_StopAllIOProcs();
		device->Finalize();
	}
}

bool
PAHP_PlugIn::HasProperty(const AudioObjectPropertyAddress &inAddress) const
{
	switch(inAddress.mSelector) {
		case kAudioObjectPropertyName:
			return true;
		default:
			return HP_HardwarePlugIn::HasProperty(inAddress);
	}

	return false;
}

bool
PAHP_PlugIn::IsPropertySettable(const AudioObjectPropertyAddress &inAddress) const
{
	switch (inAddress.mSelector) {
		case kAudioObjectPropertyName:
			return false;
		default:
			return HP_HardwarePlugIn::IsPropertySettable(inAddress);
	}

	return false;
}

UInt32
PAHP_PlugIn::GetPropertyDataSize(const AudioObjectPropertyAddress	&inAddress,
				 UInt32					 inQualifierDataSize,
				 const void				*inQualifierData) const
{
	switch (inAddress.mSelector) {
		case kAudioObjectPropertyName:
			return sizeof(CFStringRef);

		default:
			return HP_HardwarePlugIn::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData);
	}

	return 0;
}

void
PAHP_PlugIn::GetPropertyData(const AudioObjectPropertyAddress	&inAddress,
			     UInt32				 inQualifierDataSize,
			     const void				*inQualifierData,
			     UInt32				&ioDataSize,
			     void				*outData) const
{
	switch (inAddress.mSelector) {
		case kAudioObjectPropertyName:
			ThrowIf(ioDataSize != GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_PlugIn::GetPropertyData: GetPropertyDataSize() failed");
			*static_cast<CFStringRef*>(outData) = CFSTR("org.pulseaudio.PAHardwarePlugIn");
			CFRetain(*static_cast<CFStringRef*>(outData));
			break;

		default:
			HP_HardwarePlugIn::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
			break;
	}
}

void
PAHP_PlugIn::SetPropertyData(const AudioObjectPropertyAddress	&inAddress,
			     UInt32				 inQualifierDataSize,
			     const void				*inQualifierData,
			     UInt32				 inDataSize,
			     const void 			*inData,
			     const AudioTimeStamp		*inWhen)
{
	switch (inAddress.mSelector) {
		default:
			HP_HardwarePlugIn::SetPropertyData(inAddress,
							   inQualifierDataSize,
							   inQualifierData,
							   inDataSize,
							   inData,
							   inWhen);
			break;
	}
}

extern "C" void*
New_PAHP_PlugIn(CFAllocatorRef * /* allocator */, CFUUIDRef requestedTypeUUID) 
{
	if (CFEqual(requestedTypeUUID, kAudioHardwarePlugInTypeID)) {
		PAHP_PlugIn *plugIn = new PAHP_PlugIn(requestedTypeUUID);
		plugIn->Retain();
		return plugIn->GetInterface();
	}

	return NULL;
}
