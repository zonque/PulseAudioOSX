#include "PAHP_Stream.h"
#include "PAHP_Device.h"
#include "PAHP_PlugIn.h"

#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CACFNumber.h"
#include "CADebugMacros.h"
#include "CAException.h"

PAHP_Stream::PAHP_Stream(AudioStreamID inAudioStreamID,
			 PAHP_PlugIn *inPlugIn,
			 PAHP_Device *inOwningDevice,
			 bool inIsInput,
			 UInt32 inStartingDeviceChannelNumber) :
	HP_Stream(inAudioStreamID, inPlugIn, inOwningDevice, inIsInput, inStartingDeviceChannelNumber),
	plugin(inPlugIn),
	owningDevice(inOwningDevice),
	mNonMixableFormatSet(false)
{
}

PAHP_Stream::~PAHP_Stream()
{
}

void
PAHP_Stream::Initialize()
{
	HP_Stream::Initialize();

	AddAvailablePhysicalFormats();

	AudioStreamBasicDescription physicalFormat;
	physicalFormat.mSampleRate = 44100;
	physicalFormat.mFormatID = kAudioFormatLinearPCM;
	physicalFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger	|
					 kAudioFormatFlagsNativeEndian		|
					 kAudioFormatFlagIsPacked;
	physicalFormat.mBytesPerPacket = 4;
	physicalFormat.mFramesPerPacket = 1;
	physicalFormat.mBytesPerFrame = 4;
	physicalFormat.mChannelsPerFrame = 2;
	physicalFormat.mBitsPerChannel = 16;
	mFormatList->SetCurrentPhysicalFormat(physicalFormat, false);
}

void
PAHP_Stream::Teardown()
{
	// All we need to do here is make sure that if this app set the format to non-mixable, we
	// restore it to a mixable format that is closest to the current format.
	if (mNonMixableFormatSet) {
		// get the current format
		AudioStreamBasicDescription mixableFormat;
		mFormatList->GetCurrentPhysicalFormat(mixableFormat);

		// find the closest mixable format
		if(mixableFormat.mFormatID == kAudioFormatLinearPCM) {
			// for linear PCM formats, we just clear the flag
			mixableFormat.mFormatFlags &= ~kAudioFormatFlagIsNonMixable;
		} else {
			// for non-linear PCM formats, we just need to find the best available linear PCM
			// format with the same sample rate
			mixableFormat.mFormatID = kAudioFormatLinearPCM;
			mixableFormat.mFormatFlags = 0;
			mixableFormat.mBytesPerPacket = 0;
			mixableFormat.mFramesPerPacket = 1;
			mixableFormat.mBytesPerFrame = 0;
			mixableFormat.mChannelsPerFrame = 0;
			mixableFormat.mBitsPerChannel = 0;

			// ask the format list for the best match
			mFormatList->BestMatchForPhysicalFormat(mixableFormat);
		}

		// ask the format list for the best match
		mFormatList->BestMatchForPhysicalFormat(mixableFormat);

		// tell the hardware stream to set the format
		TellHardwareToSetPhysicalFormat(mixableFormat);
	}

	HP_Stream::Teardown();
}

void
PAHP_Stream::Finalize()
{
	// Finalize() is called in place of Teardown() when we're being lazy about
	// cleaning up. The idea is to do as little work as possible here.

	if (!mNonMixableFormatSet)
		return;

	// All we need to do here is make sure that if this app set the format to non-mixable, we
	// restore it to a mixable format that is closest to the current format.

	AudioStreamBasicDescription mixableFormat;
	mFormatList->GetCurrentPhysicalFormat(mixableFormat);

	// find the closest mixable format
	if (mixableFormat.mFormatID == kAudioFormatLinearPCM) {
		// for linear PCM formats, we just clear the flag
		mixableFormat.mFormatFlags &= ~kAudioFormatFlagIsNonMixable;
	} else {
		// for non-linear PCM formats, we just need to find the best available linear PCM
		// format with the same sample rate
		mixableFormat.mFormatID = kAudioFormatLinearPCM;
		mixableFormat.mFormatFlags = 0;
		mixableFormat.mBytesPerPacket = 0;
		mixableFormat.mFramesPerPacket = 1;
		mixableFormat.mBytesPerFrame = 0;
		mixableFormat.mChannelsPerFrame = 0;
		mixableFormat.mBitsPerChannel = 0;

		//	ask the format list for the best match
		mFormatList->BestMatchForPhysicalFormat(mixableFormat);
	}

	// ask the format list for the best match
	mFormatList->BestMatchForPhysicalFormat(mixableFormat);

	// tell the hardware stream to set the format
	TellHardwareToSetPhysicalFormat(mixableFormat);
}

