/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import "PAPlugin.h"
#import "PADevice.h"
#import "PAStream.h"
#import "PADeviceAudio.h"

#ifdef ENABLE_DEBUGX
#define DebugProperty(x...) DebugLog(x)
#else
#define DebugProperty(x...) do {} while(0)
#endif

@implementation PAPlugin

- (void) publishOwnedObjects
{
    [super publishOwnedObjects];
    
    for (PADevice *dev in devicesArray)
        [dev publishOwnedObjects];
}

- (void) depublishOwnedObjects
{
    for (PADevice *dev in devicesArray)
        [dev depublishOwnedObjects];
    
    [super depublishOwnedObjects];
}

- (void) createDevices
{
	PADevice *dev;

    // FIXME - this information should actually be obtained from
    // PulseAudioHelper in order to support a dynamic number of
    // virtual sound card instances.
    dev = [[PADevice alloc] initWithPluginRef: pluginRef
                                   deviceName: @"PulseAudio"
                               nInputChannels: 0
                              nOutputChannels: 2];
	dev.owningObjectID = self.objectID;
	dev.delegate = self;
	[devicesArray addObject: dev];
    
    [self publishOwnedObjects];
}

- (void) destroyDevices
{
    [self depublishOwnedObjects];
    [devicesArray removeAllObjects];
}

- (id) initWithPluginRef: (AudioHardwarePlugInRef) _pluginRef
{
    [super initWithPluginRef: _pluginRef];
    
    DebugLog(@"_pluginRef %p", _pluginRef);
    devicesArray = [[NSMutableArray arrayWithCapacity: 0] retain];
    helperConnection = [[[PAHelperConnection alloc] init] retain];
    helperConnection.delegate = self;
    
    return self;
}

- (void) dealloc
{
    [self destroyDevices];
    [helperConnection release];
    [devicesArray release];
    
    [super dealloc];
}

- (PADevice *) findDeviceByID: (AudioObjectID) searchID
{
    for (PADevice *dev in devicesArray)
        if (dev.objectID == searchID)
            return dev;
    
    return nil;
}

- (PAStream *) findStreamByID: (AudioObjectID) searchID
{
    for (PADevice *dev in devicesArray) {
        PAStream *s = [dev findStreamByID: searchID];
        if (s)
            return s;
    }
    
    return nil;
}

- (void) addOwnedObjectsToArray: (NSMutableArray *) array
{
    [array addObjectsFromArray: devicesArray];
}

#pragma mark PAHelperConnectionDelegate ###

- (void) PAHelperConnectionEstablished: (PAHelperConnection *) connection
{
	if ([devicesArray count] == 0)
		[self createDevices];
}

- (void) PAHelperConnectionDied: (PAHelperConnection *) connection
{
    [self destroyDevices];
}

- (void) PAHelperConnection: (PAHelperConnection *) connection
            receivedMessage: (NSString *) name
                       dict: (NSDictionary *) dict
{
	if ([name isEqualToString: PAOSX_MessageSetPreferences])
		for (PADevice *d in devicesArray)
			[d setPreferences: dict];
    
	if ([name isEqualToString: PAOSX_MessageSetAudioClientConfig]) {
		NSString *deviceName = [dict objectForKey: @"deviceName"];
        
		for (PADevice *d in devicesArray)
			if ([d.name isEqualToString: deviceName])
				[d setConfig: dict];
	}
}

#pragma mark ### PADeviceDelegate ###

- (void) deviceStarted: (PADevice *) device
{
    NSLog(@"%s()", __func__);
    if (![helperConnection isConnected])
        return;
        
    PADeviceAudio *audio = device.deviceAudio;
    NSLog(@"%s() audio %p", __func__, audio);
    if (!audio)
        return;
    
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity: 0];
    
    [dict setObject: [NSNumber numberWithInt: audio.ioProcBufferSize]
             forKey: @"ioProcBufferSize"];
    [dict setObject: [NSNumber numberWithFloat: audio.sampleRate]
             forKey: @"sampleRate"];
    [dict setObject: device.name
             forKey: @"deviceName"];
    [dict setObject: [NSNumber numberWithInt: getpid()]
             forKey: @"pid"];
    
    if (device.serverName)
        [dict setObject: device.serverName
                 forKey: @"serverName"];
    
    if (device.sinkForPlayback)
        [dict setObject: device.sinkForPlayback
                 forKey: @"sinkForPlayback"];
    
    if (device.sourceForRecord)
        [dict setObject: device.sourceForRecord
                 forKey: @"sourceForRecord"];
    
	[helperConnection sendMessage: PAOSX_MessageAudioClientStarted
                             dict: dict];	
}

