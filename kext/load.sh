#!/bin/sh

sync

kext=PulseAudioKext.kext
conf=$1

if [ -z "$conf" ]; then
	conf=Debug
fi

kextunload -b org.pulseaudio.driver.PulseAudioKext

mkdir /tmp/dbg

cd build/$conf

cp -r ${kext} /tmp/
chown -R root /tmp/PulseAudioKext.kext
cd /tmp
mkdir sym

if test -x /usr/bin/kextutil; then
	kextutil -v 3 -t ${kext}
else
	kextload -v 3 ${kext}
fi

