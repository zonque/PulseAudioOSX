#!/bin/sh

set -e

#
# Calling this script with no arguments will cause build.sh to be called
# to compile the whole source tree from scratch. Mainly for development
# and debugging of the package maker process, it can also be called with
# an argument to reference a tree of built stuff to be packed.
#

source version.inc

targetdir=$1

if [ -z "$targetdir" ]; then
	rm -fr /tmp/paosx.*
	targetdir=$(mktemp -u /tmp/paosx.XXXXXX)
	./build.sh $targetdir
fi

./updateReleaseNotes.sh $targetdir $version

/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
	--out output/PulseAudioOSX-Installer-${version}.pkg	\
	--id org.pulseaudio.PulseAudioOSX.installer-${version}	\
	--title "PulseAudio for Mac OS X"	\
	--resources InstallerResources/		\
	--version ${version}	\
	--root ${targetdir}	\
	--root-volume-only	\
	--verbose

