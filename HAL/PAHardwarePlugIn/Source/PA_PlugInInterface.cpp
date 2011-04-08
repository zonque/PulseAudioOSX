#include <CoreAudio/AudioHardwarePlugIn.h>
#include <pulse/pulseaudio.h>

#include "PA_PlugInInterface.h"

#define TraceCall(x) printf("%s() :%d\n", __func__, __LINE__);

// simple hack to extrapolate from a given pluginRef to our interface

static inline PA_Plugin *to_plugin(AudioHardwarePlugInRef ref)
{	
	PA_PlugInInterface *interface = (PA_PlugInInterface *) ref;
	UInt32 offset = ((Byte *) interface->GetInterface() - (Byte *) interface);
	interface = (PA_PlugInInterface *) ((Byte *) ref - offset);
	return interface->plugin;
}

// all these static functions are simply wrappers to call into the
// appropriate methods of our actual implementation (PA_Plugin)

static ULONG
Interface_AddRef(AudioHardwarePlugInRef inSelf)
{
	TraceCall();
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->AddRef();
}

static ULONG
Interface_Release(AudioHardwarePlugInRef inSelf)
{
	TraceCall();
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->Release();
}

static HRESULT
Interface_QueryInterface(AudioHardwarePlugInRef inSelf,
			 REFIID /* inUUID */,
			 LPVOID *outInterface)
{
	TraceCall();
	*outInterface = inSelf;
	return kAudioHardwareNoError;
}

static OSStatus
Interface_Initialize(AudioHardwarePlugInRef inSelf)
{
	TraceCall();
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->Initialize();
}

static OSStatus	Interface_InitializeWithObjectID(AudioHardwarePlugInRef inSelf, AudioObjectID inObjectID)
{
	TraceCall();

	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->InitializeWithObjectID(inObjectID);
}

static OSStatus
Interface_Teardown(AudioHardwarePlugInRef inSelf)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->Teardown();
}

#pragma mark ### AudioObject Operations ###

static void
Interface_ObjectShow(AudioHardwarePlugInRef inSelf,
		     AudioObjectID inObjectID)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	plugin->ObjectShow(inObjectID);
}

static Boolean
Interface_ObjectHasProperty(AudioHardwarePlugInRef inSelf,
			    AudioObjectID inObjectID,
			    const AudioObjectPropertyAddress *inAddress)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->ObjectHasProperty(inObjectID, inAddress);
}

static OSStatus
Interface_ObjectIsPropertySettable(AudioHardwarePlugInRef inSelf,
				   AudioObjectID inObjectID,
				   const AudioObjectPropertyAddress *inAddress,
				   Boolean *outIsSettable)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->ObjectIsPropertySettable(inObjectID, inAddress, outIsSettable);
}

static OSStatus
Interface_ObjectGetPropertyDataSize(AudioHardwarePlugInRef inSelf,
				    AudioObjectID inObjectID,
				    const AudioObjectPropertyAddress *inAddress,
				    UInt32 inQualifierDataSize,
				    const void *inQualifierData,
				    UInt32 *outDataSize)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->ObjectGetPropertyDataSize(inObjectID, inAddress,
						 inQualifierDataSize, inQualifierData, outDataSize);
}

static OSStatus
Interface_ObjectGetPropertyData(AudioHardwarePlugInRef inSelf,
				AudioObjectID inObjectID,
				const AudioObjectPropertyAddress *inAddress,
				UInt32 inQualifierDataSize,
				const void *inQualifierData,
				UInt32 *ioDataSize,
				void *outData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->ObjectGetPropertyData(inObjectID, inAddress,
					     inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

static OSStatus
Interface_ObjectSetPropertyData(AudioHardwarePlugInRef inSelf,
				AudioObjectID inObjectID,
				const AudioObjectPropertyAddress *inAddress,
				UInt32 inQualifierDataSize,
				const void *inQualifierData,
				UInt32 inDataSize,
				const void *inData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->ObjectSetPropertyData(inObjectID, inAddress,
					     inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### AudioDevice Operations ###

static OSStatus
Interface_DeviceCreateIOProcID(AudioHardwarePlugInRef inSelf,
			       AudioDeviceID inDeviceID,
			       AudioDeviceIOProc inProc,
			       void *inClientData,
			       AudioDeviceIOProcID *outIOProcID)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceCreateIOProcID(inDeviceID, inProc, inClientData, outIOProcID);	
}

static OSStatus
Interface_DeviceDestroyIOProcID(AudioHardwarePlugInRef inSelf,
				AudioDeviceID inDeviceID,
				AudioDeviceIOProcID inIOProcID)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceDestroyIOProcID(inDeviceID, inIOProcID);
}

static OSStatus
Interface_DeviceAddIOProc(AudioHardwarePlugInRef inSelf,
			  AudioDeviceID inDeviceID,
			  AudioDeviceIOProc inProc, 
			  void *inClientData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceAddIOProc(inDeviceID, inProc, inClientData);
}

static OSStatus
Interface_DeviceRemoveIOProc(AudioHardwarePlugInRef inSelf,
			     AudioDeviceID inDeviceID,
			     AudioDeviceIOProc inProc)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceRemoveIOProc(inDeviceID, inProc);
}

static OSStatus
Interface_DeviceStart(AudioHardwarePlugInRef inSelf,
		      AudioDeviceID inDeviceID,
		      AudioDeviceIOProc inProc)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceStart(inDeviceID, inProc);
}

