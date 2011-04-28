/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <CoreAudio/AudioHardwarePlugIn.h>

#import "PAPlugInInterface.h"

#define TraceCall(x) printf("%s() :%d\n", __func__, __LINE__);

// simple hack to extrapolate from a given pluginRef to our interface

static inline PAPlugin *to_plugin(AudioHardwarePlugInRef ref)
{
	PAPlugInInterface *interface = (PAPlugInInterface *) ref;
	UInt32 offset = ((Byte *) &interface->staticInterface - (Byte *) interface);
	interface = (PAPlugInInterface *) ((Byte *) ref - offset);	
	return interface.plugin;
}

@implementation PAPlugInInterface

@synthesize plugin;

// all these static functions are simply wrappers to call into the
// appropriate methods of our actual implementation (PAPlugin)

static ULONG
Interface_AddRef(AudioHardwarePlugInRef inSelf)
{
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin retain];
	return [plugin retainCount];
}

static ULONG
Interface_Release(AudioHardwarePlugInRef inSelf)
{
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin release];
	return [plugin retainCount];
}

static HRESULT
Interface_QueryInterface(AudioHardwarePlugInRef inSelf,
			 REFIID inUUID,
			 LPVOID *outInterface)
{
	TraceCall();
	*outInterface = inSelf;
	
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin retain];
	
	return kAudioHardwareNoError;
}

static OSStatus
Interface_Initialize(AudioHardwarePlugInRef inSelf)
{
	TraceCall();
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin retain];
	return [plugin initialize];
}

static OSStatus
Interface_InitializeWithObjectID(AudioHardwarePlugInRef inSelf, AudioObjectID inObjectID)
{
	TraceCall();
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin retain];
	return [plugin initializeWithObjectID: inObjectID];
}

static OSStatus
Interface_Teardown(AudioHardwarePlugInRef inSelf)
{
	TraceCall();
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin release];
	return kAudioHardwareNoError;
}

#pragma mark ### AudioObject Operations ###

static void
Interface_ObjectShow(AudioHardwarePlugInRef inSelf,
		     AudioObjectID inObjectID)
{
	PAPlugin *plugin = to_plugin(inSelf);
	[plugin objectShow: inObjectID];
}

static Boolean
Interface_ObjectHasProperty(AudioHardwarePlugInRef inSelf,
			    AudioObjectID inObjectID,
			    const AudioObjectPropertyAddress *inAddress)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin objectHasProperty: inObjectID
			      propertyID: inAddress];
}

static OSStatus
Interface_ObjectIsPropertySettable(AudioHardwarePlugInRef inSelf,
				   AudioObjectID inObjectID,
				   const AudioObjectPropertyAddress *inAddress,
				   Boolean *outIsSettable)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin objectIsPropertySettable: inObjectID
				       propertyID: inAddress
			  outIsPropertySettable: (BOOL *) outIsSettable];
}

static OSStatus
Interface_ObjectGetPropertyDataSize(AudioHardwarePlugInRef inSelf,
				    AudioObjectID inObjectID,
				    const AudioObjectPropertyAddress *inAddress,
				    UInt32 inQualifierDataSize,
				    const void *inQualifierData,
				    UInt32 *outDataSize)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin objectGetPropertyDataSize: inObjectID
				      propertyID: inAddress
			       qualifierDataSize: inQualifierDataSize
				   qualifierData: inQualifierData
			     outPropertyDataSize: outDataSize];
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin objectGetPropertyData: inObjectID
				  propertyID: inAddress
			   qualifierDataSize: inQualifierDataSize
			       qualifierData: inQualifierData
				  ioDataSize: ioDataSize
				     outData: outData];
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin objectSetPropertyData: inObjectID
				  propertyID: inAddress
			   qualifierDataSize: inQualifierDataSize
			       qualifierData: inQualifierData
				    dataSize: inDataSize
					data: inData];
}

#pragma mark ### AudioDevice Operations ###

