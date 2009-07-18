#!/bin/sh

conf=$1

if [ -z "$conf" ]; then
	conf="Release"
fi

rm -fr /Library/PreferencePanes/PulseAudio.prefPane
cp -r build/${conf}/PulseAudio.prefPane /Library/PreferencePanes/

echo "now call /Applications/System\ Preferences.app/Contents/MacOS/System\ Preferences"

