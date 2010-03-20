/***
 This file is part of PulseAudioOSX
 
 Copyright 2010 Daniel Mack <daniel@caiaq.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#include <CoreServices/CoreServices.h>
#include <pulse/pulseaudio.h>

#include "../kext/PAUserClientCommonTypes.h"

#include "pulseAudio.h"
#include "driverClient.h"
#include "notificationCenter.h"

int main (int argc, const char **argv) {
	
	int ret;

	ret = pulseAudioClientStart();
	if (ret) {
		printf("pulseAudioClientStart() returned %d\n", ret);
		return -1;
	}

	ret = driverClientStart();
	if (ret) {
		printf("driverClientStart() returned %d\n", ret);
		return -1;
	}

	ret = notificationCenterStart();
	if (ret) {
		printf("deviceClientStart() returned %d\n", ret);
		return -1;
	}

	CFRunLoopRun();

	printf("%s(): terminating ...\n", __func__);

	pulseAudioClientStop();
	
	return 0;
}
