echo GatewayID `cat /etc/gwid.conf` >> /usr/local/etc/wifidog.conf
echo ExternalInterface eth0 >> /usr/local/etc/wifidog.conf
echo GatewayInterface br-lan >> /usr/local/etc/wifidog.conf
echo GatewayAddress `cat /tmp/ipaddr.log` >> /usr/local/etc/wifidog.conf
echo 'AuthServer {' >> /usr/local/etc/wifidog.conf
echo Hostname `cat /etc/config/authserver.conf`  >> /usr/local/etc/wifidog.conf
echo 'SSLAvailable no
SSLPort 443
HTTPPort 80
Path /
LoginScriptPathFragment ?
PortalScriptPathFragment hao.aspx?
#PingScriptPathFragment  ping.action?
AuthScriptPathFragment  auth.aspx?
}' >> /usr/local/etc/wifidog.conf
echo GatewayPort 2060 >> /usr/local/etc/wifidog.conf
echo HTTPDMaxConn 100 >> /usr/local/etc/wifidog.conf
echo HTTPDName WiFiDog >> /usr/local/etc/wifidog.conf
echo CheckInterval 3600 >> /usr/local/etc/wifidog.conf
echo ClientTimeout 1 >> /usr/local/etc/wifidog.conf
echo TrustedMACList `cat /etc/userpage.conf` >> /usr/local/etc/wifidog.conf
echo 'FirewallRuleSet global {' >> /usr/local/etc/wifidog.conf
cat /etc/allow.conf >> /usr/local/etc/wifidog.conf
echo ' ' >> /usr/local/etc/wifidog.conf
echo } >> /usr/local/etc/wifidog.conf

echo 'FirewallRuleSet validating-users {
FirewallRule allow to 0.0.0.0/0
}
FirewallRuleSet known-users {
FirewallRule allow to 0.0.0.0/0
}
FirewallRuleSet unknown-users {
FirewallRule allow udp port 53
FirewallRule allow tcp port 53
FirewallRule allow udp port 67
FirewallRule allow tcp port 67
}
FirewallRuleSet locked-users {
FirewallRule block to 0.0.0.0/0
}' >> /usr/local/etc/wifidog.conf
