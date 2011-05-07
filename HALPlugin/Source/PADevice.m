/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <mach/mach_time.h>

#import "PADevice.h"
#import "PADeviceAudio.h"
#import "PAStream.h"
#import "PAPlugin.h"

@implementation PADevice

@synthesize name;
@synthesize delegate;
@synthesize deviceAudio;
@synthesize serverName;
@synthesize sinkForPlayback;
@synthesize sourceForRecord;

#pragma mark ### audio object handling ###

- (void) publishOwnedObjects
{
        [super publishOwnedObjects];

        for (PAStream *stream in inputStreamArray)
                [stream publishOwnedObjects];

        for (PAStream *stream in outputStreamArray)
                [stream publishOwnedObjects];
}

- (void) depublishOwnedObjects
{
        for (PAStream *stream in inputStreamArray)
                [stream depublishOwnedObjects];

        for (PAStream *stream in outputStreamArray)
                [stream depublishOwnedObjects];

        [super depublishOwnedObjects];
}

- (PAStream *) findStreamByID: (AudioObjectID) searchID
{
        for (PAStream *stream in inputStreamArray)
                if (stream.objectID == searchID)
                        return stream;

        for (PAStream *stream in outputStreamArray)
                if (stream.objectID == searchID)
                        return stream;

        return nil;
}

- (void) addOwnedObjectsToArray: (NSMutableArray *) array
{
        [array addObjectsFromArray: inputStreamArray];
        [array addObjectsFromArray: outputStreamArray];
}

#pragma mark ### connect/disconnect ###

- (void) connectToServer
{
        if (![serverConnection isConnected])
                [serverConnection connectToHost: serverName
                                           port: -1];
}

- (void) disconnectFromServer
{
        [serverConnection disconnect];
}

- (void) checkConnection
{
        // AudioDeviceStop() is likely to be called from the IOProc thread, so
        // we have to dispatch these calls in from the main thread context.

        if ([deviceAudio hasActiveProcs])
                [self performSelectorOnMainThread: @selector(connectToServer)
                                       withObject: nil
                                    waitUntilDone: YES];
        else
                [self performSelectorOnMainThread: @selector(disconnectFromServer)
                                       withObject: nil
                                    waitUntilDone: YES];
}

- (void) reconnectIfConnected
{
        if (!serverConnection || ![serverConnection isConnected])
                return;

        [self disconnectFromServer];
        [self checkConnection];
}

- (void) setConfig: (NSDictionary *) config
{
        if (serverName) {
                [serverName retain];
                serverName = nil;
        }

        if (sinkForPlayback) {
                [sinkForPlayback release];
                sinkForPlayback = nil;
        }

        if (sourceForRecord) {
                [sourceForRecord release];
                sourceForRecord = nil;
        }

        NSLog(@"%s()", __func__);
        
        serverName = [[config objectForKey: @"serverName"] retain];
        sinkForPlayback = [[config objectForKey: @"sinkForPlayback"] retain];
        sourceForRecord = [[config objectForKey: @"sourceForRecord"] retain];

        [self reconnectIfConnected];
}

- (NSString *) serverName
{
        if (!serverConnection)
                return nil;

        if ([serverConnection isLocal])
                return @"localhost";

        return [serverConnection serverName];
}

- (NSString *) sinkForPlayback
{
        if (!serverConnection)
                return nil;

        return [serverConnection connectedSink];
}

- (NSString *) sourceForRecord
{
        if (!serverConnection)
                return nil;

        return [serverConnection connectedSource];
}

#pragma mark ### PADevice ###