static OSStatus
Interface_DeviceCreateIOProcID(AudioHardwarePlugInRef inSelf,
			       AudioDeviceID inDeviceID,
			       AudioDeviceIOProc inProc,
			       void *inClientData,
			       AudioDeviceIOProcID *outIOProcID)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceCreateIOProcID: inDeviceID
				       proc: inProc
				 clientData: inClientData
				outIOProcID: outIOProcID];
}

static OSStatus
Interface_DeviceDestroyIOProcID(AudioHardwarePlugInRef inSelf,
				AudioDeviceID inDeviceID,
				AudioDeviceIOProcID inIOProcID)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceDestroyIOProcID: inDeviceID
				      procID: inIOProcID];
}

static OSStatus
Interface_DeviceAddIOProc(AudioHardwarePlugInRef inSelf,
			  AudioDeviceID inDeviceID,
			  AudioDeviceIOProc inProc, 
			  void *inClientData)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceAddIOProc: inDeviceID
				  proc: inProc
			    clientData: inClientData];
}

static OSStatus
Interface_DeviceRemoveIOProc(AudioHardwarePlugInRef inSelf,
			     AudioDeviceID inDeviceID,
			     AudioDeviceIOProc inProc)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceRemoveIOProc: inDeviceID
				     proc: inProc];
}

static OSStatus
Interface_DeviceStart(AudioHardwarePlugInRef inSelf,
		      AudioDeviceID inDeviceID,
		      AudioDeviceIOProcID inProcID)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceStart: inDeviceID
			    procID: inProcID];
}

static OSStatus
Interface_DeviceStartAtTime(AudioHardwarePlugInRef inSelf,
			    AudioDeviceID inDeviceID,
			    AudioDeviceIOProcID inProcID,
			    AudioTimeStamp *ioRequestedStartTime,
			    UInt32 inFlags)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceStartAtTime: inDeviceID
				  procID: inProcID
		      requestedStartTime: ioRequestedStartTime
				   flags: inFlags];
}

static OSStatus
Interface_DeviceStop(AudioHardwarePlugInRef inSelf,
		     AudioDeviceID inDeviceID,
		     AudioDeviceIOProcID inProcID)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceStop: inDeviceID
			   procID: inProcID];
}

static OSStatus
Interface_DeviceRead(AudioHardwarePlugInRef inSelf,
		     AudioDeviceID inDeviceID,
		     const AudioTimeStamp *inStartTime,
		     AudioBufferList *outData)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceRead: inDeviceID
			startTime: inStartTime
			  outData: outData];
}

static OSStatus
Interface_DeviceGetCurrentTime(AudioHardwarePlugInRef inSelf,
			       AudioDeviceID inDeviceID,
			       AudioTimeStamp *outTime)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceGetCurrentTime: inDeviceID
				    outTime: outTime];
}

static OSStatus	Interface_DeviceTranslateTime(AudioHardwarePlugInRef inSelf,
					      AudioDeviceID inDeviceID,
					      const AudioTimeStamp *inTime,
					      AudioTimeStamp *outTime)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceTranslateTime: inDeviceID
				    inTime: inTime
				   outTime: outTime];
}

static OSStatus
Interface_DeviceGetNearestStartTime(AudioHardwarePlugInRef inSelf,
				    AudioDeviceID inDeviceID,
				    AudioTimeStamp *ioRequestedStartTime,
				    UInt32 inFlags)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceGetNearestStartTime: inDeviceID
			      requestedStartTime: ioRequestedStartTime
					   flags: inFlags];
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceGetPropertyInfo: inDeviceID
				     channel: inChannel
				     isInput: isInput
				    propertyID: inPropertyID
			     outSize: outSize
			 outIsWritable: (BOOL *) outWritable];
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceGetProperty: inDeviceID
				 channel: inChannel
				 isInput: !!isInput
			      propertyID: inPropertyID
			      ioDataSize: ioPropertyDataSize
				 outData: outPropertyData];
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin deviceSetProperty: inDeviceID
				    when: inWhen
				 channel: inChannel
				 isInput: !!isInput
			      propertyID: inPropertyID
			dataSize: inPropertyDataSize
			    data: inPropertyData];	
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin streamGetPropertyInfo: inStreamID
				     channel: inChannel
				  propertyID: inPropertyID
				     outSize: outSize
				 outIsWritable: (BOOL *) outWritable];
}

