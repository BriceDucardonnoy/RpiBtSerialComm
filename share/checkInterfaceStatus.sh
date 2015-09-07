#!/bin/bash

# Variables
declare -a networks=('www.google.com' '8.8.8.8' 'Gateway' 'DNS');
declare -a retcodes=(0 1 2 2);
counter=0

# Functions
# Usage: getGateway <interface>
getGateway() {
	gw=`route -n | grep ${1} | grep UG | awk -F ' ' '{print $2}'`
	if [ ${#gw} -gt 0 ]; then
		echo "Not empty: <$gw>"
		networks[2]=$gw
	fi
#	route -n | grep eth0 | grep UG | awk -F ' ' '{print $2}'
}

# Usage: getDNS <interface>
getDNS() {
return ;
}

pingInterface() {
# Usage: pingInterface <interface> <address to ping> <return code if ok>
# ping method returns
#	0: ping ok
#	2: Network is unreachable or unknown host
	local result=-1
	if [ $# -ne 3 ]; then
		echo "3 parameters are expected"
		return -1
	fi

	echo "Try to join $2 from $1"
	ping -c 1 -I $1 $2
	if [ $? -eq 0 ]; then
		echo "Success!"
		result=$3
	else
		echo "Failure!"
		counter=`expr $counter + 1`
		if [ $counter -lt ${#networks[@]} ]; then
			result= echo `pingInterface $1 ${networks[$counter]} ${retcodes[$counter]}`
		else
			result=-1
		fi
	fi
	echo "Return $result"
	return $result
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

#gw=$(getGateway eth1)

case "$1" in 
	ETH0|eth0)
		interface="eth0"
		;;
	WLAN0|wlan0)
		interface="wlan0"
		;;
	*)
		exit -1
		;;
esac

while true; do
#	retcode= echo `pingInterface $interface ${networks[$counter]} ${retcodes[$counter]}`
#	retcode=$(pingInterface $interface ${networks[$counter]} ${retcodes[$counter]})
	getGateway $interface
	pingInterface $interface ${networks[$counter]} ${retcodes[$counter]}
	retcode=$?
	echo "Will write $retcode"
	if [ $retcode -eq 255 ]; then
		retcode=-1
	fi
	echo "Will write $retcode"
	echo $retcode > /tmp/${interface}Status
	sleep 1
done

# TODO BDY: replace eth1 per eth0
exit 0
