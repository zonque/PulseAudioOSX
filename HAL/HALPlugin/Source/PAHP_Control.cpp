#include "PAHP_Control.h"
#include "PAHP_Device.h"
#include "PAHP_PlugIn.h"

#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CACFNumber.h"
#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"

PAHP_LevelControl::PAHP_LevelControl(AudioObjectID inObjectID,
				     AudioClassID inClassID,
				     AudioObjectPropertyScope inDevicePropertyScope,
				     AudioObjectPropertyElement inDevicePropertyElement,
				     PAHP_PlugIn *inPlugIn,
				     PAHP_Device *inOwningDevice) :
	HP_LevelControl(inObjectID, inClassID, inPlugIn, inOwningDevice),
	devicePropertyScope(inDevicePropertyScope),
	devicePropertyElement(inDevicePropertyElement),
	volumeCurve(),
	currentRawValue(0)
{
}

PAHP_LevelControl::~PAHP_LevelControl()
{
}

void
PAHP_LevelControl::Initialize()
{
	//	cache the info about the control
	SInt32 minRaw = 0;
	SInt32 maxRaw = 1024;
	Float32 minDB = -90;
	Float32 maxDB = 0;
	
	//	set up the volume curve
	volumeCurve.ResetRange();
	volumeCurve.AddRange(minRaw, maxRaw, minDB, maxDB);
	
	//	cache the raw value
	CacheRawValue();
}

void
PAHP_LevelControl::Teardown()
{
}

AudioObjectPropertyScope
PAHP_LevelControl::GetPropertyScope() const
{
	return devicePropertyScope;
}

AudioObjectPropertyElement
PAHP_LevelControl::GetPropertyElement() const
{
	return devicePropertyElement;
}

Float32
PAHP_LevelControl::GetMinimumDBValue() const
{
	return volumeCurve.GetMinimumDB();
}

Float32
PAHP_LevelControl::GetMaximumDBValue() const
{
	return volumeCurve.GetMaximumDB();
}

Float32
PAHP_LevelControl::GetDBValue() const
{
	return volumeCurve.ConvertRawToDB(GetRawValue());
}

void
PAHP_LevelControl::SetDBValue(Float32 inDBValue)
{	
	SetRawValue(volumeCurve.ConvertDBToRaw(inDBValue));
}

Float32
PAHP_LevelControl::GetScalarValue() const
{
	return volumeCurve.ConvertRawToScalar(GetRawValue());
}

void
PAHP_LevelControl::SetScalarValue(Float32 inScalarValue)
{
	SetRawValue(volumeCurve.ConvertScalarToRaw(inScalarValue));

	CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
								    &kCFCopyStringDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	CFNumberRef value = CFNumberCreate(NULL, kCFNumberFloatType, &inScalarValue);
	CFDictionarySetValue(userInfo, CFSTR("value"), value);
	CFRelease(value);

	CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
					     CFSTR("updateStreamVolume"),
					     CFSTR("PAHP_LevelControl"),
					     userInfo,
					     true);
	CFRelease(userInfo);
}

Float32
PAHP_LevelControl::ConverScalarValueToDBValue(Float32 inScalarValue) const
{
	return volumeCurve.ConvertScalarToDB(inScalarValue);
}

Float32
PAHP_LevelControl::ConverDBValueToScalarValue(Float32 inDBValue) const
{
	return volumeCurve.ConvertDBToScalar(inDBValue);
}

SInt32
PAHP_LevelControl::GetRawValue() const
{
	//	Always get the value from the hardware and cache it in currentRawValue. Note that if
	//	getting the value from the hardware fails for any reason, we just return currentRawValue.
	//	We always just return currentRawValue here because there is no hardware to talk to.
	return currentRawValue;
}

void
PAHP_LevelControl::SetRawValue(SInt32 inRawValue)
{
	//	Set the value in hardware. Note that currentRawValue should be updated only if setting the
	//	hardware value is synchronous. Otherwise, currentRawValue will be updated when the hardware
	//	notifies us that the value of the control changed. Here, we just directly set
	//	currentRawValue because there is no hardware.
	if (inRawValue != currentRawValue) {
		currentRawValue = inRawValue;

		//	we also have to send the change notification
		ValueChanged();
	}
}

void
PAHP_LevelControl::CacheRawValue()
{
	//	Set currentRawValue to the value of the hardware. We do nothing here because there is no
	//	hardware.
}

#pragma mark ### PAHP_BooleanControl ###

