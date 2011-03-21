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
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <sys/sysctl.h>

#include "../kext/PAUserClientCommonTypes.h"

#include "pulseAudio.h"
#include "driverClient.h"
#include "notificationCenter.h"

int makeRealtime(void)
{
	struct thread_time_constraint_policy ttcpolicy;
	uint64_t freq = 0;
	size_t size = sizeof(freq);
	int ret;
	
	ret = sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0);
	if (ret < 0) {
		printf("Unable to read CPU frequency, acquisition of real-time scheduling failed.\n");
		return -1;
	}
	
	printf("sysctl for hw.cpufrequency: %llu\n", freq);
	
	/* See http://developer.apple.com/library/mac/#documentation/Darwin/Conceptual/KernelProgramming/scheduler/scheduler.html */
	ttcpolicy.period = freq / 160;
	ttcpolicy.computation = freq / 3300;
	ttcpolicy.constraint = freq / 2200;
	ttcpolicy.preemptible = 1;
	
	ret = thread_policy_set(mach_thread_self(),
				THREAD_TIME_CONSTRAINT_POLICY,
				(thread_policy_t) &ttcpolicy,
				THREAD_TIME_CONSTRAINT_POLICY_COUNT);
	if (ret) {
		printf("Unable to set real-time thread priority (%08x).\n", ret);
		return -1;
	}
	
	printf("Successfully acquired real-time thread priority.\n");
	return 0;
}	

int main (int argc, const char **argv) {
	
	int ret;

	makeRealtime();

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
