--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_dhcp.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local ds = require "luci.dispatcher"

local has_v2 = nixio.fs.access("/lib/network/dhcp.sh")

require("luci.tools.webadmin")
m = Map("dhcp", nil)

nw.init(m.uci)

--
-- Rules
--


s = m:section(TypedSection, "dhcp", translate("DHCP List"))
s.addremove = false
s.anonymous = true
s.template = "cbi/tblsection_d"
--s.extedit   = ds.build_url("admin", "network", "dhcp", "rule", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	local created = TypedSection.create(self, section)
	m.uci:save("network")
	luci.http.redirect(ds.build_url(
		"admin", "network", "dhcp", "rule", created
	))
	return
end

networkid = s:option(Value1, "networkid", translate("Network type"))
networkid.size = 15
function networkid.cfgvalue(self, s)
	local f = self.map:get(s, "networkid")
	if f == "n1" then
		return "STB"
	elseif f == "n2" then
		return "Phone"
	elseif f == "n3" then
		return "Camera"
	elseif f == "n4" then
		return "computer"
	end
end

start_ip = s:option(Value, "start", translate("Start IP"))
start_ip.datatype = "ipaddr"
start_ip.size = 15
function start_ip.cfgvalue(self, s)
	return self.map:get(s, "start") or "-"
end

end_ip = s:option(Value, "end", translate("End IP"))
end_ip.datatype = "ipaddr"
end_ip.size = 15
function end_ip.cfgvalue(self, s)
	return self.map:get(s, "end") or "-"
end
--[[
netmask = s:option(Value, "netmask", translate("Netmask"))
function netmask.cfgvalue(self, s)
	return self.map:get(s, "netmask") or "-"
end
]]--
leasetime = s:option(Value, "leasetime", translate("Leasetime (hours)"))
leasetime.size = 15
leasetime.datatype = "leaset"
function leasetime.cfgvalue(self, s)
	return self.map:get(s, "leasetime") or "12h"
end

return m