bool
PAHP_Stream::HasProperty(const AudioObjectPropertyAddress &inAddress) const
{
	CAMutex::Locker stateMutex(GetOwningDevice()->GetDeviceStateMutex());

	// do the work if we still have to
	switch (inAddress.mSelector) {
		default:
			return HP_Stream::HasProperty(inAddress);
	}

	return false;
}

bool
PAHP_Stream::IsPropertySettable(const AudioObjectPropertyAddress &inAddress) const
{
	CAMutex::Locker stateMutex(GetOwningDevice()->GetDeviceStateMutex());

	switch (inAddress.mSelector) {
		default:
			return HP_Stream::IsPropertySettable(inAddress);
	}

	return false;
}

UInt32
PAHP_Stream::GetPropertyDataSize(const AudioObjectPropertyAddress &inAddress,
				 UInt32 inQualifierDataSize,
				 const void *inQualifierData) const
{
	CAMutex::Locker stateMutex(GetOwningDevice()->GetDeviceStateMutex());

	switch (inAddress.mSelector) {
		default:
			return HP_Stream::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData);
	}

	return 0;
}

void
PAHP_Stream::GetPropertyData(const AudioObjectPropertyAddress &inAddress,
			     UInt32 inQualifierDataSize,
			     const void *inQualifierData,
			     UInt32 &ioDataSize,
			     void *outData) const
{
	CAMutex::Locker stateMutex(GetOwningDevice()->GetDeviceStateMutex());

	switch (inAddress.mSelector) {
		default:
			HP_Stream::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
			break;
	};
}

void
PAHP_Stream::SetPropertyData(const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData, const AudioTimeStamp* inWhen)
{
	ThrowIf(!owningDevice->HogModeIsOwnedBySelfOrIsFree(),
		CAException(kAudioDevicePermissionsError),
		"PAHP_Stream::SetPropertyData: can't set the property because hog mode is owned by another process");

	CAMutex::Locker stateMutex(GetOwningDevice()->GetDeviceStateMutex());

	bool newIsMixable;
	AudioStreamBasicDescription newFormat;
	const AudioStreamBasicDescription *formatDataPtr = static_cast<const AudioStreamBasicDescription*>(inData);

	switch (inAddress.mSelector) {
		//  device properties
		case kAudioDevicePropertySupportsMixing:
			ThrowIf(inDataSize != sizeof(UInt32),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Stream::SetPropertyData: wrong data size for kAudioDevicePropertySupportsMixing");
			newIsMixable = *(static_cast<const UInt32*>(inData)) != 0;

			//	keep track if this process is setting the format to non-mixable
			if(newIsMixable)
				mNonMixableFormatSet = false;
			else
				mNonMixableFormatSet = true;

			//	set the new format
			mFormatList->SetIsMixable(newIsMixable, true);
			break;

		//  stream properties
		case kAudioStreamPropertyVirtualFormat:
			//  aka kAudioDevicePropertyStreamFormat
			ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Stream::SetPropertyData: wrong data size for kAudioStreamPropertyVirtualFormat");

			// make a modifiable copy
			newFormat = *formatDataPtr;

			// screen the format
			ThrowIf(!mFormatList->SanityCheckVirtualFormat(newFormat),
				CAException(kAudioDeviceUnsupportedFormatError),
				"PAHP_Stream::SetPropertyData: given format is not supported for kAudioStreamPropertyVirtualFormat");

			// look for a best match to what was asked for
			mFormatList->BestMatchForVirtualFormat(newFormat);

			// keep track if this process is setting the format to non-mixable
			mNonMixableFormatSet = !CAStreamBasicDescription::IsMixable(newFormat);

			// set the new format
			mFormatList->SetCurrentVirtualFormat(newFormat, true);
			break;

		case kAudioStreamPropertyPhysicalFormat:
			ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription),
				CAException(kAudioHardwareBadPropertySizeError),
				"PAHP_Stream::SetPropertyData: wrong data size for kAudioStreamPropertyPhysicalFormat");

			// make a modifiable copy
			newFormat = *formatDataPtr;

			// screen the format
			ThrowIf(!mFormatList->SanityCheckPhysicalFormat(newFormat),
				CAException(kAudioDeviceUnsupportedFormatError),
				"PAHP_Stream::SetPropertyData: given format is not supported for kAudioStreamPropertyPhysicalFormat");

			// look for a best match to what was asked for
			mFormatList->BestMatchForPhysicalFormat(newFormat);

			// keep track if this process is setting the format to non-mixable
			mNonMixableFormatSet = !CAStreamBasicDescription::IsMixable(newFormat);

			// set the new format
			mFormatList->SetCurrentPhysicalFormat(newFormat, true);
			break;

		default:
			HP_Stream::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData, inWhen);
			break;
	}
}

