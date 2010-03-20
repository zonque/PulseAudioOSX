/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <stdio.h>

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>
#include <pulse/context.h>

#include "pulseAudio.h"

static pa_threaded_mainloop *loop;

static void deviceWriteCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
	printf(" >>>> n_bytes %d\n", nbytes);
}

int pulseAudioClientStart(void)
{
	int ret;

	loop = pa_threaded_mainloop_new();
	if (!loop) {
		printf("%s(): pa_threaded_mainloop_new() failed\n", __func__);
		return -1;
	}
	
	ret = pa_threaded_mainloop_start(loop);
	if (ret) {
		printf("%s(): pa_threaded_mainloop_start() returned %d\n", __func__, ret);
		return ret;
	}

#if 0
	pa_context *pa_con = pa_context_new(pulseAudioAPI(), "audioDaemon");
	pa_context_connect(pa_con, NULL, 0, NULL);
	
//	while (pa_context_get_state(pa_con) != PA_CONTEXT_READY)
		sleep(3);
	
	pa_sample_spec ss;
	
	ss.format = PA_SAMPLE_FLOAT32;
	ss.channels = 2;
	ss.rate = 44100;

	pa_stream *s = pa_stream_new(pa_con, "ficken", &ss, NULL);
	pa_stream_connect_playback(s, NULL, NULL, 0, NULL, NULL);
	pa_stream_set_write_callback(s, deviceWriteCallback, NULL);
#endif

	return 0;
}

void pulseAudioClientStop(void)
{
	if (loop) {
		pa_threaded_mainloop_stop(loop);
		pa_threaded_mainloop_free(loop);
		loop = NULL;
	}
}

pa_mainloop_api *pulseAudioAPI(void)
{
	return pa_threaded_mainloop_get_api(loop);
}
