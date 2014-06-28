echo GatewayID `cat /etc/gwid.conf` >> /etc/wifidog.conf
echo ExternalInterface eth0 >> /etc/wifidog.conf
echo GatewayInterface br-lan >> /etc/wifidog.conf
echo GatewayAddress `cat /tmp/ipaddr.log` >> /etc/wifidog.conf
echo 'AuthServer {' >> /etc/wifidog.conf
echo Hostname `cat /etc/config/authserver.conf`  >> /etc/wifidog.conf
echo 'SSLAvailable no
SSLPort 443
HTTPPort 9001
Path /fcp/router/
LoginScriptPathFragment login?
PortalScriptPathFragment tosuccess?
PingScriptPathFragment  ping?
AuthScriptPathFragment  auth?
}' >> /etc/wifidog.conf
echo GatewayPort 2060 >> /etc/wifidog.conf
echo HTTPDMaxConn 100 >> /etc/wifidog.conf
echo HTTPDName WiFiDog >> /etc/wifidog.conf
echo CheckInterval 3600 >> /etc/wifidog.conf
echo ClientTimeout 1 >> /etc/wifidog.conf
echo TrustedMACList `cat /etc/userpage.conf` >> /etc/wifidog.conf
echo 'FirewallRuleSet global {' >> /etc/wifidog.conf
cat /etc/allow.conf >> /etc/wifidog.conf
echo ' ' >> /etc/wifidog.conf
echo } >> /etc/wifidog.conf

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
}' >> /etc/wifidog.conf