PAHP_BooleanControl::PAHP_BooleanControl(AudioObjectID inObjectID,
					 AudioClassID inClassID,
					 AudioObjectPropertyScope inDevicePropertyScope,
					 AudioObjectPropertyElement inDevicePropertyElement,
					 PAHP_PlugIn *inPlugIn,
					 PAHP_Device *inOwningDevice) :
	HP_BooleanControl(inObjectID, inClassID, inPlugIn, inOwningDevice),
	mDevicePropertyScope(inDevicePropertyScope),
	mDevicePropertyElement(inDevicePropertyElement),
	mCurrentValue(false)
{
}

PAHP_BooleanControl::~PAHP_BooleanControl()
{
}

void
PAHP_BooleanControl::Initialize()
{
	CacheValue();
}

void
PAHP_BooleanControl::Teardown()
{
}

AudioObjectPropertyScope
PAHP_BooleanControl::GetPropertyScope() const
{
	return mDevicePropertyScope;
}

AudioObjectPropertyElement
PAHP_BooleanControl::GetPropertyElement() const
{
	return mDevicePropertyElement;
}

bool
PAHP_BooleanControl::GetValue() const
{
	//	Always get the value from the hardware and cache it in mCurrentValue. Note that if
	//	getting the value from the hardware fails for any reason, we just return mCurrentValue.
	//	We always just return mCurrentValue here because there is no hardware to talk to.
	return mCurrentValue;
}

void
PAHP_BooleanControl::SetValue(bool inValue)
{
	//	Set the value in hardware. Note that mCurrentValue should be updated only if setting the
	//	hardware value is synchronous. Otherwise, mCurrentValue will be updated when the hardware
	//	notifies us that the value of the control changed. Here, we just directly set
	//	mCurrentValue because there is no hardware.
	if (inValue != mCurrentValue) {
		mCurrentValue = inValue;
		
		// we also have to send the change notification
		ValueChanged();
		
		CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
									    &kCFCopyStringDictionaryKeyCallBacks,
									    &kCFTypeDictionaryValueCallBacks);
		CFNumberRef value = CFNumberCreate(NULL, kCFNumberIntType, &inValue);
		CFDictionarySetValue(userInfo, CFSTR("value"), value);
		CFRelease(value);
		
		CFNotificationCenterPostNotification(CFNotificationCenterGetDistributedCenter(),
						     CFSTR("updateStreamMute"),
						     CFSTR("PAHP_BooleanControl"),
						     userInfo,
						     true);
		CFRelease(userInfo);
		
	}
}

void
PAHP_BooleanControl::CacheValue()
{
	//	Set mCurrentValue to the value of the hardware. We do nothing here because there is no hardware.
}

#pragma mark ### PAHP_SelectorControl ###

PAHP_SelectorControl::PAHP_SelectorControl(AudioObjectID inObjectID,
					   AudioClassID inClassID,
					   AudioObjectPropertyScope inDevicePropertyScope,
					   AudioObjectPropertyElement inDevicePropertyElement,
					   PAHP_PlugIn *inPlugIn,
					   PAHP_Device *inOwningDevice) :
	HP_SelectorControl(inObjectID, inClassID, inPlugIn, inOwningDevice),
	devicePropertyScope(inDevicePropertyScope),
	devicePropertyElement(inDevicePropertyElement),
	selectorMap(),
	currentItemID(0)
{
}

PAHP_SelectorControl::~PAHP_SelectorControl()
{
}

void
PAHP_SelectorControl::Initialize()
{
	//	clear the current items
	selectorMap.clear();

	//	Insert items into mSelectorMap for all the items in this control. Here, we just stick in a
	//	few fake items.
	for (UInt32 idx = 0; idx < 4; idx++) {
		//	make a name for the item
		CACFString name(CFStringCreateWithFormat(NULL, NULL, CFSTR("Item %u"), idx));

		//	insert it into the map, using the item index as the item ID
		selectorMap.insert(SelectorMap::value_type(idx, SelectorItem(name.CopyCFString(), 0)));
	}

	//	cache the current item ID
	CacheCurrentItemID();
}

void
PAHP_SelectorControl::Teardown()
{
	selectorMap.clear();
}

AudioObjectPropertyScope
PAHP_SelectorControl::GetPropertyScope() const
{
	return devicePropertyScope;
}

AudioObjectPropertyElement
PAHP_SelectorControl::GetPropertyElement() const
{
	return devicePropertyElement;
}

