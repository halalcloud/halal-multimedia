#! /bin/sh
# platform是指x86还是x64;config是指debug/release
platform=$1
config=$2
echo `pwd`
cp -a ./lib* ../../lib/unix/$platform/
cp ../../src/bits/bbits.h ../../include/unix 

# generator document
../../3rdParty/unix/$platform/doxygen-1.8.11/bin/doxygen --help

