#!/bin/bash

cmd=$1;

if [ ! -e wan_aftup ]; then
	eval "make clean > /dev/null"
	eval "make > /dev/null"
fi

if [ -e /proc/net/wanrouter ]; then

	echo
	echo "Warning: Wanpipe modules loaded"
	echo "         Please remove wanpipe modules from the kernel"
	echo
	echo "         eg: wanrouter stop"
	echo
	exit 1
fi


trap '' 2

eval "./scripts/load.sh"

if [ "$cmd" == "auto" ]; then
	eval "./wan_aftup -auto"
else
	eval "./wan_aftup "
fi

eval "./scripts/unload.sh"

if [ -f "/tmp/fw-upd-err.txt" ]; then
	eval 'grep -nr "Failed with" /tmp/fw-upd-err.txt  | grep "manufcaturer ID as FF" >/dev/null 2>/dev/null'
	if [ $? -eq 0 ]; then
		 echo ""
		 echo "****************************************************************************************"
		 echo -e " Please \e[31m\e[4mREBOOT SYSTEM\e[0m and Try Firmware Upgrade Again to ensure the card functionality!! "
		 echo "****************************************************************************************"

		 rm -rf /tmp/fw-upd-err.txt
	fi
fi