- (id) initWithPluginRef: (AudioHardwarePlugInRef) ref
              deviceName: (NSString *) _name
          nInputChannels: (UInt32) nInputChannels
         nOutputChannels: (UInt32) nOutputChannels
{
        [super initWithPluginRef: ref];

        OSStatus ret = AudioObjectCreate(pluginRef,
                                         kAudioObjectSystemObject,
                                         kAudioDeviceClassID,
                                         &objectID);

        if (ret != kAudioHardwareNoError)
                DebugLog("AudioObjectCreate() failed");

        isRunning = NO;

        memset(&streamDescription, 0, sizeof(streamDescription));
        memset(&physicalFormat, 0, sizeof(physicalFormat));

        deviceAudio = [[[PADeviceAudio alloc] initWithPADevice: self] retain];
        serverConnection = [[[PAServerConnection alloc] init] retain];
        serverConnection.delegate = self;

        streamDescription.mSampleRate = deviceAudio.sampleRate;
        streamDescription.mFormatID = kAudioFormatLinearPCM;
        streamDescription.mFormatFlags = kAudioFormatFlagsCanonical;
        streamDescription.mBitsPerChannel = 32;
        streamDescription.mChannelsPerFrame = 2;
        streamDescription.mFramesPerPacket = 1;
        streamDescription.mBytesPerFrame = deviceAudio.bytesPerSampleFrame;
        streamDescription.mBytesPerPacket = streamDescription.mFramesPerPacket *
                                            streamDescription.mBytesPerFrame;

        physicalFormat.mSampleRateRange.mMinimum = deviceAudio.sampleRate;
        physicalFormat.mSampleRateRange.mMaximum = deviceAudio.sampleRate;
        memcpy(&physicalFormat.mFormat, &streamDescription, sizeof(streamDescription));

        inputStreamArray = [[NSMutableArray arrayWithCapacity: 0] retain];
        outputStreamArray = [[NSMutableArray arrayWithCapacity: 0] retain];

        PAStream *stream;

        for (UInt32 i = 0; i < nInputChannels / 2; i++) {
                stream = [[PAStream alloc] initWithDevice: self
                                                  isInput: YES
                                          startingChannel: 1];
                stream.owningObjectID = self.objectID;
                [inputStreamArray addObject: stream];
        }

        for (UInt32 i = 0; i < nOutputChannels / 2; i++) {
                stream = [[PAStream alloc] initWithDevice: self
                                                  isInput: NO
                                          startingChannel: 1];
                stream.owningObjectID = self.objectID;
                [outputStreamArray addObject: stream];
        }

        name = [_name retain];
        manufacturer = @"pulseaudio.org";
        modelUID = [NSString stringWithFormat: @"%@:%d,%d",
                                        name,
                                        [inputStreamArray count],
                                        [outputStreamArray count]];
        deviceUID = [NSString stringWithFormat: @"org.pulseaudio.HALPlugin.%@", modelUID];

        return self;
}

- (void) dealloc
{
        [name release];
        [serverConnection disconnect];
        [serverConnection release];
        [deviceAudio release];
        [inputStreamArray release];
        [outputStreamArray release];
        [super dealloc];
}

#pragma mark ### Plugin interface ###

- (OSStatus) createIOProcID: (AudioDeviceIOProc) proc
                 clientData: (void *) clientData
                outIOProcID: (AudioDeviceIOProcID *) outProcID
{
        DebugLog();

        AudioDeviceIOProcID pid = [deviceAudio createIOProcID: proc
                                                   clientData: clientData];
        if (!pid)
                return kAudioHardwareIllegalOperationError;

        if (outProcID)
                *outProcID = pid;

        return kAudioHardwareNoError;
}

- (OSStatus) destroyIOProcID: (AudioDeviceIOProcID) procID
{
        DebugLog();

        [deviceAudio removeIOProcID: procID];
        [self checkConnection];
        return kAudioHardwareNoError;
}

- (OSStatus) addIOProc: (AudioDeviceIOProc) proc
            clientData: (void *) clientData
{
        DebugLog();

        [deviceAudio createIOProcID: proc
                         clientData: clientData];
        return kAudioHardwareNoError;
}

- (OSStatus) removeIOProc: (AudioDeviceIOProc) proc
{
        DebugLog();

        [deviceAudio removeIOProcID: proc];
        [self checkConnection];
        return kAudioHardwareNoError;
}

- (OSStatus) start: (AudioDeviceIOProc) procID
{
        DebugLog();

        [deviceAudio setIOProcIDEnabled: procID
                                enabled: YES];

        [self checkConnection];
        return kAudioHardwareNoError;
}

- (OSStatus) startAtTime: (AudioDeviceIOProc) procID
    ioRequestedStartTime: (AudioTimeStamp *) ioRequestedStartTime
                   flags: (UInt32) flags
{
        DebugLog();

        [deviceAudio setStartTimeForProcID: procID
                                 timeStamp: ioRequestedStartTime
                                     flags: flags];
        [self checkConnection];
        return kAudioHardwareNoError;
}

- (OSStatus) stop: (AudioDeviceIOProc) procID
{
        DebugLog();

        [deviceAudio setIOProcIDEnabled: procID
                                enabled: NO];
        [self checkConnection];
        return kAudioHardwareNoError;
}

