rm -f /etc/newwanmac.conf
ifconfig eth1.2 | grep eth0 | awk '{print $5}'  >> /etc/newwanmac.conf

