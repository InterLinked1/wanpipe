#!/bin/sh

home=$(pwd)

if [ ! -d patches ]; then
	echo "This program must be run from wanpipe/ directory"
	exit 0
fi
	
cd patches/kdrivers/src/net
if [ ! -e sdladrv_src.c ]; then
  	ln -s sdladrv.c sdladrv_src.c
fi

cd $home

cd patches/kdrivers/src/wanrouter
if [ ! -e af_wanpipe_src.c ]; then
        ln -s af_wanpipe.c af_wanpipe_src.c
fi

cd $home

cd patches/kdrivers/src/wan_aften
if [ ! -e wan_aften_src.c ]; then
	ln -s wan_aften.c wan_aften_src.c
fi

cd $home

cd patches/kdrivers/include
if [ ! -e linux ]; then
	ln -s . linux
fi

cd $home

echo "Done"

exit 0
