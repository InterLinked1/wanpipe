#!/bin/sh

cmd=$1

cli=/usr/local/ss7box/sangoma_isup_cli

if [ "$cmd" = "inuse" ]; then
	$cli --ckt-report --span all --chan all | grep -c "Y   Y"
elif [ "$cmd" = "free" ]; then
	$cli --ckt-report --span all --chan all | grep -c "Y   n"
else
	$cli --ckt-report --span all --chan all
fi

