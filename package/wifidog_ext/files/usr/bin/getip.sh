rm /tmp/ipaddr.log
ifconfig br-lan  | sed -n '/inet addr:/ s/inet addr://pg' | awk -F" " '{print $1}'  >> /tmp/ipaddr.log