- (void) deviceStopped: (PADevice *) device
{
	NSDictionary *dict = [NSDictionary dictionaryWithObject: device.name
                                                     forKey: @"deviceName"];
	[helperConnection sendMessage: PAOSX_MessageAudioClientStopped
                             dict: dict];	
}

#pragma mark ### PlugIn Operations ###

- (OSStatus) initialize
{
    return [self initializeWithObjectID: kAudioObjectUnknown];
}

- (OSStatus) initializeWithObjectID: (AudioObjectID) oid
{
    self.objectID = oid;
    
	if ([helperConnection connectWithRetry: YES])
		[self createDevices];
	
    return kAudioHardwareNoError;
}

#pragma mark ### AudioObject Operations ###

- (void) objectShow: (AudioObjectID) oid
{
    PAObject *o = [self findObjectByID: oid];
    if (o) {
        [o lock];
        [o show];
        [o unlock];
    }
}

- (BOOL) objectHasProperty: (AudioObjectID) oid
                propertyID: (const AudioObjectPropertyAddress *) address
{
    BOOL ret = NO;
    PAObject *o = [self findObjectByID: oid];
    
    if (o) {
        [o lock];
        ret = [o hasProperty: address];
        [o unlock];
        
        if (!ret) {
            DebugProperty("id %d (%@) has NO property '%c%c%c%c'",
                          (int) oid, [o className],
                          ((int) address->mSelector >> 24) & 0xff,
                          ((int) address->mSelector >> 16) & 0xff,
                          ((int) address->mSelector >> 8)  & 0xff,
                          ((int) address->mSelector >> 0)  & 0xff);
        }
    } else {
        DebugLog("Illegal inObjectID %d", (int) oid);
    }
    
    return ret;
}

- (OSStatus) objectIsPropertySettable: (AudioObjectID) oid
                           propertyID: (const AudioObjectPropertyAddress *) address
                outIsPropertySettable: (BOOL *) outIsPropertySettable;
{
    OSStatus ret = kAudioHardwareBadObjectError;
    PAObject *o = [self findObjectByID: oid];
    
    if (o) {
        [o lock];
        *outIsPropertySettable = [o isPropertySettable: address];
        [o unlock];
        
        DebugProperty("asked id %d (%@) for '%c%c%c%c', -> %s",
                      (int) oid, [o className],
                      ((int) address->mSelector >> 24) & 0xff,
                      ((int) address->mSelector >> 16) & 0xff,
                      ((int) address->mSelector >> 8)  & 0xff,
                      ((int) address->mSelector >> 0)  & 0xff,
                      *outIsPropertySettable ? "YES" : "NO");
        
        ret = kAudioHardwareNoError;
    } else {
        DebugLog("Illegal inObjectID %d", (int) oid);
    }
    
    return ret;
}

- (OSStatus) objectGetPropertyDataSize: (AudioObjectID) oid
                            propertyID: (const AudioObjectPropertyAddress *) address
                     qualifierDataSize: (UInt32) qualifierDataSize
                         qualifierData: (const void *) qualifierData
                   outPropertyDataSize: (UInt32 *) outSize
{
    OSStatus ret = kAudioHardwareBadObjectError;
    PAObject *o = [self findObjectByID: oid];
    
    if (o) {
        [o lock];
        ret = [o getPropertyDataSize: address
                   qualifierDataSize: qualifierDataSize
                       qualifierData: qualifierData
                             outSize: outSize];
        [o unlock];
        
        DebugProperty("asked id %d (%@) for '%c%c%c%c' -> %d",
                      (int) oid, [o className],
                      ((int) address->mSelector >> 24) & 0xff,
                      ((int) address->mSelector >> 16) & 0xff,
                      ((int) address->mSelector >> 8)  & 0xff,
                      ((int) address->mSelector >> 0)  & 0xff,
                      outSize ? (int) *outSize : 0);
    } else {
        DebugLog("Illegal inObjectID %d", (int) oid);
    }
    
    return ret;
}