- (OSStatus) read: (const AudioTimeStamp *) startTime
          outData: (AudioBufferList *) outData
{
        return kAudioHardwareUnsupportedOperationError;
}

#pragma mark ### time related ###

- (OSStatus) getCurrentTime: (AudioTimeStamp *) outTime
{
        memset(outTime, 0, sizeof(*outTime));

        outTime->mRateScalar = 1.0;
        outTime->mHostTime = mach_absolute_time();
        outTime->mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;

        return kAudioHardwareNoError;
}

- (OSStatus) translateTime: (const AudioTimeStamp *) inTime
                   outTime: (AudioTimeStamp *) outTime
{
        memcpy(outTime, inTime, sizeof(*outTime));
        return kAudioHardwareNoError;
}

- (OSStatus) getNearestStartTime: (AudioTimeStamp *) ioRequestedStartTime
                           flags: (UInt32) flags
{
        memset(ioRequestedStartTime, 0, sizeof(*ioRequestedStartTime));
        return kAudioHardwareNoError;
}

#pragma mark ### properties ###

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address
{
        switch (address->mSelector) {
                //case kAudioDevicePropertyIcon:
                case kAudioDevicePropertyActualSampleRate:
                case kAudioDevicePropertyBufferFrameSize:
                case kAudioDevicePropertyBufferFrameSizeRange:
                case kAudioDevicePropertyBufferSize:
                case kAudioDevicePropertyBufferSizeRange:
                case kAudioDevicePropertyClockDomain:
                //case kAudioDevicePropertyConfigurationApplication:
                case kAudioDevicePropertyDeviceCanBeDefaultDevice:
                case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
                case kAudioDevicePropertyDeviceHasChanged:
                case kAudioDevicePropertyDeviceIsAlive:
                case kAudioDevicePropertyDeviceIsRunning:
                case kAudioDevicePropertyDeviceIsRunningSomewhere:
                case kAudioDevicePropertyDeviceManufacturer:
                case kAudioDevicePropertyDeviceName:
                case kAudioDevicePropertyDeviceUID:
                case kAudioDevicePropertyIOProcStreamUsage:
                case kAudioDevicePropertyIsHidden:
                case kAudioDevicePropertyLatency:
                case kAudioDevicePropertyModelUID:
                case kAudioDevicePropertyNominalSampleRate:
                //case kAudioDevicePropertyRelatedDevices:
                case kAudioDevicePropertySafetyOffset:
                case kAudioDevicePropertyStreamConfiguration:
                case kAudioDevicePropertyStreams:
                case kAudioDevicePropertyTransportType:
                case kAudioObjectPropertyManufacturer:
                case kAudioObjectPropertyName:
                case kAudioDevicePropertyAvailableNominalSampleRates:
                case kAudioDevicePropertyUsesVariableBufferFrameSizes:

                // stream properties
                case kAudioStreamPropertyAvailableVirtualFormats:
                case kAudioStreamPropertyAvailablePhysicalFormats:
                case kAudioDevicePropertyStreamFormats:
                case kAudioDevicePropertyStreamFormat:
                case kAudioStreamPropertyPhysicalFormats:
                case kAudioStreamPropertyPhysicalFormat:
                        return YES;
        }

        return [super hasProperty: address];
}

- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address
{
        switch (address->mSelector) {
                case kAudioDevicePropertyBufferFrameSize:
                case kAudioDevicePropertyBufferSize:
                case kAudioDevicePropertyNominalSampleRate:

                // stream properties
                case kAudioStreamPropertyPhysicalFormat:
                case kAudioDevicePropertyStreamFormat:
                        return YES;
        }

        return [super isPropertySettable: address];
}

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
               qualifierDataSize: (UInt32) qualifierDataSize
                   qualifierData: (const void *) qualifierData
                         outSize: (UInt32 *) outSize
{
        BOOL isInput = address->mScope == kAudioDevicePropertyScopeInput;

        switch (address->mSelector) {
                case kAudioObjectPropertyName:
                case kAudioObjectPropertyManufacturer:
                case kAudioDevicePropertyConfigurationApplication:
                case kAudioDevicePropertyDeviceUID:
                case kAudioDevicePropertyModelUID:
                        *outSize = sizeof(NSString *);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyRelatedDevices:
                        *outSize = 0; //sizeof(AudioObjectID);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyIsHidden:
                case kAudioDevicePropertyTransportType:
                case kAudioDevicePropertyClockDomain:
                case kAudioDevicePropertyDeviceIsAlive:
                case kAudioDevicePropertyDeviceHasChanged:
                case kAudioDevicePropertyDeviceIsRunning:
                case kAudioDevicePropertyDeviceIsRunningSomewhere:
                case kAudioDevicePropertyDeviceCanBeDefaultDevice:
                case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
                case kAudioDevicePropertyLatency:
                case kAudioDevicePropertyBufferFrameSize:
                case kAudioDevicePropertySafetyOffset:
                case kAudioDevicePropertyBufferSize:
                case kAudioDevicePropertyUsesVariableBufferFrameSizes:
                        *outSize = sizeof(UInt32);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyBufferFrameSizeRange:
                case kAudioDevicePropertyBufferSizeRange:
                        *outSize = sizeof(AudioValueRange);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyStreams:
                        *outSize = sizeof(AudioStreamID) * (isInput ? [inputStreamArray count] : [outputStreamArray count]);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyStreamConfiguration:
                        *outSize = sizeof(AudioBufferList);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyIOProcStreamUsage:
                        *outSize = 0; //sizeof(AudioHardwareIOProcStreamUsage) - sizeof(UInt32[kVariableLengthArray]);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyNominalSampleRate:
                case kAudioDevicePropertyActualSampleRate:
                        *outSize = sizeof(Float64);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceName:
                        *outSize = [name length] + 1;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceManufacturer:
                        *outSize = [manufacturer length] + 1;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyAvailableNominalSampleRates:
                        *outSize = sizeof(AudioValueRange);
                        return kAudioHardwareNoError;

                //case kAudioDevicePropertyIcon:
                //        return sizeof(CFURLRef);

                // Stream properties
                case kAudioStreamPropertyAvailableVirtualFormats:
                case kAudioStreamPropertyAvailablePhysicalFormats:
                        *outSize = sizeof(AudioStreamRangedDescription);
                        return kAudioHardwareNoError;

                case kAudioStreamPropertyPhysicalFormats:
                case kAudioStreamPropertyPhysicalFormat:
                case kAudioDevicePropertyStreamFormats:
                case kAudioDevicePropertyStreamFormat:
                        *outSize = sizeof(AudioStreamBasicDescription);
                        return kAudioHardwareNoError;
        }

        return [super getPropertyDataSize: address
                        qualifierDataSize: qualifierDataSize
                            qualifierData: qualifierData
                                  outSize: outSize];
}

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                  ioDataSize: (UInt32 *) ioDataSize
                     outData: (void *) outData
{
        BOOL isInput = address->mScope == kAudioDevicePropertyScopeInput;
        char tmp[100];

        switch (address->mSelector) {
                case kAudioObjectPropertyName:
                        *(NSString **) outData = [name copy];
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyModelUID:
                        *(NSString **) outData = [modelUID copy];
                        *ioDataSize = sizeof(CFStringRef);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceUID:
                        *(NSString **) outData = [deviceUID copy];
                        *ioDataSize = sizeof(NSString *);
                        return kAudioHardwareNoError;

                case kAudioObjectPropertyManufacturer:
                        *(NSString **) outData = [manufacturer copy];
                        *ioDataSize = sizeof(CFStringRef);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceName:
                        *ioDataSize = MIN(*ioDataSize, sizeof(tmp));
                        memset(outData, 0, *ioDataSize);
                        [name getCString: tmp
                               maxLength: sizeof(tmp)
                                encoding: NSASCIIStringEncoding];
                        memcpy(outData, tmp, *ioDataSize);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceManufacturer:
                        *ioDataSize = MIN(*ioDataSize, sizeof(tmp));
                        memset(outData, 0, *ioDataSize);
                        [manufacturer getCString: tmp
                                       maxLength: sizeof(tmp)
                                        encoding: NSASCIIStringEncoding];
                        memcpy(outData, tmp, *ioDataSize);
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyTransportType:
                        *(UInt32 *) outData = 'virt';
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
                case kAudioDevicePropertyDeviceCanBeDefaultDevice:
                case kAudioDevicePropertyDeviceIsAlive:
                        /* "always true" */
                        *(UInt32 *) outData = 1;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyIsHidden:
                case kAudioDevicePropertyUsesVariableBufferFrameSizes:
                        /* "always false" */
                        *(UInt32 *) outData = 0;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyDeviceIsRunning:
                case kAudioDevicePropertyDeviceIsRunningSomewhere:
                        *(UInt32 *) outData = isRunning;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyBufferFrameSize:
                        *(UInt32 *) outData = deviceAudio.ioProcBufferSize;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyBufferSize:
                        *(UInt32 *) outData = deviceAudio.ioProcBufferSize * deviceAudio.bytesPerSampleFrame;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyIOProcStreamUsage: {
                        AudioHardwareIOProcStreamUsage *usage = outData;
                        NSArray *streamArray = isInput ? inputStreamArray : outputStreamArray;
                        usage->mNumberStreams = [streamArray count];

                        for (UInt32 i = 0; i < usage->mNumberStreams; i++)
                                usage->mStreamIsOn[i] = [deviceAudio channelIsActive: i];

                        return kAudioHardwareNoError;
                }

                case kAudioDevicePropertyBufferFrameSizeRange: {
                        AudioValueRange *range = outData;
                        // FIXME
                        range->mMinimum = 64;
                        range->mMaximum = 8192;
                        return kAudioHardwareNoError;
                }

                case kAudioDevicePropertyAvailableNominalSampleRates: {
                        AudioValueRange *range = outData;
                        range->mMinimum = deviceAudio.sampleRate;
                        range->mMaximum = deviceAudio.sampleRate;
                        return kAudioHardwareNoError;
                }

                case kAudioDevicePropertyActualSampleRate:
                case kAudioDevicePropertyNominalSampleRate:
                        *(Float64 *) outData = deviceAudio.sampleRate;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyClockDomain:
                        *(UInt32 *) outData = 0;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertySafetyOffset:
                case kAudioDevicePropertyLatency:
                        *(UInt32 *) outData = 0;
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyStreams: {
                        AudioStreamID *list = outData;
                        NSArray *streamArray = (isInput ? inputStreamArray : outputStreamArray);
                        UInt32 n = MIN([streamArray count], *ioDataSize / sizeof(UInt32));
                        *ioDataSize = n * sizeof(UInt32);

                        for (PAStream *stream in streamArray)
                                *list++ = stream.objectID;

                        return kAudioHardwareNoError;
                }

                case kAudioDevicePropertyStreamConfiguration: {
                        AudioBufferList *list = outData;
                        memset(list, 0, sizeof(*list));
                        list->mNumberBuffers = 1;
                        list->mBuffers[0].mNumberChannels = (isInput ? [inputStreamArray count] : [outputStreamArray count]) * 2;
                        list->mBuffers[0].mDataByteSize = deviceAudio.ioProcBufferSize * deviceAudio.bytesPerSampleFrame;
                        *ioDataSize = sizeof(*list);
                        return kAudioHardwareNoError;
                }

                case kAudioStreamPropertyAvailablePhysicalFormats:
                case kAudioStreamPropertyAvailableVirtualFormats:
                        *ioDataSize = MIN(*ioDataSize, sizeof(physicalFormat));
                        memcpy(outData, &physicalFormat, *ioDataSize);
                        return kAudioHardwareNoError;

                case kAudioStreamPropertyPhysicalFormats:
                case kAudioStreamPropertyPhysicalFormat:
                case kAudioDevicePropertyStreamFormats:
                case kAudioDevicePropertyStreamFormat:
                        *ioDataSize = MIN(*ioDataSize, sizeof(streamDescription));
                        memcpy(outData, &streamDescription, *ioDataSize);
                        return kAudioHardwareNoError;
        }

        return [super getPropertyData: address
                    qualifierDataSize: qualifierDataSize
                        qualifierData: qualifierData
                           ioDataSize: ioDataSize
                              outData: outData];
}

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                    dataSize: (UInt32) dataSize
                        data: (const void *) data
{
        switch (address->mSelector) {
                case kAudioDevicePropertyDeviceIsRunning:
                        [deviceAudio setAllIOProcs: (BOOL) *(UInt32 *) data];
                        [self checkConnection];
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyBufferFrameSize:
                        deviceAudio.ioProcBufferSize = *(UInt32 *) data;
                        [self reconnectIfConnected];
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyBufferSize:
                        deviceAudio.ioProcBufferSize = (*(UInt32 *) data) / deviceAudio.bytesPerSampleFrame;
                        [self reconnectIfConnected];
                        return kAudioHardwareNoError;

                case kAudioDevicePropertyNominalSampleRate:
                        deviceAudio.sampleRate = *(const Float64*) data;
                        DebugLog("SETTING sample rate %f", deviceAudio.sampleRate);
                        [self reconnectIfConnected];
                        return kAudioHardwareNoError;

                //case kAudioStreamPropertyPhysicalFormat:
                //        return kAudioHardwareNoError;

                //case kAudioDevicePropertyStreamFormat:
                //        inDataSize = MIN(inDataSize, sizeof(streamDescription));
                //        memcpy(&streamDescription, inData, inDataSize);
                //        return kAudioHardwareNoError;
        }

        return [super setPropertyData: address
                    qualifierDataSize: qualifierDataSize
                        qualifierData: qualifierData
                             dataSize: dataSize
                                 data: data];
}

