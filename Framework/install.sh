#!/bin/sh

name=PulseAudio.framework
dest=/Library/Frameworks/
config=Release

rm -fr $dest/$name
cp -a build/$config/$name $dest/

