#!/bin/sh

export PATH="/opt/local/bin:/opt/local/sbin:$PATH"
export CC="gcc-4.2"
export CFLAGS="-I/opt/local/include -O0 -g"
export LDFLAGS="-L/opt/local/lib"
staging=/Library/Frameworks/pulse.framework/Resources/

./autogen.sh \
	--prefix=$staging		\
	--enable-mac-universal		\
	--disable-dependency-tracking	\
	--disable-jack			\
	--disable-hal			\
	--disable-bluez			\
	--disable-dbus			\
	--disable-avahi			\
	$* && make

rm -fr staging
make install