static OSStatus
Interface_StreamGetProperty(AudioHardwarePlugInRef inSelf,
			    AudioStreamID inStreamID,
			    UInt32 inChannel,
			    AudioDevicePropertyID inPropertyID,
			    UInt32 *ioPropertyDataSize,
			    void *outPropertyData)
{
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin streamGetProperty: inStreamID
				 channel: inChannel
			      propertyID: inPropertyID
			      ioDataSize: ioPropertyDataSize
				 outData: outPropertyData];
	
	
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
	PAPlugin *plugin = to_plugin(inSelf);
	return [plugin streamSetProperty: inStreamID
				    when: inWhen
				 channel: inChannel
			      propertyID: inPropertyID
				dataSize: inPropertyDataSize
				    data: inPropertyData];
}

- (id) initWithPool: (NSAutoreleasePool *) _pool
{
	[super init];
	
	pool = _pool;
	
	staticInterface = (AudioHardwarePlugInInterface *) malloc(sizeof(AudioHardwarePlugInInterface));
	bzero(staticInterface, sizeof(*staticInterface));
	
	//	IUnknown Routines
	staticInterface->QueryInterface	= (HRESULT (*)(void*, CFUUIDBytes, void**)) Interface_QueryInterface;
	staticInterface->AddRef		= (ULONG (*)(void*)) Interface_AddRef;
	staticInterface->Release	= (ULONG (*)(void*)) Interface_Release;

	//	HAL Plug-In Routines
	staticInterface->Initialize			= Interface_Initialize;
	staticInterface->InitializeWithObjectID		= Interface_InitializeWithObjectID;
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
	staticInterface->DeviceCreateIOProcID		= Interface_DeviceCreateIOProcID;
	staticInterface->DeviceDestroyIOProcID		= Interface_DeviceDestroyIOProcID;
	staticInterface->DeviceStartAtTime		= Interface_DeviceStartAtTime;
	staticInterface->DeviceGetNearestStartTime	= Interface_DeviceGetNearestStartTime;
	staticInterface->StreamGetPropertyInfo		= Interface_StreamGetPropertyInfo;
	staticInterface->StreamGetProperty		= Interface_StreamGetProperty;
	staticInterface->StreamSetProperty		= Interface_StreamSetProperty;
	staticInterface->ObjectShow			= Interface_ObjectShow;
	staticInterface->ObjectHasProperty		= Interface_ObjectHasProperty;
	staticInterface->ObjectIsPropertySettable	= Interface_ObjectIsPropertySettable;
	staticInterface->ObjectGetPropertyDataSize	= Interface_ObjectGetPropertyDataSize;
	staticInterface->ObjectGetPropertyData		= Interface_ObjectGetPropertyData;
	staticInterface->ObjectSetPropertyData		= Interface_ObjectSetPropertyData;

	plugin = [[PAPlugin alloc] initWithPluginRef: &staticInterface];
	
	return self;
}

- (void) dealloc
{
	free(staticInterface);
	[super dealloc];
}

- (AudioHardwarePlugInRef) getInterface
{
	DebugLog("self %p, &staticInterface %p", self, staticInterface);
	return &staticInterface;
}

@end

void *
New_PAHAL_PlugIn(CFAllocatorRef *allocator, CFUUIDRef requestedTypeUUID) 
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (CFEqual(requestedTypeUUID, kAudioHardwarePlugInTypeID)) {
		PAPlugInInterface *interface = [[PAPlugInInterface alloc] initWithPool: pool];
		return [interface getInterface];
	}
	
	return NULL;
}
