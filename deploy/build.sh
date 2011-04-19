#!/bin/sh

targetdir=$1

if [ -z "$targetdir" ]; then
	echo "Usage: $0 <targetdir>"
	exit 1
fi

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
cd $base/../Framework
./fixupFramework.sh
dest=$targetdir/Library/Frameworks/
mkdir -p $dest
cp -a /Library/Frameworks/pulse.framework $dest

################################## HAL plugin ##################################
cd $base/../HALPlugin/
XcodeBuild HALPlugin.xcodeproj
dest=$targetdir/Library/Audio/Plugins/HAL/
mkdir -p $dest
cp -a build/Release/PulseAudio.plugin $dest

################################## Preference Pane ##################################
cd $base/../PreferencePane/
XcodeBuild PAPreferencePane.xcodeproj
dest=$targetdir/Library/PreferencePanes/
mkdir -p $dest
cp -a build/Release/PulseAudio.prefPane $dest

################################## PulseAudioHelper ##################################
cd $base/../PulseAudioHelper/
XcodeBuild PulseAudioHelper.xcodeproj
dest=$targetdir/Library/StartupItems/PulseAudioHelper
mkdir -p $dest
cp -a build/Release/PulseAudioHelper $dest
cp -a StartupParameters.plist $dest

################################## PulseConsole ##################################
cd $base/../PulseConsole/
XcodeBuild PulseConsole.xcodeproj
dest=$targetdir/Applications/
mkdir -p $dest
cp -a build/Release/PulseConsole.app $dest


echo "Built and staged into $targetdir"

