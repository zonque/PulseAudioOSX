/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <PulseAudio/PulseAudio.h>

#import "PAObject.h"

#define CHANNELS_PER_STREAM 2

@class PAStream;
@class PAPlugin;
@class PADevice;
@class PADeviceAudio;

@protocol PADeviceDelegate
@required
- (void) deviceStarted: (PADevice *) device;
- (void) deviceStopped: (PADevice *) device;
@end

@interface PADevice : PAObject <PAServerConnectionDelegate>
{
    NSMutableArray *inputStreamArray;
    NSMutableArray *outputStreamArray;
    
    NSObject <PADeviceDelegate> *delegate;
    
    NSString *name;
    NSString *manufacturer;
    NSString *modelUID;
    NSString *deviceUID;
    
    BOOL isRunning;
    
    AudioStreamBasicDescription streamDescription;
    AudioStreamRangedDescription physicalFormat;
    
    PAServerConnection *serverConnection;
    PADeviceAudio *deviceAudio;
    
    NSString *serverName;
    NSString *sinkForPlayback;
    NSString *sourceForRecord;
}

@property (nonatomic, readonly) PADeviceAudio *deviceAudio;
@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) NSString *serverName;
@property (nonatomic, readonly) NSString *sinkForPlayback;
@property (nonatomic, readonly) NSString *sourceForRecord;
@property (nonatomic, readonly) NSArray *inputStreamArray;
@property (nonatomic, readonly) NSArray *outputStreamArray;
@property (nonatomic, assign) NSObject *delegate;

- (NSString *) serverName;
- (NSString *) sinkForPlayback;
- (NSString *) sourceForRecord;

- (id) initWithPluginRef: (AudioHardwarePlugInRef) ref
              deviceName: (NSString *) _name
          nInputChannels: (UInt32) nInputChannels
         nOutputChannels: (UInt32) nOutputChannels;

- (PAStream *) findStreamByID: (AudioObjectID) searchID;
- (void) setConfig: (NSDictionary *) config;
- (void) setPreferences: (NSDictionary *) preferences;

#pragma mark ### PAObject ###

- (void) publishOwnedObjects;
- (void) depublishOwnedObjects;

#pragma mark ### Plugin interface ###

- (OSStatus) createIOProcID: (AudioDeviceIOProc) proc
                 clientData: (void *) clientData
                outIOProcID: (AudioDeviceIOProcID *) outIOProcID;

- (OSStatus) destroyIOProcID: (AudioDeviceIOProcID) inIOProcID;

- (OSStatus) addIOProc: (AudioDeviceIOProc) proc
            clientData: (void *) clientData;

- (OSStatus) removeIOProc: (AudioDeviceIOProc) proc;

- (OSStatus) start: (AudioDeviceIOProc) inProcID;

- (OSStatus) startAtTime: (AudioDeviceIOProc) procID
    ioRequestedStartTime: (AudioTimeStamp *) ioRequestedStartTime
                   flags: (UInt32) flags;

- (OSStatus) stop: (AudioDeviceIOProc) procID;

- (OSStatus) read: (const AudioTimeStamp *) startTime
          outData: (AudioBufferList *) outData;

- (OSStatus) getCurrentTime: (AudioTimeStamp *) outTime;

- (OSStatus) translateTime: (const AudioTimeStamp *) inTime
                   outTime: (AudioTimeStamp *) outTime;

- (OSStatus) getNearestStartTime: (AudioTimeStamp *) ioRequestedStartTime
                           flags: (UInt32) flags;

#pragma mark ### properties ###

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address;
- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address;

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
               qualifierDataSize: (UInt32) qualifierDataSize
                   qualifierData: (const void *) qualifierData
                         outSize: (UInt32 *) outSize;

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                  ioDataSize: (UInt32 *) ioDataSize
                     outData: (void *) outData;

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                    dataSize: (UInt32) dataSize
                        data: (const void *) data;

#pragma mark ### properties (legacy interface) ###

- (OSStatus) getPropertyInfo: (UInt32) channel
                     isInput: (BOOL) isInput
                  propertyID: (AudioDevicePropertyID) propertyID
                     outSize: (UInt32 *) outSize
                 outWritable: (BOOL *) outWritable;

- (OSStatus) getProperty: (UInt32) channel
                 isInput: (BOOL) isInput
              propertyID: (AudioDevicePropertyID) property
              ioDataSize: (UInt32 *) ioPropertyDataSize
                 outData: (void *) data;

- (OSStatus) setProperty: (const AudioTimeStamp *) when
                 channel: (UInt32) channel
                 isInput: (BOOL) isInput
              propertyID: (AudioDevicePropertyID) propertyID
                dataSize: (UInt32) dataSize
                    data: (const void *) data;

#pragma mark ### PAServerConnectionDelegate ###

- (void) PAServerConnectionEstablished: (PAServerConnection *) connection;
- (void) PAServerConnectionFailed: (PAServerConnection *) connection;
- (void) PAServerConnectionEnded: (PAServerConnection *) connection;
- (UInt32) PAServerConnection: (PAServerConnection *) connection
              hasPlaybackData: (Byte *) playbackData
                   recordData: (const Byte *) recordData
                     byteSize: (UInt32) byteSize;

@end
