/***
 This file is part of PulseAudioOSX
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseAudioOSX is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 ***/

#ifndef PAOSX_SHARED_OBJECT_NAMES_H
#define PAOSX_SHARED_OBJECT_NAMES_H

#define PAOSX_HALPluginName			"org.pulseaudio.HALPlugin"
#define PAOSX_HALPluginMsgScanDevices		"org.pulseaudio.HALPlugin.scanDevices"
#define PAOSX_HALPluginMsgAnnounceDevice	"org.pulseaudio.HALPlugin.announceDevice"
#define PAOSX_HALPluginMsgSignOffDevice		"org.pulseaudio.HALPlugin.signOffDevice"
#define PAOSX_HALPluginMsgSetConfiguration	"org.pulseaudio.HALPlugin.setConfiguation"

#define PAOSX_HelperName		"org.pulseaudio.PulseAudioHelper"
#define PAOSX_HelperMsgServiceStarted	"org.pulseaudio.PulseAudioHelper.serviceStarted"

#define PAOSX_HelperMsgSetLocalServerEnabled	"org.pulseaudio.PulseAudioHelper.setLocalServerEnabled"
#define PAOSX_HelperMsgQueryLocalServerEnabled	"org.pulseaudio.PulseAudioHelper.queryLocalServerEnabled"
#define PAOSX_HelperMsgSetStatusBarEnabled	"org.pulseaudio.PulseAudioHelper.setStatusBarEnabled"
#define PAOSX_HelperMsgQueryStatusBarEnabled	"org.pulseaudio.PulseAudioHelper.queryStatusBarEnabled"
#define PAOSX_HelperMsgSetGrowlFlags		"org.pulseaudio.PulseAudioHelper.setGrowlFlags"
#define PAOSX_HelperMsgQueryGrowlFlags		"org.pulseaudio.PulseAudioHelper.queryGrowlFlags"

#define PAOSX_PreferencePaneName		"org.pulseaudio.PulseAudioPreferencePane"

#endif /* PAOSX_SHARED_OBJECT_NAMES_H */
