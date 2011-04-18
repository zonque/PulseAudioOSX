#!/bin/sh

#
# Calling this script with no arguments will cause build.sh to be called
# to compile the whole source tree from scratch. Mainly for development
# and debugging of the package maker process, it can also be called with
# an argument to reference a tree of built stuff to be packed.
#

targetdir=$1
version=0.1.0

if [ -z "$targetdir" ]; then
	rm -fr /tmp/paosx.*
	targetdir=$(mktemp -u /tmp/paosx.XXXXXX)
	./build.sh $targetdir
fi

/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
	--root ${targetdir}	\
	--out output/PulseAudioOSX-Installer-${version}.pkg	\
	--id org.pulseaudio.PulseAudioOSX.installer-${version}	\
	--version ${version}	\
	--title "PulseAudio for Mac OS X"	\
	--resources InstallerResources/		\
	--verbose	\
	--root-volume-only

