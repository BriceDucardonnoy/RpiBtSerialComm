#!/bin/bash

# Variables
declare -a networks=('www.google.com' '8.8.8.8' 'Gateway' 'DNS');
declare -a retcodes=(0 1 2 2);
counter=0
result=-1

# Functions
getGateway() {
return ;
}

getDNS() {
return ;
}

pingInterface() {
# Usage: pingInterface <interface> <address to ping> <return code if ok>
# ping method returns
#	0: ping ok
#	2: Network is unreachable or unknown host
	if [ $# -ne 3 ]; then
		echo "3 parameters are expected"
		return -1
	fi

	echo "Try to join $2 from $1"
	ping -c 1 -I $1 $2
	if [ $? -eq 0 ]; then
		return $3
	else
		echo "Failure!"
		counter=`expr $counter + 1`
		if [ $counter -lt ${#networks[@]} ]; then
			result= echo `pingInterface $1 ${networks[$counter]} ${retcodes[$counter]}`
		else
			result=-1
		fi
		return $result
	fi
	return 0
}

# Start main part
# Returns:
#-1|255	No network pingable
#	0	www.google.com pingable	-> all is good
#	1	8.8.8.8 pingable 		-> probably DNS problem
#	2	Gateway or DNS 			-> internal network reachable, probably not opened to the extern world

if [ $# -ne 1 ]; then
	echo "Usage $0 <ETH0|eth0|WLAN0|wlan0>"
	exit -1
fi

case "$1" in 
	ETH0|eth0)
		result= echo `pingInterface eth0 ${networks[$counter]} ${retcodes[$counter]}`
		exit $result
		;;
	WLAN0|wlan0)
		result= echo `pingInterface "wlan0" ${networks[$counter]} ${retcodes[$counter]}`
		exit $result
		;;
	*)
		exit -1
		;;
esac
# TODO BDY: write result in a file
exit 0

