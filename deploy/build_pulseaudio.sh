#!/bin/sh

#
# This needs to be in its own script, as the other parts of the build system
# don't like to have environment variables redefined
#

export PATH="/opt/local/bin:/opt/local/sbin:$PATH"
export CC="gcc-4.2"
#export CFLAGS="-I/opt/local/include -O0 -g"
export CFLAGS="-I/opt/local/include -O2"
export LDFLAGS="-L/opt/local/lib"
staging=/Library/Frameworks/pulse.framework/

./autogen.sh \
	--prefix=$staging/Resources	\
	--enable-mac-universal		\
	--disable-dependency-tracking	\
	--disable-jack			\
	--disable-hal			\
	--disable-bluez			\
	--disable-dbus			\
	--disable-avahi			\
	$* && make

rm -fr $staging
make clean
make install

