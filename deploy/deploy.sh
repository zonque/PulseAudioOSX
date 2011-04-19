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

outdir=output/
pkgname=PulseAudioOSX-Installer-${version}.pkg
zipname=${pkgname/.pkg/.zip}

test -d ${output} || mkdir ${output}

/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
	--id org.pulseaudio.PulseAudioOSX.installer-${version}	\
	--title "PulseAudio for Mac OS X v${version}"	\
	--resources InstallerResources/		\
	--out ${outdir}/${pkgname}	\
	--version ${version}	\
	--root ${targetdir}	\
	--root-volume-only	\
	--verbose

cd ${outdir}
zip -r -9 $zipname $pkgname

