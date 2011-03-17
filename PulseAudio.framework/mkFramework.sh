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

function relocate_libs()
{
	pattern=$1; shift
	prefix=$1; shift
	path=$1; shift

	# as the resolving process can raise new dependencies, we have to do this in a loop

	while [ "$(find $path -type f -depth 1 | xargs otool -L | grep $pattern)" ]; do
		for lib in $(find $path -type f -depth 1); do
			for deplib in $(otool -L $lib | grep $pattern | cut -d" " -f1); do
				baselib=$(basename $deplib)
				echo "resolving dependency $deplib for $baselib ..."
				test -f $libpath/$baselib || cp $deplib $libpath/
				install_name_tool -id $prefix/$baselib $lib
				install_name_tool -change $deplib $prefix/$baselib $lib

				# check if it worked
				if [ "$(otool -L $lib | grep $deplib)" ]; then
					echo "ERROR. Cannot change name $deplib in $lib"
					echo "You might need to build this library with -headerpad_max_install_names"
					exit 1;
				fi
			done
		done
	done
}

libpath=$output/Versions/A/Resources/
modpath=$libpath/modules/
binpath=$libpath/bin/
portlibs=/opt/local/lib

for iter in 1 2; do
	relocate_libs $portlibs		@loader_path 		$libpath
	relocate_libs $staging		@loader_path 		$libpath
	relocate_libs $portlibs		@loader_path/.. 	$modpath
	relocate_libs $staging		@loader_path/..		$modpath
	relocate_libs $portlibs		@executable_path/..	$binpath
	relocate_libs $staging		@executable_path/..	$binpath
done

