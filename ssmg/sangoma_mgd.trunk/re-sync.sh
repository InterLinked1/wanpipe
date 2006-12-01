#!/bin/sh

home=`pwd`;

ss7boost="ss7boost"
ss7box="ss7boxd"

#NICE="nice -n -5 "
NICE=
RENICE="renice -15 -p "

if [ ! -e /usr/local/ss7box/$ss7boost ]; then
	echo "Error: ss7boost not found in /usr/local/ss7box dir";
	exit 1
fi
if [ ! -e /usr/local/ss7box/$ss7boxd ]; then
	echo "Error: ss7boxd not found in /usr/local/ss7box dir";
	exit 1
fi

sangoma_mgd -term

kill -TERM $(pidof asterisk);


make clean
make
if [ $? -ne 0 ]; then
	exit 1;
fi
make install

if [ "$1" = "all" ]; then
	cp -f g711.h /usr/src/asterisk
	perl /usr/src/asterisk/contrib/scripts/astxs -install chan_woomera.c
	if [ $? -ne 0 ]; then
		exit 1;
	fi
fi 

cd /usr/local/ss7box
./$ss7boost --term
sleep 1
eval "$NICE ./$ss7boost" 
sleep 5

if [ "$RENICE" != "" ]; then
	echo "Running RNICE on ss7boost"
	eval "$RENICE $(pidof $ss7boost)"
	echo "Running RNICE on ss7box"
	eval "$RENICE $(pidof $ss7box)"
fi

cd $home


./clog.sh


if [ 1 -eq 0 ]; then
asterisk -c
else
eval "$NICE asterisk -g" 
#Enable this to start asterisk in priority mode
#in this case comment out the one above.
#eval "$NICE asterisk -g -p"
sleep 1
asterisk -c -r
fi