static OSStatus
Interface_DeviceStartAtTime(AudioHardwarePlugInRef inSelf,
			    AudioDeviceID inDeviceID,
			    AudioDeviceIOProc inProc,
			    AudioTimeStamp *ioRequestedStartTime,
			    UInt32 inFlags)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceStartAtTime(inDeviceID, inProc, ioRequestedStartTime, inFlags);
}

static OSStatus
Interface_DeviceStop(AudioHardwarePlugInRef inSelf,
		     AudioDeviceID inDeviceID,
		     AudioDeviceIOProc inProc)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceStop(inDeviceID, inProc);
}

static OSStatus
Interface_DeviceRead(AudioHardwarePlugInRef inSelf,
		     AudioDeviceID inDeviceID,
		     const AudioTimeStamp *inStartTime,
		     AudioBufferList *outData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceRead(inDeviceID, inStartTime, outData);
}

static OSStatus
Interface_DeviceGetCurrentTime(AudioHardwarePlugInRef inSelf,
			       AudioDeviceID inDeviceID,
			       AudioTimeStamp *outTime)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceGetCurrentTime(inDeviceID, outTime);
}

static OSStatus	Interface_DeviceTranslateTime(AudioHardwarePlugInRef inSelf,
					      AudioDeviceID inDeviceID,
					      const AudioTimeStamp *inTime,
					      AudioTimeStamp *outTime)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceTranslateTime(inDeviceID, inTime, outTime);
}

static OSStatus
Interface_DeviceGetNearestStartTime(AudioHardwarePlugInRef inSelf,
				    AudioDeviceID inDeviceID,
				    AudioTimeStamp *ioRequestedStartTime,
				    UInt32 inFlags)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceGetNearestStartTime(inDeviceID, ioRequestedStartTime, inFlags);
}

static OSStatus
Interface_DeviceGetPropertyInfo(AudioHardwarePlugInRef inSelf,
				AudioDeviceID inDeviceID,
				UInt32 inChannel,
				Boolean isInput,
				AudioDevicePropertyID inPropertyID,
				UInt32 *outSize,
				Boolean *outWritable)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceGetPropertyInfo(inDeviceID, inChannel, isInput,
					     inPropertyID, outSize, outWritable);
}

static OSStatus
Interface_DeviceGetProperty(AudioHardwarePlugInRef inSelf,
			    AudioDeviceID inDeviceID,
			    UInt32 inChannel,
			    Boolean isInput,
			    AudioDevicePropertyID inPropertyID,
			    UInt32 *ioPropertyDataSize,
			    void *outPropertyData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceGetProperty(inDeviceID, inChannel, isInput,
					 inPropertyID, ioPropertyDataSize, outPropertyData);
}

static OSStatus
Interface_DeviceSetProperty(AudioHardwarePlugInRef inSelf,
			    AudioDeviceID inDeviceID,
			    const AudioTimeStamp *inWhen,
			    UInt32 inChannel,
			    Boolean isInput,
			    AudioDevicePropertyID inPropertyID,
			    UInt32 inPropertyDataSize,
			    const void *inPropertyData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->DeviceSetProperty(inDeviceID, inWhen, inChannel, isInput,
					 inPropertyID, inPropertyDataSize, inPropertyData);
}

#pragma mark ### AudioStream Operations ###

static OSStatus
Interface_StreamGetPropertyInfo(AudioHardwarePlugInRef inSelf,
				AudioStreamID inStreamID,
				UInt32 inChannel,
				AudioDevicePropertyID inPropertyID,
				UInt32 *outSize,
				Boolean *outWritable)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->StreamGetPropertyInfo(inStreamID, inChannel,
					     inPropertyID, outSize, outWritable);	
}

