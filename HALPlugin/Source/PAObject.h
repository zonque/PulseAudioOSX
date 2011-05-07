/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <Foundation/Foundation.h>
#import <CoreAudio/AudioHardwarePlugIn.h>

#ifdef ENABLE_DEBUG
#define DebugLog(format, args...) \
        NSLog(@"%s, line %d: " format "\n", \
                __func__, __LINE__, ## args);
#else
#define DebugLog(format, args...) do {} while(0)
#endif

@interface PAObject : NSObject {
        AudioHardwarePlugInRef pluginRef;
        AudioObjectID objectID;
        AudioObjectID owningObjectID;
        NSLock *lock;
}

@property (readonly) AudioHardwarePlugInRef pluginRef;
@property (nonatomic, assign) AudioObjectID objectID;
@property (nonatomic, assign) AudioObjectID owningObjectID;

- (id) initWithPluginRef: (AudioHardwarePlugInRef) ref;

- (void) lock;
- (void) unlock;

- (void) show;

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address;
- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address;

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
               qualifierDataSize: (UInt32) qualifierDataSize
                   qualifierData: (const void *) qualifierData
                         outSize: (UInt32 *) outDataSize;

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

- (PAObject *) findObjectByID: (AudioObjectID) searchID;
- (void) addOwnedObjectsToArray: (NSMutableArray *) array;
- (void) publishOwnedObjects;
- (void) depublishOwnedObjects;

@end
