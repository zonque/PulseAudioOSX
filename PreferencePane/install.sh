#!/bin/sh

conf=$1

if [ -z "$conf" ]; then
	conf="Release"
fi

dest=/Library/PreferencePanes/PulseAudio.prefPane

rm -fr $dest
cp -r build/${conf}/PulseAudio.prefPane /Library/PreferencePanes/

echo "now call /Applications/System\ Preferences.app/Contents/MacOS/System\ Preferences $dest"

