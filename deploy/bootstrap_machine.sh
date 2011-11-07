#!/bin/sh

PORTS="autoconf automake intltool libtool libsndfile speex-devel gdbm liboil json-c"

for port in $PORTS; do
	port install $port +universal;
done

# we need to stage the pulseaudio binaries to /Library/Frameworks/PulseAudio.framework, hence
# this path needs to be writable by the current user. This is admittedly a hack, but I lack a
# better solution for now.

chown $USER /Library/Frameworks