bool
PAHP_Stream::TellHardwareToSetPhysicalFormat(const AudioStreamBasicDescription& /*inFormat*/)
{
	// this method is called to tell the hardware to change format. It returns true if the format
	// change took place immediately, which is the casee for this sample device.
	return true;
}

void
PAHP_Stream::RefreshAvailablePhysicalFormats()
{
	mFormatList->RemoveAllFormats();
	AddAvailablePhysicalFormats();

	CAPropertyAddressList changedProperties;
	CAPropertyAddress address(kAudioStreamPropertyAvailablePhysicalFormats);
	changedProperties.AppendUniqueItem(address);
	address.mSelector = kAudioStreamPropertyAvailableVirtualFormats;
	changedProperties.AppendUniqueItem(address);
	address.mSelector = kAudioStreamPropertyPhysicalFormats;
	changedProperties.AppendUniqueItem(address);
	address.mSelector = kAudioDevicePropertyStreamFormats;
	changedProperties.AppendUniqueItem(address);
	PropertiesChanged(changedProperties.GetNumberItems(), changedProperties.GetItems());
}

void
PAHP_Stream::AddAvailablePhysicalFormats()
{
	// basically, for this sample device, we're only going add two formats
	AudioStreamRangedDescription physicalFormat;

	// the first is 16 bit stereo
	physicalFormat.mFormat.mSampleRate = 44100;
	physicalFormat.mSampleRateRange.mMinimum = 44100;
	physicalFormat.mSampleRateRange.mMaximum = 44100;
	physicalFormat.mFormat.mFormatID = kAudioFormatLinearPCM;
	physicalFormat.mFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger	|
						 kAudioFormatFlagsNativeEndian		|
						 kAudioFormatFlagIsPacked;
	physicalFormat.mFormat.mBytesPerPacket = 4;
	physicalFormat.mFormat.mFramesPerPacket = 1;
	physicalFormat.mFormat.mBytesPerFrame = 4;
	physicalFormat.mFormat.mChannelsPerFrame = 2;
	physicalFormat.mFormat.mBitsPerChannel = 16;
	mFormatList->AddPhysicalFormat(physicalFormat);

	// the other is 24 bit packed in 32 bit stereo
	physicalFormat.mFormat.mSampleRate = 44100;
	physicalFormat.mSampleRateRange.mMinimum = 44100;
	physicalFormat.mSampleRateRange.mMaximum = 44100;
	physicalFormat.mFormat.mFormatID = kAudioFormatLinearPCM;
	physicalFormat.mFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsAlignedHigh;
	physicalFormat.mFormat.mBytesPerPacket = 8;
	physicalFormat.mFormat.mFramesPerPacket = 1;
	physicalFormat.mFormat.mBytesPerFrame = 8;
	physicalFormat.mFormat.mChannelsPerFrame = 2;
	physicalFormat.mFormat.mBitsPerChannel = 24;
	mFormatList->AddPhysicalFormat(physicalFormat);
}
