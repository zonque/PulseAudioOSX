/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#import <mach/mach_time.h>
#import <Foundation/Foundation.h>
#import <PulseAudio/PulseAudio.h>
#import <CoreAudio/AudioHardwarePlugIn.h>

#import "PADeviceAudio.h"
#import "PADevice.h"

@implementation PADeviceAudio

@synthesize ioProcBufferSize;
@synthesize sampleRate;
@synthesize bytesPerSampleFrame;

struct IOProcTracker
{
        AudioDeviceIOProc proc;
        AudioTimeStamp startTime;
        UInt32 startTimeFlags;
        void *clientData;
        Boolean enabled;

        // for one-shot DeviceRead() listeners
        /*
         MPSemaphoreID semaphore;
         AudioBufferList *bufferList;
         */

        IOProcTracker *prev;
        IOProcTracker *next;
};

- (id) initWithPADevice: (PADevice *) _device
{
        [super init];

        device = _device;
        lock = [NSLock alloc];
        ioProcBufferSize = 1024;
        sampleRate = 44100.0f;
        bytesPerSampleFrame = sizeof(Float32) * 2;

        return self;
}

- (void) dealloc
{
        while (tracker) {
                IOProcTracker *next = tracker->next;
                free(tracker);
                tracker = next;
        }

        [lock release];
        [super dealloc];
}

#pragma mark ### IOProc housekeeping ###

- (IOProcTracker *) findTracker: (void *) value
{
        [lock lock];

        for (IOProcTracker *t = tracker; tracker; tracker = tracker->next)
                if ((t == value) ||
                    (t->proc == value)) {
                        [lock unlock];
                        return t;
                }

        [lock unlock];

        return NULL;
}

- (BOOL) hasActiveProcs
{
        for (IOProcTracker *t = tracker; tracker; tracker = tracker->next)
                if (t->enabled)
                        return YES;

        return NO;
}

- (AudioDeviceIOProcID) createIOProcID: (AudioDeviceIOProc) proc
                            clientData: (void *) clientData
{
        IOProcTracker *new = (IOProcTracker *) malloc(sizeof(IOProcTracker));

        bzero(new, sizeof(*new));

        new->proc = proc;
        new->clientData = clientData;

        [lock lock];

        if (tracker) {
                IOProcTracker *t = tracker;

                while (t->next)
                        t = t->next;

                t->next = new;
                new->prev = t;
        } else {
                tracker = new;
        }

        [lock unlock];

        return (AudioDeviceIOProcID) new;
}

- (void) removeIOProcID: (AudioDeviceIOProcID) procID
{
        IOProcTracker *t = [self findTracker: procID];

        if (!t)
                return;

        if (t == tracker) {
                [lock lock];
                free(tracker);
                tracker = t->next;
                [lock unlock];
                return;
        }

        [lock lock];

        if (t->next)
                t->next->prev = t->prev;

        if (t->prev)
                t->prev->next = t->next;

        [lock unlock];

        free(t);
}

- (void) setIOProcIDEnabled: (AudioDeviceIOProcID) procID
                    enabled: (BOOL) enabled
{
        IOProcTracker *t = [self findTracker: procID];
        if (t) {
                [lock lock];
                t->enabled = enabled;
                [lock unlock];
        }
}

- (void) setAllIOProcs: (BOOL) enabled
{
        [lock lock];

        for (IOProcTracker *t = tracker; tracker; tracker = tracker->next)
                t->enabled = enabled;

        [lock unlock];
}

- (void) setStartTimeForProcID: (AudioDeviceIOProcID) procID
                     timeStamp: (AudioTimeStamp *) timeStamp
                         flags: (UInt32) flags
{
        IOProcTracker *t = [self findTracker: procID];
        if (!t)
                return;

        [lock lock];
        memcpy(&t->startTime, timeStamp, sizeof(*timeStamp));
        t->startTimeFlags = flags;
        t->enabled = YES;
        [lock unlock];
}

#pragma mark stream configuration handling ###

- (UInt32) countActiveChannels
{
        return 2;
}

- (BOOL) channelIsActive: (UInt32) channel
{
        return YES;
}

- (void) setChannelActive: (UInt32) channel
                   active: (BOOL) active
{
}

#pragma mark ### audio processing ###

- (UInt32) PAServerConnection: (PAServerConnection *) connection
              hasPlaybackData: (Byte *) playbackData
                   recordData: (const Byte *) recordData
                     byteSize: (UInt32) byteSize
{
        Float64 usecPerFrame = 1000000.0 / sampleRate;
        UInt32 ioProcSize = ioProcBufferSize * bytesPerSampleFrame;

        AudioBufferList inputList, outputList;
        memset(&inputList, 0, sizeof(inputList));
        memset(&outputList, 0, sizeof(outputList));

        AudioTimeStamp now, inputTime, outputTime;
        memset(&now, 0, sizeof(AudioTimeStamp));
        memset(&inputTime, 0, sizeof(AudioTimeStamp));
        memset(&outputTime, 0, sizeof(AudioTimeStamp));

        now.mRateScalar = inputTime.mRateScalar = outputTime.mRateScalar = 1.0;
        now.mHostTime =  inputTime.mHostTime = outputTime.mHostTime = mach_absolute_time();
        now.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;

        inputTime.mFlags = now.mFlags | kAudioTimeStampSampleTimeValid;
        outputTime.mFlags = now.mFlags | kAudioTimeStampSampleTimeValid;

        AudioObjectID deviceID = device.objectID;
        UInt32 count = 0, framesPlayed = 0;

        while (byteSize >= ioProcSize) {
                outputList.mBuffers[0].mNumberChannels = 2;
                outputList.mBuffers[0].mDataByteSize = ioProcSize;
                outputList.mBuffers[0].mData = playbackData;
                outputList.mNumberBuffers = 1;

                inputList.mBuffers[0].mNumberChannels = 2;
                inputList.mBuffers[0].mDataByteSize = ioProcSize;
                inputList.mBuffers[0].mData = playbackData;
                inputList.mNumberBuffers = 1;

                inputTime.mSampleTime = (framesPlayed * usecPerFrame) - 10000;
                outputTime.mSampleTime = (framesPlayed * usecPerFrame) + 10000;

                for (IOProcTracker *t = tracker; t; t = t->next) {
                        BOOL enabled = t->enabled;

                        if ((t->startTime.mFlags & kAudioTimeStampHostTimeValid) &&
                            (t->startTime.mHostTime > now.mHostTime))
                                enabled = false;

                        if ((t->startTime.mFlags & kAudioTimeStampSampleTimeValid) &&
                            (t->startTime.mSampleTime > now.mSampleTime))
                                enabled = false;

                        //enabled = false;

                        if (enabled) {
                                t->proc(deviceID, &now,
                                        &inputList, &inputTime,
                                        &outputList, &outputTime,
                                        t->clientData);
                        }
                }

                byteSize -= ioProcSize;
                //inputBuffer += ioProcSize;
                playbackData += ioProcSize;
                count += ioProcSize;
                framesPlayed += ioProcSize / ioProcBufferSize;
        }

        //DebugLog("writing %d, count %d", outputBufferPos - buf, count);

        return count;
}

@end