#pragma mark ### properties (legacy interface) ###

- (OSStatus) getPropertyInfo: (UInt32) channel
                     isInput: (BOOL) isInput
                    propertyID: (AudioDevicePropertyID) propertyID
                     outSize: (UInt32 *) outSize
                 outWritable: (BOOL *) outWritable
{
        AudioObjectPropertyAddress addr;
        OSStatus ret = kAudioHardwareNoError;

        addr.mSelector = propertyID;
        addr.mElement = channel;
        addr.mScope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;

        if (![self hasProperty: &addr])
                return kAudioHardwareUnknownPropertyError;

        if (outWritable)
                *outWritable = [self isPropertySettable: &addr];

        if (outSize)
                ret = [self getPropertyDataSize: &addr
                              qualifierDataSize: 0
                                  qualifierData: NULL
                                        outSize: outSize];

        return ret;
}

- (OSStatus) getProperty: (UInt32) channel
                 isInput: (BOOL) isInput
                propertyID: (AudioDevicePropertyID) propertyID
              ioDataSize: (UInt32 *) ioDataSize
                 outData: (void *) data
{
        AudioObjectPropertyAddress addr;

        addr.mSelector = propertyID;
        addr.mElement = channel;
        addr.mScope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;

        return [self getPropertyData: &addr
                   qualifierDataSize: 0
                       qualifierData: NULL
                          ioDataSize: ioDataSize
                             outData: data];
}

