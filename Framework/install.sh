#!/bin/sh

name=PulseAudio.framework
dest=/Library/Frameworks/
config=Release

test -d $dest/$name || mkdir $dest/$name
cp -a build/$config/$name/* $dest/$name/

