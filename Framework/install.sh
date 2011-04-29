#!/bin/sh

name=PulseAudio.framework
dest=/Library/Frameworks/
config=Release

rm -fr $dest/$name
test -d $dest/$name || mkdir $dest/$name
rm -fr $dest/$name/*
cp -a build/$config/$name/* $dest/$name/

