rm -f /tmp/wanipaddr
wan_mode=`uci get network.wan.proto`
if [ "$wan_mode" = "pppoe" ]; then
	ifconfig ppp0  | sed -n '/inet addr:/ s/inet addr://pg' | awk -F" " '{print $1}'  > /tmp/wanipaddr
else
	ifconfig eth1.2  | sed -n '/inet addr:/ s/inet addr://pg' | awk -F" " '{print $1}'  > /tmp/wanipaddr
fi

