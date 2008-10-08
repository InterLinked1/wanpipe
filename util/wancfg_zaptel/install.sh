#!/bin/sh


WAN_VIRTUAL=$2
WZDIR=${1:-usr/local/sbin/wancfg_zaptel}

if [ -z $WZDIR ]; then
	echo "Directory not found $WZDIR"  	
	exit 1
fi

if [ -e $WAN_VIRTUAL/$WZDIR ]; then 
	rm -rf $WAN_VIRTUAL/$WZDIR
fi 
	
mkdir -p $WAN_VIRTUAL/$WZDIR 
cp -rf . $WAN_VIRTUAL/$WZDIR

install -D -m 755 setup-sangoma $WAN_VIRTUAL/usr/local/sbin/setup-sangoma
install -D -m 755 wancfg_zaptel $WAN_VIRTUAL/usr/sbin/wancfg_zaptel
install -D -m 755 wancfg_dahdi $WAN_VIRTUAL/usr/sbin/wancfg_dahdi
install -D -m 755 wancfg_smg $WAN_VIRTUAL/usr/sbin/wancfg_smg
install -D -m 755 wancfg_tdmapi $WAN_VIRTUAL/usr/sbin/wancfg_tdmapi
install -D -m 755 wancfg_hp_tdmapi $WAN_VIRTUAL/usr/sbin/wancfg_hp_tdmapi


#cp -rf setup-sangoma $WAN_VIRTUAL/usr/local/sbin
#chmod 755 $WAN_VIRTUAL/usr/local/sbin/setup-sangoma


#cp -rf wancfg_zaptel $WAN_VIRTUAL/usr/sbin
#chmod 755 $WAN_VIRTUAL/usr/sbin/wancfg_zaptel

#cp -rf wancfg_smg $WAN_VIRTUAL/usr/sbin
#chmod 755 $WAN_VIRTUAL/usr/sbin/wancfg_smg

#cp -rf wancfg_tdmapi $WAN_VIRTUAL/usr/sbin
#chmod 755 $WAN_VIRTUAL/usr/sbin/wancfg_tdmapi
