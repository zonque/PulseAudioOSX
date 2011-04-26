/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import "PAObject.h"

@implementation PAObject

@synthesize pluginRef;
@synthesize objectID;

- (id) initWithPluginRef: (AudioHardwarePlugInRef) ref
{
	[super init];

	lock = [[NSLock alloc] init];
	pluginRef = ref;
	
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

- (PAObject *) findObjectByID: (AudioObjectID) searchID
{
	return nil;
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
	
	[array release];
	
	DebugLog("publishing %d objects", count);
	for (UInt32 i = 0; i < count; i++)
		DebugLog(" ... %d", list[i]);
	
	if (count)
		AudioObjectsPublishedAndDied(pluginRef,
					     kAudioObjectSystemObject,
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
	
	[array release];
	
	if (count)
		AudioObjectsPublishedAndDied(pluginRef,
					     kAudioObjectSystemObject,
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
			[array release];
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
			
			[array release];
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
