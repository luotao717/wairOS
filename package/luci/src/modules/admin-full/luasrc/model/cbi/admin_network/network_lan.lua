--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: network.lua 5948 2010-03-27 14:54:06Z jow $
]]--

require("luci.tools.webadmin")

m = Map("network", translate("Network"))

s = m:section(NamedSection, "lan", "interface", translate("Local Network"))
s.addremove = false
s:option(Value, "ipaddr", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Address"))

nm = s:option(Value, "netmask", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Netmask"))
nm:value("255.255.255.0")
nm:value("255.255.0.0")
nm:value("255.0.0.0")
--[[
gw = s:option(Value, "gateway", translate("<abbr title=\"Internet Protocol Version 4\">IPv4</abbr>-Gateway") .. translate(" (optional)"))
gw.rmempty = true
dns = s:option(Value, "dns", translate("<abbr title=\"Domain Name System\">DNS</abbr>-Server") .. translate(" (optional)"))
dns.rmempty = true
--]]

return m
