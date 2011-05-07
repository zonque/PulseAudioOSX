/***
 This file is part of PulseAudioOSX

 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>

 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License (LGPL) as
 published by the Free Software Foundation; either version 2.1 of the
 License, or (at your option) any later version.
 ***/

#import <pulse/pulseaudio.h>
#import "PAServerConnectionInternal.h"
#import "PAServerConnectionAudio.h"

@implementation PAServerConnectionAudio

@synthesize sinkForPlayback;
@synthesize sourceForRecord;

- (void) streamStartedCallback: (pa_stream *) stream
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        NSString *device = [NSString stringWithCString: pa_stream_get_device_name(stream)
                                              encoding: NSASCIIStringEncoding];

        if (stream == PAPlaybackStream)
                if (!sinkForPlayback)
                        sinkForPlayback = [device retain];

        if (stream == PARecordStream)
                if (!sourceForRecord)
                        sourceForRecord = [device retain];

        BOOL ready = YES;

        if (PAPlaybackStream && !pa_stream_get_state(PAPlaybackStream) == PA_STREAM_READY)
                ready = NO;

        if (PARecordStream && !pa_stream_get_state(PARecordStream) == PA_STREAM_READY)
                ready = NO;

        if (ready)
                [serverConnection performSelectorOnMainThread: @selector(setAudioStarted)
                                                   withObject: nil
                                                waitUntilDone: NO];

        [pool drain];
}

- (void) streamWriteCallback: (pa_stream *) stream
                      nBytes: (size_t) nBytes
{
        size_t written;

        do {
                Byte *outputBuffer = NULL;

                written = nBytes;
                pa_stream_begin_write(PAPlaybackStream, (void **) &outputBuffer, &written);

                written = [serverConnection.delegate PAServerConnection: serverConnection
                                                        hasPlaybackData: outputBuffer
                                                             recordData: NULL
                                                               byteSize: written];

                pa_stream_write(PAPlaybackStream, outputBuffer, written, NULL, 0,
                                (pa_seek_mode_t) PA_SEEK_RELATIVE);

                nBytes -= written;

        } while (written > 0 && nBytes > 0);
}

- (void) streamReadCallback: (pa_stream *) stream
                     nBytes: (size_t) nBytes
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");
}

- (void) streamOverflowCallback: (pa_stream *) stream
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");
}

- (void) streamUnderflowCallback: (pa_stream *) stream
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");
}

- (void) streamEventCallback: (pa_stream *) stream
                        name: (const char *) name
                    propList: (pa_proplist *) propList
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");
}

- (void) streamBufferAttrCallback: (pa_stream *) stream
{
        NSLog(@"%s() %s", __func__, stream == PARecordStream ? "input" : "output");
}

#pragma mark ### static wrappers ###
// streams
static void staticStreamStartedCallback(pa_stream *stream, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamStartedCallback: stream];
}

static void staticStreamWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamWriteCallback: stream
                         nBytes: nbytes];
}

static void staticStreamReadCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamReadCallback: stream
                        nBytes: nbytes];
}

static void staticStreamOverflowCallback(pa_stream *stream, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamOverflowCallback: stream];
}

static void staticStreamUnderflowCallback(pa_stream *stream, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamUnderflowCallback: stream];
}

static void staticStreamEventCallback(pa_stream *stream, const char *name, pa_proplist *pl, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamEventCallback: stream
                           name: name
                       propList: pl];
}

static void staticStreamBufferAttrCallback(pa_stream *stream, void *userdata)
{
        PAServerConnectionAudio *sc = userdata;
        [sc streamBufferAttrCallback: stream];
}

#pragma mark ### PAServerConnectionAudio ###

- (id) initWithPAServerConnection: (PAServerConnection *) _serverConnection
                          context: (pa_context *) _context
                        nChannels: (UInt32) nChannels
                       sampleRate: (Float64) _sampleRate
                 ioProcBufferSize: (UInt32) _ioProcBufferSize
                  sinkForPlayback: (NSString *) sink
                  sourceForRecord: (NSString *) source
{
        [super init];

        NSLog(@"%s()", __func__);

        serverConnection = _serverConnection;
        PAContext = _context;

        ioBufferFrameSize = _ioProcBufferSize;

        if (sinkForPlayback)
                [sinkForPlayback release];

        sinkForPlayback = [sink retain];

        if (sourceForRecord)
                [sourceForRecord release];

        sourceForRecord = [source retain];

        sampleSpec.format = PA_SAMPLE_FLOAT32;
        sampleSpec.rate = _sampleRate;
        sampleSpec.channels = nChannels;

        bufAttr.tlength = ioBufferFrameSize * pa_frame_size(&sampleSpec);
        bufAttr.maxlength = -1;
        bufAttr.minreq = -1;
        bufAttr.prebuf = -1;

        int ret = 0;

        PAPlaybackStream = pa_stream_new(PAContext, "Playback", &sampleSpec, NULL);

        if (!PAPlaybackStream)
                return nil;

        pa_stream_set_event_callback(PAPlaybackStream, staticStreamEventCallback, self);
        pa_stream_set_write_callback(PAPlaybackStream, staticStreamWriteCallback, self);
        pa_stream_set_started_callback(PAPlaybackStream, staticStreamStartedCallback, self);
        pa_stream_set_overflow_callback(PAPlaybackStream, staticStreamOverflowCallback, self);
        pa_stream_set_underflow_callback(PAPlaybackStream, staticStreamUnderflowCallback, self);
        pa_stream_set_buffer_attr_callback(PAPlaybackStream, staticStreamBufferAttrCallback, self);

        /*
         PARecordStream = pa_stream_new(PAContext, "Record", &sampleSpec, NULL);
         //pa_stream_set_read_callback(PARecordStream, staticStreamReadCallback, self);
         pa_stream_set_event_callback(PARecordStream, staticStreamEventCallback, self);
         pa_stream_set_overflow_callback(PARecordStream, staticStreamOverflowCallback, self);
         pa_stream_set_underflow_callback(PARecordStream, staticStreamUnderflowCallback, self);
         pa_stream_set_buffer_attr_callback(PARecordStream, staticStreamBufferAttrCallback, self);
         pa_stream_connect_record(PARecordStream, NULL, &bufAttr, (pa_stream_flags_t) 0);
         */

        if (PAPlaybackStream)
                ret = pa_stream_connect_playback(PAPlaybackStream,
                                                 sink ? [sink cStringUsingEncoding: NSASCIIStringEncoding] : NULL,
                                                 &bufAttr,
                                                 (pa_stream_flags_t)  (PA_STREAM_INTERPOLATE_TIMING |
                                                                       PA_STREAM_AUTO_TIMING_UPDATE),
                                                 NULL, NULL);

        if (ret != 0)
                return nil;


        return self;
}

- (void) dealloc
{
        if (PAPlaybackStream) {
                pa_stream_flush(PAPlaybackStream, NULL, NULL);
                pa_stream_disconnect(PAPlaybackStream);
                pa_stream_unref(PAPlaybackStream);
                PAPlaybackStream = NULL;
        }

        if (PARecordStream) {
                pa_stream_disconnect(PARecordStream);
                pa_stream_unref(PARecordStream);
                PARecordStream = NULL;
        }

        if (sinkForPlayback) {
                [sinkForPlayback release];
                sinkForPlayback = nil;
        }

        if (sourceForRecord) {
                [sourceForRecord release];
                sourceForRecord = nil;
        }

        pa_xfree(inputDummyBuffer);
        inputDummyBuffer = NULL;

        pa_xfree(outputDummyBuffer);
        outputDummyBuffer = NULL;

        [super dealloc];
}

@end

