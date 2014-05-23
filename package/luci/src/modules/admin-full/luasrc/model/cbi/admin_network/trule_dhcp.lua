--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_dhcp.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/network/dhcp.sh")
local dsp = require "luci.dispatcher"
arg[1] = arg[1] or ""

m = Map("dhcp", nil)

m.redirect = dsp.build_url("admin", "network", "dhcp")

if not m.uci:get(arg[1]) == "rule" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "rule", translate("DHCP Settings"))
s.anonymous = true
s.addremove = false

device_type = s:option(ListValue, "device_type", translate("Device Type"))
device_type:value("0", translate("Computer"))
device_type:value("1", translate("Voip"))
device_type:value("2", translate("STB"))
device_type:value("3", translate("Other"))

sip = s:option(Value, "start_ip", translate("Start IP"))
sip.rmempty = false
sip.maxlength = 15
sip.datatype = "ipaddr"

eip = s:option(Value, "end_ip", translate("End IP"))
eip.rmempty = false
eip.maxlength = 15
eip.datatype = "ipaddr"

return m
