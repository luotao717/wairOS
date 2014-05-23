--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_mac.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")
local dsp = require "luci.dispatcher"

arg[1] = arg[1] or ""

m = Map("firewall", nil)

m.redirect = dsp.build_url("customer", "security", "macf")

if not m.uci:get(arg[1]) == "macfilter" then
	luci.http.redirect(m.redirect)
	return
end
s = m:section(NamedSection, arg[1], "macfilter", translate("MAC Filter"))
s.anonymous = true
s.addremove = false

mac = s:option(Value, "src_mac", translate("MAC Address"))
mac.rmempty = true
mac.maxlength = 17
mac.datatype = "macaddr"
mac.default ="00:00:00:00:00:01"

function mac.validate(self, value,section)
	
	
		local valid = true
		local old_value = self.map.uci:get("firewall",section,"src_mac")
	
		
		if old_value == value then
			valid = true
		else
			self.map.uci:foreach("firewall", "macfilter", function(section)
				 if section.src_mac == value    then
					valid = false
				end
			end)
		end

		if valid then
			return value
		else
			return nil,translate("MAC Address required")
		end

end

name = s:option(Value, "name", translate("Description"))
name.rmempty = true
name.size = 15
name.maxlength = 15
--[[
protocol = s:option(ListValue, "protocol", translate("Protocol"))
protocol:value("all", translate("ALL"))
protocol:value("arp", translate("ARP"))
protocol:value("ip", translate("IP"))

frameflow = s:option(ListValue, "dir", translate("Frame Flow"))
frameflow:value("lan->wan", translate("LAN -> WAN"))
frameflow:value("wan->lan", translate("WAN -> LAN"))
]]--
return m