static OSStatus
Interface_StreamGetProperty(AudioHardwarePlugInRef inSelf,
			    AudioStreamID inStreamID,
			    UInt32 inChannel,
			    AudioDevicePropertyID inPropertyID,
			    UInt32 *ioPropertyDataSize,
			    void *outPropertyData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->StreamGetProperty(inStreamID, inChannel,
					 inPropertyID, ioPropertyDataSize, outPropertyData);	
}

static OSStatus
Interface_StreamSetProperty(AudioHardwarePlugInRef inSelf,
			    AudioStreamID inStreamID,
			    const AudioTimeStamp *inWhen,
			    UInt32 inChannel,
			    AudioDevicePropertyID inPropertyID,
			    UInt32 inPropertyDataSize,
			    const void *inPropertyData)
{
	PA_Plugin *plugin = to_plugin(inSelf);
	return plugin->StreamSetProperty(inStreamID, inWhen, inChannel,
					 inPropertyID, inPropertyDataSize, inPropertyData);	
}

PA_PlugInInterface::PA_PlugInInterface()
{
	staticInterface = pa_xnew0(AudioHardwarePlugInInterface, 1);

	//	Padding for COM
	staticInterface->_reserved	= NULL,

	//	IUnknown Routines
	staticInterface->QueryInterface		= (HRESULT (*)(void*, CFUUIDBytes, void**)) Interface_QueryInterface;
	staticInterface->AddRef			= (ULONG (*)(void*)) Interface_AddRef;
	staticInterface->Release		= (ULONG (*)(void*)) Interface_Release;

	//	HAL Plug-In Routines
	staticInterface->Initialize			= Interface_Initialize;
	staticInterface->Teardown			= Interface_Teardown;
	staticInterface->DeviceAddIOProc		= Interface_DeviceAddIOProc;
	staticInterface->DeviceRemoveIOProc		= Interface_DeviceRemoveIOProc;
	staticInterface->DeviceStart			= Interface_DeviceStart;
	staticInterface->DeviceStop			= Interface_DeviceStop;
	staticInterface->DeviceRead			= Interface_DeviceRead;
	staticInterface->DeviceGetCurrentTime		= Interface_DeviceGetCurrentTime;
	staticInterface->DeviceTranslateTime		= Interface_DeviceTranslateTime;
	staticInterface->DeviceGetPropertyInfo		= Interface_DeviceGetPropertyInfo;
	staticInterface->DeviceGetProperty		= Interface_DeviceGetProperty;
	staticInterface->DeviceSetProperty		= Interface_DeviceSetProperty;
	staticInterface->StreamGetPropertyInfo		= Interface_StreamGetPropertyInfo;
	staticInterface->StreamGetProperty		= Interface_StreamGetProperty;
	staticInterface->StreamSetProperty		= Interface_StreamSetProperty;
	staticInterface->DeviceStartAtTime		= Interface_DeviceStartAtTime;
	staticInterface->DeviceGetNearestStartTime	= Interface_DeviceGetNearestStartTime;
	staticInterface->InitializeWithObjectID		= Interface_InitializeWithObjectID;
	staticInterface->ObjectShow			= Interface_ObjectShow;
	staticInterface->ObjectHasProperty		= Interface_ObjectHasProperty;
	staticInterface->ObjectIsPropertySettable	= Interface_ObjectIsPropertySettable;
	staticInterface->ObjectGetPropertyDataSize	= Interface_ObjectGetPropertyDataSize;
	staticInterface->ObjectGetPropertyData		= Interface_ObjectGetPropertyData;
	staticInterface->ObjectSetPropertyData		= Interface_ObjectSetPropertyData;
	staticInterface->DeviceCreateIOProcID		= Interface_DeviceCreateIOProcID;
	staticInterface->DeviceDestroyIOProcID		= Interface_DeviceDestroyIOProcID;

	plugin = new PA_Plugin(GetInterface());
}

PA_PlugInInterface::~PA_PlugInInterface()
{
	delete plugin;
	pa_xfree(staticInterface);
}

extern "C" void*
New_PAHP_PlugIn(CFAllocatorRef * /* allocator */, CFUUIDRef requestedTypeUUID) 
{
	if (CFEqual(requestedTypeUUID, kAudioHardwarePlugInTypeID)) {
		PA_PlugInInterface *interface = new PA_PlugInInterface();
		return interface->GetInterface();
	}

	return NULL;
}

