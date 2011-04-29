#!/bin/sh

#
# This script is intended to copy resources from the PulseAudio installation
# location to the Framework bundle and make it self-contained.
# Therefore, all dependencies to all binaries are resolved, the dependecy libs
# are copied over and install_name_tool is called to modify the library
# locations inside the binary. The result is a fully encapsulated Framework
# that has no dependencies other than the typical Mac OS X libraries.
#
# Instructions:
#  - use MacPorts to install the depencency libraries for PulseAudio (see the Wiki for a complete list)
#  - build the pulseaudio source tree and set --prefix to /opt/local/pulseaudio
#  - build the Framework sources (which will implicitly link against /opt/local/pulseaudio
#  - install the bundle to /Library/Frameworks (use the install.sh script)
#  - call this script
#

set -e

framework=/Library/Frameworks/PulseAudio.framework

if [ ! -d $framework ]; then
	echo "Huh? $framework does not exist?"
	exit 1
fi

resources=$framework/Resources
papath=/opt/local/pulseaudio/

libpath=$resources/lib
modpath=$resources/lib/modules
binpath=$resources/bin
portlibs=/opt/local/lib

mkdir -p $libpath
mkdir -p $modpath
mkdir -p $binpath

cp -a $papath/bin/* $binpath/
cp -a $papath/lib/*.dylib $libpath/
cp -a $papath/lib/pulse-1.0/* $modpath/

function copy_with_symlinks()
{
	source=$1
	dest=$2
	sdir=$(dirname $source)

	cp -a $source $dest

	while [ "$(stat -f%T $source)" == "@" ]; do
		source=$sdir/$(readlink $source)
		echo "follow link $source"
		cp -a $source $dest
	done
}

function relocate_libs()
{
	pattern=$1; shift
	prefix=$1; shift
	path=$1; shift
	libpath=$1; shift

	echo "prefix $prefix"

	# as the resolving process can raise new dependencies, we have to do this in a loop
	run=1

	while [ $run -eq 1 ]; do
		run=0

		for file in $(find $path -type f -depth 1); do
			(file $file | grep -qi binary) && \
				install_name_tool -id $prefix/$(basename $file) $file
			for deplib in $(otool -L $file | grep $pattern | cut -d" " -f1); do
				basename=$(basename $deplib)
				if [ ! -e $path/$basename ]; then
					echo "Copying $deplib"
					copy_with_symlinks $deplib $libpath/
					run=1
				fi

				#if [ -h $libpath/$baselib ]; then
				#	rlink=$(readlink $deplib)
				#	deplib=$(dirname $deplib)/$rlink
				#	baselib=$rlink
				#	echo "Redirecting to $rlink"
				#fi

				if [ -f $path/$basename ]; then
					install_name_tool -id $prefix/$baselib $path/$basename
				fi

				echo "resolving dependency $deplib for $(basename $file) ..."

				install_name_tool -change $deplib $prefix/$basename $file
			done
		done
	done
}


relocate_libs $portlibs		$libpath	$libpath			$libpath
relocate_libs $portlibs		$libpath	$binpath			$libpath

relocate_libs $papath		$libpath	$libpath			$libpath
relocate_libs $papath		$libpath	$binpath			$libpath
relocate_libs $papath		$libpath	$framework/Versions/Current/	$libpath


