/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <pulseaudio@zonque.org>
 
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