- (OSStatus) objectGetPropertyData: (AudioObjectID) oid
                        propertyID: (const AudioObjectPropertyAddress *) address
                 qualifierDataSize: (UInt32) qualifierDataSize
                     qualifierData: (const void *) qualifierData
                        ioDataSize: (UInt32 *) ioDataSize
                           outData: (void *) outData
{
    OSStatus ret = kAudioHardwareBadObjectError;
    PAObject *o = [self findObjectByID: oid];
    
    if (o) {
        [o lock];
        
        NSLog(@"XXXX asking id %d (%@) for '%c%c%c%c'",
              (int) oid, [o className],
              ((int) address->mSelector >> 24) & 0xff,
              ((int) address->mSelector >> 16) & 0xff,
              ((int) address->mSelector >> 8)  & 0xff,
              ((int) address->mSelector >> 0)  & 0xff);
        
        ret = [o getPropertyData: address
               qualifierDataSize: qualifierDataSize
                   qualifierData: qualifierData
                      ioDataSize: ioDataSize
                         outData: outData];
        [o unlock];
        
        DebugProperty("asked id %d (%@) for '%c%c%c%c'",
                      (int) oid, [o className],
                      ((int) address->mSelector >> 24) & 0xff,
                      ((int) address->mSelector >> 16) & 0xff,
                      ((int) address->mSelector >> 8)  & 0xff,
                      ((int) address->mSelector >> 0)  & 0xff);
    } else {
        DebugLog("Illegal inObjectID %d", (int) oid);
    }
    
    return ret;
}

- (OSStatus) objectSetPropertyData: (AudioObjectID) oid
                        propertyID: (const AudioObjectPropertyAddress *) address
                 qualifierDataSize: (UInt32) qualifierDataSize
                     qualifierData: (const void *) qualifierData
                          dataSize: (UInt32) dataSize
                              data: (const void *) data
{
    OSStatus ret = kAudioHardwareBadObjectError;
    PAObject *o = [self findObjectByID: oid];
    
    if (o) {
        DebugProperty("asked id %d (%@) for '%c%c%c%c'",
                      (int) oid, [o className],
                      ((int) address->mSelector >> 24) & 0xff,
                      ((int) address->mSelector >> 16) & 0xff,
                      ((int) address->mSelector >> 8)  & 0xff,
                      ((int) address->mSelector >> 0)  & 0xff);
        
        [o lock];
        ret = [o setPropertyData: address
               qualifierDataSize: qualifierDataSize
                   qualifierData: qualifierData
                        dataSize: dataSize
                            data: data];
        [o unlock];
    } else {
        DebugLog("Illegal inObjectID %d", (int) oid);
    }
    
    return ret;
}

#pragma mark ### AudioDevice Operations ###

- (OSStatus) deviceCreateIOProcID: (AudioDeviceID) did
                             proc: (AudioDeviceIOProc) proc
                       clientData: (void *) clientData
                      outIOProcID: (AudioDeviceIOProcID *) outIOProcID
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev createIOProcID: proc
                       clientData: clientData
                      outIOProcID: outIOProcID];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceDestroyIOProcID: (AudioDeviceID) did
                            procID: (AudioDeviceIOProcID) procID
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev destroyIOProcID: procID];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceAddIOProc: (AudioDeviceID) did
                        proc: (AudioDeviceIOProc) proc
                  clientData: (void *) clientData;
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev addIOProc: proc
                  clientData: clientData];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceRemoveIOProc: (AudioDeviceID) did
                           proc: (AudioDeviceIOProc) proc
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev removeIOProc: proc];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceStart: (AudioDeviceID) did
                  procID: (AudioDeviceIOProcID) procID
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev start: procID];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceStartAtTime: (AudioDeviceID) did
                        procID: (AudioDeviceIOProcID) procID
            requestedStartTime: (AudioTimeStamp *) requestedStartTime
                         flags: (UInt32) flags
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        return [dev startAtTime: procID
           ioRequestedStartTime: requestedStartTime
                          flags: flags];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceStop: (AudioDeviceID) did
                 procID: (AudioDeviceIOProcID) procID
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev stop: procID];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceRead: (AudioDeviceID) did
              startTime: (const AudioTimeStamp *) startTime
                outData: (AudioBufferList *) outData
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        ret = [dev read: startTime
                outData: outData];
    }
    
    return ret;
}