- (OSStatus) setProperty: (const AudioTimeStamp *) when
                 channel: (UInt32) channel
                 isInput: (BOOL) isInput
                propertyID: (AudioDevicePropertyID) propertyID
                dataSize: (UInt32) dataSize
                    data: (const void *) data;
{
        AudioObjectPropertyAddress addr;

        addr.mSelector = propertyID;
        addr.mElement = channel;
        addr.mScope = isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;

        return [self setPropertyData: &addr
                   qualifierDataSize: 0
                       qualifierData: NULL
                            dataSize: dataSize
                                data: data];
}

#pragma mark ### PAServerConnectionDelegate ###

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection
{
        NSLog(@"%s()", __func__);
        BOOL ret = [serverConnection addAudioStreams: [deviceAudio countActiveChannels]
                                          sampleRate: deviceAudio.sampleRate
                                    ioProcBufferSize: deviceAudio.ioProcBufferSize
                                     sinkForPlayback: sinkForPlayback
                                     sourceForRecord: sourceForRecord];
        if (!ret)
                NSLog(@"%s(): addAudioStreams failed!?", __func__);
}

- (void) PAServerConnectionFailed: (PAServerConnection *) connection
{
        NSLog(@"%s()", __func__);
}

- (void) PAServerConnectionEnded: (PAServerConnection *) connection
{
        NSLog(@"%s()", __func__);
        if (delegate)
                [delegate deviceStopped: self];
}

- (void) PAServerConnectionAudioStarted: (PAServerConnection *) connection
{
        NSLog(@"%s()", __func__);

        if (sinkForPlayback)
                [sinkForPlayback release];

        if (sourceForRecord)
                [sourceForRecord release];

        sinkForPlayback = [[serverConnection connectedSink] retain];
        sourceForRecord = [[serverConnection connectedSource] retain];

        if (delegate)
                [delegate deviceStarted: self];
}

- (UInt32) PAServerConnection: (PAServerConnection *) connection
              hasPlaybackData: (Byte *) playbackData
                   recordData: (const Byte *) recordData
                     byteSize: (UInt32) byteSize
{
        return [deviceAudio PAServerConnection: connection
                               hasPlaybackData: playbackData
                                    recordData: recordData
                                      byteSize: byteSize];
}

@end
