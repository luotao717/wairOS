#!/bin/sh

# wifidog devbind 

WANMAC=`ifconfig eth1.2  | sed -n '/HWaddr/ s/^.*HWaddr //pg' | sed 's/://g' | awk -F" " '{print $1}'`
#echo $WANMAC
#echo $1
#echo $2
#echo $3
if [ "$1" = "bind" ]; then
	httpsend -f fansupBound $WANMAC $2 $3 1
else
	httpsend -f fansupBound $WANMAC $2 $3 0
fi
#echo $0 $@ >> /tmp/wifidog_binddev
