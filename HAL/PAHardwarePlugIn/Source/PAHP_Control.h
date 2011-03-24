#ifndef _PAHP_CONTROL_H_
#define _PAHP_CONTROL_H_

#include "HP_Control.h"
#include "CAVolumeCurve.h"

class PAHP_Device;
class PAHP_PlugIn;

class PAHP_LevelControl : public HP_LevelControl
{
public:
						PAHP_LevelControl(AudioObjectID inObjectID,
								  AudioClassID inClassID,
								  AudioObjectPropertyScope inDevicePropertyScope,
								  AudioObjectPropertyElement inDevicePropertyElement,
								  PAHP_PlugIn *inPlugIn,
								  PAHP_Device *inOwningDevice);
	virtual					~PAHP_LevelControl();
	
	virtual void				Initialize();
	virtual void				Teardown();

public:
	virtual AudioObjectPropertyScope	GetPropertyScope() const;
	virtual AudioObjectPropertyElement	GetPropertyElement() const;

	virtual Float32				GetMinimumDBValue() const;
	virtual Float32				GetMaximumDBValue() const;

	virtual Float32				GetDBValue() const;
	virtual void				SetDBValue(Float32 inDBValue);

	virtual Float32				GetScalarValue() const;
	virtual void				SetScalarValue(Float32 inScalarValue);

	virtual Float32				ConverScalarValueToDBValue(Float32 inScalarValue) const;
	virtual Float32				ConverDBValueToScalarValue(Float32 inDBValue) const;
	
private:
	SInt32					GetRawValue() const;
	void					SetRawValue(SInt32 inRawValue);
	void					CacheRawValue();

	AudioObjectPropertyScope		devicePropertyScope;
	AudioObjectPropertyElement		devicePropertyElement;
	CAVolumeCurve				volumeCurve;
	SInt32					currentRawValue;

};

#pragma mark ### PAHP_BooleanControl ###

class PAHP_BooleanControl : public HP_BooleanControl
{
public:
	PAHP_BooleanControl(AudioObjectID		inObjectID,
			    AudioClassID		inClassID,
			    AudioObjectPropertyScope	inDevicePropertyScope,
			    AudioObjectPropertyElement	inDevicePropertyElement,
			    PAHP_PlugIn			*inPlugIn,
			    PAHP_Device			*inOwningDevice);
	virtual					~PAHP_BooleanControl();

	virtual void				Initialize();
	virtual void				Teardown();

//	Attributes
public:
	virtual AudioObjectPropertyScope	GetPropertyScope() const;
	virtual AudioObjectPropertyElement	GetPropertyElement() const;

	virtual bool				GetValue() const;
	virtual void				SetValue(bool inValue);

//	Implementation
private:
	virtual void				CacheValue();

	AudioObjectPropertyScope		mDevicePropertyScope;
	AudioObjectPropertyElement		mDevicePropertyElement;
	bool					mCurrentValue;

};

#pragma mark ### PAHP_SelectorControl ###

class PAHP_SelectorControl : public HP_SelectorControl
{
public:
						PAHP_SelectorControl(AudioObjectID		inObjectID,
								     AudioClassID		inClassID,
								     AudioObjectPropertyScope	inDevicePropertyScope,
								     AudioObjectPropertyElement	inDevicePropertyElement,
								     PAHP_PlugIn		*inPlugIn,
								     PAHP_Device		*inOwningDevice);
	virtual					~PAHP_SelectorControl();
	
	virtual void				Initialize();
	virtual void				Teardown();

public:
	virtual AudioObjectPropertyScope	GetPropertyScope() const;
	virtual AudioObjectPropertyElement	GetPropertyElement() const;

	virtual UInt32				GetNumberItems() const;

	virtual UInt32				GetCurrentItemID() const;
	virtual UInt32				GetCurrentItemIndex() const;
	
	virtual void				SetCurrentItemByID(UInt32 inItemID);
	virtual void				SetCurrentItemByIndex(UInt32 inItemIndex);
	
	virtual UInt32				GetItemIDForIndex(UInt32 inItemIndex) const;
	virtual UInt32				GetItemIndexForID(UInt32 inItemID) const;
	
	virtual CFStringRef			CopyItemNameByID(UInt32 inItemID) const;
	virtual CFStringRef			CopyItemNameByIndex(UInt32 inItemIndex) const;

	virtual CFStringRef			CopyItemNameByIDWithoutLocalizing(UInt32 inItemID) const;
	virtual CFStringRef			CopyItemNameByIndexWithoutLocalizing(UInt32 inItemIndex) const;

	virtual UInt32				GetItemKindByID(UInt32 inItemID) const;
	virtual UInt32				GetItemKindByIndex(UInt32 inItemIndex) const;

private:
	void					CacheCurrentItemID();
	
	struct SelectorItem
	{
		CFStringRef			mItemName;
		UInt32				mItemKind;
		
		SelectorItem() : mItemName(NULL), mItemKind(0) {}
		SelectorItem(CFStringRef inItemName, UInt32 inItemKind) : mItemName(inItemName), mItemKind(inItemKind) {}
		SelectorItem(const SelectorItem& inItem) : mItemName(inItem.mItemName), mItemKind(inItem.mItemKind) {
			if (mItemName)
				CFRetain(mItemName);
		}
		SelectorItem&	operator=(const SelectorItem& inItem) {
			if (mItemName)
				CFRelease(mItemName);
			
			mItemName = inItem.mItemName;
			
			if (mItemName)
				CFRetain(mItemName);
			
			mItemKind = inItem.mItemKind;
			return *this;
		}
		~SelectorItem() {
			if (mItemName)
				CFRelease(mItemName);
		}
	};
	typedef std::map<UInt32, SelectorItem>	SelectorMap;
	
	AudioObjectPropertyScope		devicePropertyScope;
	AudioObjectPropertyElement		devicePropertyElement;
	SelectorMap				selectorMap;
	UInt32					currentItemID;

};

//==================================================================================================
//	PAHP_StereoPanControl
//==================================================================================================

class PAHP_StereoPanControl : public HP_StereoPanControl
{
public:
						PAHP_StereoPanControl(AudioObjectID			inObjectID,
								      AudioClassID			inClassID,
								      AudioObjectPropertyScope		inDevicePropertyScope,
								      AudioObjectPropertyElement	inDevicePropertyElement,
								      UInt32				inLeftChannel,
								      UInt32				inRightChannel,
								      PAHP_PlugIn			*inPlugIn,
								      PAHP_Device			*inOwningDevice);
	virtual					~PAHP_StereoPanControl();

	virtual void				Initialize();
	virtual void				Teardown();

public:
	virtual AudioObjectPropertyScope	GetPropertyScope() const;
	virtual AudioObjectPropertyElement	GetPropertyElement() const;

	virtual Float32				GetValue() const;
	virtual void				SetValue(Float32 inValue);
	virtual void				GetChannels(UInt32& outLeftChannel, UInt32& outRightChannel) const;

private:
	virtual SInt32				GetRawValue() const;
	virtual void				SetRawValue(SInt32 inValue);
	virtual void				CacheRawValue();
	
	AudioObjectPropertyScope		devicePropertyScope;
	AudioObjectPropertyElement		devicePropertyElement;
	UInt32					leftChannel;
	UInt32					rightChannel;
	SInt32					fullLeftRawValue;
	SInt32					centerRawValue;
	SInt32					fullRightRawValue;
	SInt32					currentRawValue;

};

#endif /* _PAHP_CONTROL_H_ */
