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

echo "Compile Wireless-tools submodule"

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

