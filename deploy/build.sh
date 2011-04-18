#!/bin/sh

framework=/Library/Frameworks/pulse.framework/Resources/

set -x
set -e

function XcodeBuild()
{
	proj=$1; shift
	xcodebuild -project $proj clean
	xcodebuild -project $proj -alltargets -parallelizeTargets -configuration Release
}

base=$(pwd)

################################## pulseaudio source tree ##################################
cd $base/../../pulseaudio/
sh $base/build_pulseaudio.sh

################################## framework ##################################
cd $base/../PulseAudio.framework
./fixupFramework.sh

################################## HAL plugin ##################################
cd $base/../HAL/HALPlugin/
XcodeBuild HALPlugin.xcodeproj

################################## Preference Pane ##################################
cd $base/../PreferencePane/
XcodeBuild PAPreferencePane.xcodeproj

################################## PulseAudioHelper ##################################
cd $base/../PulseAudioHelper/
XcodeBuild PulseAudioHelper.xcodeproj

################################## PulseConsole ##################################
cd $base/../PulseConsole/
XcodeBuild PulseConsole.xcodeproj

