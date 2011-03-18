#!/bin/sh

set -e

framework=/Library/Frameworks/pulse.framework

if [ "$1" ]; then
	framework=$1
fi

if [ ! -d "$framework" ]; then
	echo "Error. $framework does not exist"
	exit 1
fi

if [ ! -f template/Info.plist ]; then
	echo "You need to run this script from ./"
	exit 1
fi

resources=$framework/Resources

version=$(grep pa_get_headers_version $resources/include/pulse/version.h | \
	  cut -f2 -d\")
shortversion=${version%%-*}

if [ -z "$version" ]; then
	echo "unable to determine version number."
	exit 1
fi

echo "Fixing up framework for version $version"

# generate skeleton

mkdir -p $framework/Versions/A

oldpwd=$(pwd)

# create symlinks

cd $framework/Versions/
ln -fs A Current
cd A
ln -s ../../Resources/include/pulse Headers
ln -s ../../Resources Resources
cd ../..
ln -fs Versions/Current/Headers Headers
ln -fs Versions/Current/Resources Resources
cd $oldpwd

# generate Info.plist from template

cat template/Info.plist | \
	sed -e "s/##version##/$version/g" \
	> $framework/Versions/A/Resources/Info.plist

# resolve library dependencies

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

libpath=$resources/lib/
modpath=$resources/lib/modules/
binpath=$resources/bin/
portlibs=/opt/local/lib

relocate_libs $portlibs		$resources/lib	$libpath	$libpath
relocate_libs $portlibs		$resources/lib	$binpath	$libpath

cp $libpath/libpulse.dylib $framework/pulse