UInt32
PAHP_SelectorControl::GetNumberItems() const
{
	return selectorMap.size();
}

UInt32
PAHP_SelectorControl::GetCurrentItemID() const
{
	//	Always get the value from the hardware and cache it in mCurrentItemID. Note that if
	//	getting the value from the hardware fails for any reason, we just return mCurrentItemID.
	//	We always just return mCurrentItemID here because there is no hardware to talk to.
	return currentItemID;
}

UInt32
PAHP_SelectorControl::GetCurrentItemIndex() const
{
	return GetItemIndexForID(GetCurrentItemID());
}

void
PAHP_SelectorControl::SetCurrentItemByID(UInt32 inItemID)
{
	//	Set the value in hardware. Note that mCurrentItemID should be updated only if setting the
	//	hardware value is synchronous. Otherwise, mCurrentItemID will be updated when the hardware
	//	notifies us that the value of the control changed. Here, we just directly set
	//	mCurrentItemID because there is no hardware.
	if (inItemID != currentItemID) {
		currentItemID = inItemID;

		//	we also have to send the change notification
		ValueChanged();
	}
}

void
PAHP_SelectorControl::SetCurrentItemByIndex(UInt32 inItemIndex)
{
	SetCurrentItemByID(GetItemIDForIndex(inItemIndex));
}

