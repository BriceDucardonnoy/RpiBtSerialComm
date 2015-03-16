#!/bin/bash

#if [ $# > 0 ] && [ "$1" == "arm" ]; then
HOST="";
PWD=`pwd`

if [ "$1" == "arm" ]; then
	which arm-linux-gnueabihf-gcc
	if [ $? -ne 0 ]; then
		#echo "First compilation time on this console: update the PATH"	
		echo "Load the ARM path"
		source ${HOME}/arm/rpi/rpi-linaro-environment-setup
		# WARNING: at the end of the script execution, PATH is released (or do export PATH...)
		#echo `which arm-linux-gnueabihf-gcc`
	fi
	HOST=arm-linux-gnueabihf
fi

echo "Clean repositories"
# Clean repositories
[ -d autom4te.cache ] && rm -rf autom4te.cache
[ -d install ] && rm -rf install
mkdir install
#find src -name "*\.deps" -exec rm -rf {} \;
rm -rf `find src -name "*\.deps"`
find src -name "*\.o" -exec rm -rf {} \;

echo "Update submodules"
# Update submodules
git submodule init
git submodule update

echo "Compile external libraries"
####### How to recompile lib nl. This operation is necessary at the 1st build to update path in .pc (package config) files #######
#cd dependencies/libnl-3.2.25
#make distclean
#rm -r install
#mkdir install && ./configure --prefix=`pwd`/install --host=${HOST}
#make install
#cd ../../
#cd ../iw-3.17/
#PREFIX=`pwd`/install CC=arm-linux-gnueabihf-gcc PKG_CONFIG_PATH=`pwd`/../libnl-3.2.25/install/lib/pkgconfig/ make install

echo "Invoke autotools"
# Autotools invocation
libtoolize -c --automake
aclocal
#autoheader # We don't use config.h or config.h.in so useless
automake --add-missing --foreign
autoconf
#./configure --prefix=/tmp/bdy/streameverywhere/install --host=arm-angstrom-linux-gnueabi
#./configure --prefix=/tmp/bdy/streameverywhere/install --host=arm-linux-gnueabihf
./configure --prefix=${PWD}/install --host=${HOST}

#TODO: make some init to avoid the make -k
#TODO BDY: run it from build-RPi to have all .o and .deps in build-RPi folder
#cd build-RPi && ../configure --prefix=${PWD}/install --host=${HOST}
echo "Configuration done. You can do 'make clean install' now"