- (OSStatus) deviceGetCurrentTime: (AudioDeviceID) did
                          outTime: (AudioTimeStamp *) outTime
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev getCurrentTime: outTime];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceTranslateTime: (AudioDeviceID) did
                          inTime: (const AudioTimeStamp *) inTime
                         outTime: (AudioTimeStamp *) outTime
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev translateTime: inTime
                         outTime: outTime];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceGetNearestStartTime: (AudioDeviceID) did
                    requestedStartTime: (AudioTimeStamp *) requestedStartTime
                                 flags: (UInt32) flags
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev getNearestStartTime: requestedStartTime
                                 flags: flags];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceGetPropertyInfo: (AudioDeviceID) did
                           channel: (UInt32) channel
                           isInput: (BOOL) isInput
                        propertyID: (AudioDevicePropertyID) property
                           outSize: (UInt32 *) outSize
                     outIsWritable: (BOOL *) outIsWritable
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev getPropertyInfo: channel
                           isInput: isInput
                        propertyID: property
                           outSize: outSize
                       outWritable: outIsWritable];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceGetProperty: (AudioDeviceID) did
                       channel: (UInt32) channel
                       isInput: (BOOL) isInput
                    propertyID: (AudioDevicePropertyID) property
                    ioDataSize: (UInt32 *) ioDataSize
                       outData: (void *) outData
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev getProperty: channel
                       isInput: isInput
                    propertyID: property
                    ioDataSize: ioDataSize
                       outData: outData];
        [dev unlock];
    }
    
    return ret;
}

- (OSStatus) deviceSetProperty: (AudioDeviceID) did
                          when: (const AudioTimeStamp *) when
                       channel: (UInt32) channel
                       isInput: (BOOL) isInput
                    propertyID: (AudioDevicePropertyID) property
                      dataSize: (UInt32) dataSize
                          data: (const void *) data
{
    OSStatus ret = kAudioHardwareBadDeviceError;
    PADevice *dev = [self findDeviceByID: did];
    
    if (dev) {
        [dev lock];
        ret = [dev setProperty: when
                       channel: channel
                       isInput: isInput
                    propertyID: property
                      dataSize: dataSize
                          data: data];
        [dev unlock];
    }
    
    return ret;
}

#pragma mark ### AudioStream Operations ###

- (OSStatus) streamGetPropertyInfo: (AudioStreamID) sid
                           channel: (UInt32) channel
                        propertyID: (AudioDevicePropertyID) propertyID
                           outSize: (UInt32 *) outSize
                     outIsWritable: (BOOL *) outIsWritable
{
    OSStatus ret = kAudioHardwareBadStreamError;
    PAStream *stream = [self findStreamByID: sid];
    
    if (stream) {
        [stream lock];
        ret = [stream getPropertyInfo: channel
                           propertyID: propertyID
                              outSize: outSize
                        outIsWritable: outIsWritable];
        [stream unlock];
    }
    
    return ret;
}

- (OSStatus) streamGetProperty: (AudioStreamID) sid
                       channel: (UInt32) channel
                    propertyID: (AudioDevicePropertyID) propertyID
                    ioDataSize: (UInt32 *) ioDataSize
                       outData: (void *) outData
{
    OSStatus ret = kAudioHardwareBadStreamError;
    PAStream *stream = [self findStreamByID: sid];
    
    if (stream) {
        [stream lock];
        ret = [stream getProperty: channel
                       propertyID: propertyID
                       ioDataSize: ioDataSize
                             data: outData];
        [stream unlock];
    }
    
    return ret;
}

- (OSStatus) streamSetProperty: (AudioStreamID) sid
                          when: (const AudioTimeStamp *) when
                       channel: (UInt32) channel
                    propertyID: (AudioDevicePropertyID) propertyID
                      dataSize: (UInt32) dataSize
                          data: (const void *) data;
{
    OSStatus ret = kAudioHardwareBadStreamError;
    PAStream *stream = [self findStreamByID: sid];
    
    if (stream) {
        [stream lock];
        ret = [stream setProperty: when
                          channel: channel
                       propertyID: propertyID
                         dataSize: dataSize
                             data: data];
        [stream unlock];
    }
    
    return ret;
}

@end
