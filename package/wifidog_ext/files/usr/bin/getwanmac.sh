rm -f /etc/gwid.conf
ifconfig eth1.2  | sed -n '/HWaddr/ s/^.*HWaddr //pg' | sed 's/://g' | awk -F" " '{print $1}'  >> /etc/gwid.conf

