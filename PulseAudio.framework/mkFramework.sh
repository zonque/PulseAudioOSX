#!/bin/bash

set -e

staging=$1
output=PulseAudio.framework

if [ "$2" ]; then
	output=$2
	autodel=no
fi

if [ -z "$staging" ]; then
	echo "Usage: $0 <stagingdir> [<outputdir>]"
	exit 1
fi

if [ ! -f template/Info.plist ]; then
	echo "You need to run this script from ./"
	exit 1
fi

version=$(grep pa_get_headers_version $staging/include/pulse/version.h | \
	  cut -f2 -d\")
shortversion=${version%%-*}

if [ -z "$version" ]; then
	echo "unable to determine version number."
	exit 1
fi

echo "Building framework for version $version"

rm -fr $output

# generate skeleton

mkdir -p $output/Versions/A/Headers
mkdir -p $output/Versions/A/Resources

oldpwd=$(pwd)

# create symlinks

cd $output/Versions/
ln -s A Current
cd $oldpwd

cd $output/
ln -s Versions/Current/Headers Headers
ln -s Versions/Current/Resources Resources
cd $oldpwd

# generate Info.plist from template

cat template/Info.plist | \
	sed -e "s/##version##/$version/g" \
	> $output/Versions/A/Resources/Info.plist

# copy resources

cp -r $staging/include/pulse/* $output/Versions/A/Headers/
cp -r $staging/bin $output/Versions/A/Resources/
cp -r $staging/lib/*.dylib $output/Versions/A/Resources/
cp -r $staging/lib/pulse-$shortversion/modules $output/Versions/A/Resources/

# resolve library dependencies

libpath=$output/Versions/A/Resources/

# as the resolving process can raise new dependencies to /opt/local/...,
# we have to do this in a loop

while [ "$(otool -L $libpath/*.dylib | grep /opt/local)" ]; do
	for lib in $libpath/*.dylib; do
		for deplib in $(otool -L $lib | grep /opt/local | cut -d" " -f1); do
			baselib=$(basename $deplib)
			echo "resolving dependency $deplib for $baselib ..."
			test -f $libpath/$baselib || cp $deplib $libpath/
			install_name_tool -id "@loader_path/$baselib" $lib
			install_name_tool -change $deplib "@loader_path/$baselib" $lib

			# check if it worked
			if [ "$(otool -L $lib | grep $deplib)" ]; then
				echo "ERROR. Cannot change name $deplib in $lib"
				echo "You might need to build this library with -headerpad_max_install_names"
				exit 1;
			fi
		done
	done
done

binpath=$libpath/bin/

for bin in $binpath/*; do
	for deplib in 	\
		$(otool -L $bin | grep /opt/local | cut -d" " -f1)	\
		$(otool -L $bin | grep libpulse | cut -d" " -f1); do
			baselib=$(basename $deplib)
			install_name_tool -change $deplib "@executable_path/../$baselib" $bin
	done
done