UInt32
PAHP_SelectorControl::GetItemIDForIndex(UInt32 inItemIndex) const
{
	ThrowIf(inItemIndex >= selectorMap.size(),
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_SelectorControl::GetItemIDForIndex: index out of range");
	SelectorMap::const_iterator iterator = selectorMap.begin();
	std::advance(iterator, inItemIndex);
	return iterator->first;
}

UInt32	PAHP_SelectorControl::GetItemIndexForID(UInt32 inItemID) const
{
	UInt32 idx = 0;
	bool wasFound = false;
	SelectorMap::const_iterator iterator = selectorMap.begin();
	
	while (!wasFound && (iterator != selectorMap.end())) {
		if (iterator->first == inItemID) {
			wasFound = true;
		} else {
			idx++;
			std::advance(iterator, 1);
		}
	}

	ThrowIf(!wasFound,
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_SelectorControl::GetItemIndexForID: ID not in selector map");
	return idx;
}

CFStringRef
PAHP_SelectorControl::CopyItemNameByID(UInt32 inItemID) const
{
	SelectorMap::const_iterator iterator = selectorMap.find(inItemID);
	ThrowIf(iterator == selectorMap.end(),
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_SelectorControl::CopyItemNameByID: ID not in selector map");

	return (CFStringRef) CFRetain(iterator->second.mItemName);
}

CFStringRef
PAHP_SelectorControl::CopyItemNameByIndex(UInt32 inItemIndex) const
{
	if (inItemIndex < selectorMap.size()) {
		SelectorMap::const_iterator iterator = selectorMap.begin();
		std::advance(iterator, inItemIndex);
		ThrowIf(iterator == selectorMap.end(),
			CAException(kAudioHardwareIllegalOperationError),
			"PAHP_SelectorControl::CopyItemNameByIndex: index out of range");

		return (CFStringRef) CFRetain(iterator->second.mItemName);
	}

	return NULL;
}

CFStringRef
PAHP_SelectorControl::CopyItemNameByIDWithoutLocalizing(UInt32 inItemID) const
{
	return CopyItemNameByID(inItemID);
}

CFStringRef
PAHP_SelectorControl::CopyItemNameByIndexWithoutLocalizing(UInt32 inItemIndex) const
{
	return CopyItemNameByIndex(inItemIndex);
}

UInt32
PAHP_SelectorControl::GetItemKindByID(UInt32 inItemID) const
{
	SelectorMap::const_iterator iterator = selectorMap.find(inItemID);
	ThrowIf(iterator == selectorMap.end(),
		CAException(kAudioHardwareIllegalOperationError),
		"PAHP_SelectorControl::GetItemKindByID: ID not in selector map");

	return iterator->second.mItemKind;
}

UInt32
PAHP_SelectorControl::GetItemKindByIndex(UInt32 inItemIndex) const
{
	if (inItemIndex < selectorMap.size()) {
		SelectorMap::const_iterator iterator = selectorMap.begin();
		std::advance(iterator, inItemIndex);
		ThrowIf(iterator == selectorMap.end(),
			CAException(kAudioHardwareIllegalOperationError),
			"PAHP_SelectorControl::GetItemKindByIndex: index out of range");
		return iterator->second.mItemKind;
	}

	return 0;
}

void
PAHP_SelectorControl::CacheCurrentItemID()
{
	// Set mCurrentItemID to the value of the hardware. We do nothing here because there is no hardware.
}

#pragma mark ### PAHP_StereoPanControl ###

PAHP_StereoPanControl::PAHP_StereoPanControl(AudioObjectID inObjectID,
					     AudioClassID inClassID,
					     AudioObjectPropertyScope inDevicePropertyScope,
					     AudioObjectPropertyElement inDevicePropertyElement,
					     UInt32 inLeftChannel,
					     UInt32 inRightChannel,
					     PAHP_PlugIn *inPlugIn,
					     PAHP_Device *inOwningDevice) :
	HP_StereoPanControl(inObjectID, inClassID, inPlugIn, inOwningDevice),
	devicePropertyScope(inDevicePropertyScope),
	devicePropertyElement(inDevicePropertyElement),
	leftChannel(inLeftChannel),
	rightChannel(inRightChannel),
	fullLeftRawValue(0),
	centerRawValue(0),
	fullRightRawValue(0),
	currentRawValue(0)
{
}

PAHP_StereoPanControl::~PAHP_StereoPanControl()
{
}

void
PAHP_StereoPanControl::Initialize()
{
	// cache the info about the control
	fullLeftRawValue = 0;
	centerRawValue = 512;
	fullRightRawValue = 1024;
	
	// set the value to center, since we don't have any hardware
	currentRawValue = centerRawValue;

	// cache the current raw value
	CacheRawValue();
}

void
PAHP_StereoPanControl::Teardown()
{
}

AudioObjectPropertyScope
PAHP_StereoPanControl::GetPropertyScope() const
{
	return devicePropertyScope;
}

AudioObjectPropertyElement
PAHP_StereoPanControl::GetPropertyElement() const
{
	return devicePropertyElement;
}

Float32
PAHP_StereoPanControl::GetValue() const
{
	Float32	val = 0.0;
	SInt32	rawValue = GetRawValue();
	Float32	span;

	if (rawValue == centerRawValue) {
		val = 0.5;
	} else if (rawValue > centerRawValue) {
		span = fullRightRawValue - centerRawValue;
		val = rawValue - centerRawValue;
		val *= 0.5;
		val /= span;
		val += 0.5;
	} else {
		span = centerRawValue - fullLeftRawValue;
		val = rawValue - fullLeftRawValue;
		val *= 0.5;
		val /= span;
	}

	return val;
}

void
PAHP_StereoPanControl::SetValue(Float32 inValue)
{
	SInt32 rawValue = 0;
	Float32 span;

	if (inValue == 0.5) {
		rawValue = centerRawValue;
	} else if(inValue > 0.5) {
		span = fullRightRawValue - centerRawValue;
		inValue -= 0.5;
		inValue *= span;
		inValue *= 2.0;
		rawValue = static_cast<SInt32>(inValue);
		rawValue += centerRawValue;
	} else {
		span = centerRawValue - fullLeftRawValue;
		inValue *= span;
		inValue *= 2.0;
		rawValue = static_cast<SInt32>(inValue);
	}

	SetRawValue(rawValue);
}

void
PAHP_StereoPanControl::GetChannels(UInt32 &outLeftChannel,
				   UInt32 &outRightChannel) const
{
	outLeftChannel = leftChannel;
	outRightChannel = rightChannel;
}

SInt32
PAHP_StereoPanControl::GetRawValue() const
{
	//	Always get the value from the hardware and cache it in currentRawValue. Note that if
	//	getting the value from the hardware fails for any reason, we just return currentRawValue.
	//	We always just return currentRawValue here because there is no hardware to talk to.
	return currentRawValue;
}

void
PAHP_StereoPanControl::SetRawValue(SInt32 inValue)
{
	//	Set the value in hardware. Note that currentRawValue should be updated only if setting the
	//	hardware value is synchronous. Otherwise, currentRawValue will be updated when the hardware
	//	notifies us that the value of the control changed. Here, we just directly set
	//	currentRawValue because there is no hardware.
	if (inValue != currentRawValue) {
		currentRawValue = inValue;
		
		//	we also have to send the change notification
		ValueChanged();
	}
}

void
PAHP_StereoPanControl::CacheRawValue()
{
	//	Set currentRawValue to the value of the hardware. We do nothing here because there is no
	//	hardware.
}
