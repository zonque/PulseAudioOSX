/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import "PAObject.h"

@implementation PAObject

@synthesize pluginRef;
@synthesize objectID;
@synthesize owningObjectID;

- (id) initWithPluginRef: (AudioHardwarePlugInRef) ref
{
        [super init];

        lock = [[NSLock alloc] init];
        pluginRef = ref;
        owningObjectID = kAudioObjectSystemObject;

        return self;
}

- (void) dealloc
{
        [lock release];
        [super dealloc];
}

- (void) lock
{
        [lock lock];
}

- (void) unlock
{
        [lock unlock];
}

/*
- (id) retain
{
        DebugLog(" retainCount %d (%@)", [self retainCount], [self className])
        [super retain];
        return self;
}

- (void) release
{
        DebugLog(" retainCount %d (%@)", [self retainCount], [self className])
        [super release];
}
*/

- (PAObject *) findObjectByID: (AudioObjectID) searchID
{
        if (objectID == searchID)
                return self;

        PAObject *ret = nil;
        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
        [self addOwnedObjectsToArray: array];

        for (PAObject *o in array) {
                ret = [o findObjectByID: searchID];
                if (ret)
                        break;
        }

        return ret;
}

- (void) addOwnedObjectsToArray: (NSMutableArray *) array
{
        // the top-level class implementation does nothing
}

- (void) publishOwnedObjects
{
        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
        [self addOwnedObjectsToArray: array];
        AudioObjectID list[[array count]];
        UInt32 count = 0;

        for (PAObject *o in array)
                list[count++] = o.objectID;

        DebugLog("publishing %d objects with pluginRef %p (owningObjectID %d)", count, pluginRef, owningObjectID);
        for (UInt32 i = 0; i < count; i++)
                DebugLog(" ... %d", list[i]);

        if (count)
                AudioObjectsPublishedAndDied(pluginRef,
                                             owningObjectID,
                                             count, list,
                                             0, NULL);
}

- (void) depublishOwnedObjects
{
        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
        [self addOwnedObjectsToArray: array];
        AudioObjectID list[[array count]];
        UInt32 count = 0;

        for (PAObject *o in array)
                list[count++] = o.objectID;

        DebugLog("DEpublishing %d objects with pluginRef %p (owningObjectID %d)", count, pluginRef, owningObjectID);
        for (UInt32 i = 0; i < count; i++)
                DebugLog(" ... %d", list[i]);

        if (count)
                AudioObjectsPublishedAndDied(pluginRef,
                                             owningObjectID,
                                             0, NULL,
                                             count, list);
}

#pragma mark ### Plugin interface ###

- (void) show
{
        NSLog(@"%@", self);
}

- (BOOL) hasProperty: (const AudioObjectPropertyAddress *) address
{
        switch (address->mSelector) {
                case kAudioObjectPropertyOwnedObjects:
                case kAudioObjectPropertyListenerAdded:
                case kAudioObjectPropertyListenerRemoved:
                        return YES;
        }

        return NO;
}

- (BOOL) isPropertySettable: (const AudioObjectPropertyAddress *) address
{
        switch (address->mSelector) {
                case kAudioObjectPropertyListenerAdded:
                case kAudioObjectPropertyListenerRemoved:
                        return YES;
        }

        return NO;
}

- (OSStatus) getPropertyDataSize: (const AudioObjectPropertyAddress *) address
               qualifierDataSize: (UInt32) qualifierDataSize
                   qualifierData: (const void *) qualifierData
                         outSize: (UInt32 *) outDataSize
{
        switch (address->mSelector) {
                case kAudioObjectPropertyOwnedObjects: {
                        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
                        [self addOwnedObjectsToArray: array];
                        *outDataSize = sizeof(AudioObjectID) * [array count];
                        return kAudioHardwareNoError;
                }
                case kAudioObjectPropertyListenerAdded:
                case kAudioObjectPropertyListenerRemoved:
                        *outDataSize = sizeof(AudioObjectPropertyAddress);
                        return kAudioHardwareNoError;
        }

        *outDataSize = 0;

        DebugLog("Unhandled property for id %d: '%c%c%c%c'",
                 (int) self.objectID,
                 ((int) address->mSelector >> 24) & 0xff,
                 ((int) address->mSelector >> 16) & 0xff,
                 ((int) address->mSelector >> 8) & 0xff,
                 ((int) address->mSelector >> 0) & 0xff);

        return kAudioHardwareUnknownPropertyError;
}

- (OSStatus) getPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                  ioDataSize: (UInt32 *) ioDataSize
                     outData: (void *) outData;
{
        switch (address->mSelector) {
                case kAudioObjectPropertyOwnedObjects: {
                        NSMutableArray *array = [NSMutableArray arrayWithCapacity: 0];
                        [self addOwnedObjectsToArray: array];
                        AudioObjectID *out = outData;

                        for (PAObject *o in array)
                                *out++ = o.objectID;

                        return kAudioHardwareNoError;
                }
                case kAudioObjectPropertyListenerAdded:
                case kAudioObjectPropertyListenerRemoved:
                        //ASSERT
                        memset(outData, 0, *ioDataSize);
                        return kAudioHardwareNoError;
        }

        DebugLog("Unhandled property for id %d: '%c%c%c%c'",
                 (int) self.objectID,
                 ((int) address->mSelector >> 24) & 0xff,
                 ((int) address->mSelector >> 16) & 0xff,
                 ((int) address->mSelector >> 8) & 0xff,
                 ((int) address->mSelector >> 0) & 0xff);

        return kAudioHardwareUnknownPropertyError;
}

- (OSStatus) setPropertyData: (const AudioObjectPropertyAddress *) address
           qualifierDataSize: (UInt32) qualifierDataSize
               qualifierData: (const void *) qualifierData
                    dataSize: (UInt32) dataSize
                        data: (const void *) data
{
        switch (address->mSelector) {
                case kAudioObjectPropertyListenerAdded: {
                        const AudioObjectPropertyAddress *listen = data;
                        DebugLog("Added listener for property '%c%c%c%c'",
                                 ((int) listen->mSelector >> 24) & 0xff,
                                 ((int) listen->mSelector >> 16) & 0xff,
                                 ((int) listen->mSelector >> 8) & 0xff,
                                 ((int) listen->mSelector >> 0) & 0xff);
                        return kAudioHardwareNoError;
                }
                case kAudioObjectPropertyListenerRemoved:
                        return kAudioHardwareNoError;
        }

        DebugLog("Unhandled property for id %d: '%c%c%c%c'",
                 (int) self.objectID,
                 ((int) address->mSelector >> 24) & 0xff,
                 ((int) address->mSelector >> 16) & 0xff,
                 ((int) address->mSelector >> 8) & 0xff,
                 ((int) address->mSelector >> 0) & 0xff);

        return kAudioHardwareUnknownPropertyError;
}

@end
