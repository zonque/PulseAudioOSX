#!/bin/sh

#
# This needs to be in its own script, as the other parts of the build system
# don't like to have environment variables redefined
#

set -x

export PATH="/opt/local/bin:/opt/local/sbin:$PATH"
export CC="gcc"
#export CFLAGS="-I/opt/local/include -O0 -g"
export CFLAGS="-I/opt/local/include -O2"
export LDFLAGS="-L/opt/local/lib"
staging=/Library/Frameworks/PulseAudio.framework/Contents/MacOS

if [ ! -d $staging ]; then
	echo "$staging does not exist. Please create and make it writeable for your user"
	exit 1;
fi

./autogen.sh \
	--prefix=$staging \
	--enable-mac-universal		\
	--disable-dependency-tracking	\
	--disable-jack			\
	--disable-glib2			\
	--disable-hal			\
	--disable-bluez			\
	--disable-dbus			\
	--disable-avahi			\
	--with-mac-version-min=10.6	\
	--with-mac-sysroot=/Developer/SDKs/MacOSX10.6.sdk	\
	$* && make clean install

