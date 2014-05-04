#!/bin/sh

lock /tmp/run/.wpslock
# only registrar mode
iwpriv ra0 set WscConfMode=4
# 1:un-configured, 2:configured-AP
iwpriv ra0 set WscConfStatus=2
case "$1" in
	pbc)
		iwpriv ra0 set WscMode=2
	;;
	pin)
		iwpriv ra0 set WscMode=1
		iwpriv ra0 set WscPinCode=$2
	;;
esac
iwpriv ra0 set WscGetConf=1
iwpriv ra0 set WscStatus=0

lock -u /tmp/run/.wpslock
