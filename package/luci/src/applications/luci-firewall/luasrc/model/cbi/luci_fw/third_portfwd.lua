--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_portfwd.lua 6983 2011-04-13 00:33:42Z soma $
]]--

--local has_v2 = nixio.fs.access("/lib/services/portmir.sh")
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()

arg[1] = arg[1] or ""

m = Map("firewall", nil)
--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("firewall", "redirect", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("firewall", "redirect", nil, 
		{
		  ["protocol"]  = "0",
 		  ["interface"] = "0",
		  ["status"]  = "0"
		})
		m.uci:save("firewall")
	end
end
]]--
m.redirect = dsp.build_url("admin", "services", "portfwd")

if not m.uci:get(arg[1]) == "rule" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "redirect", translate("Port Forwarding"))
s.anonymous = true
s.addremove = false

internalip = s:option(Value, "dest_ip", translate("Internal IP"))
internalip.rmempty = false
internalip.maxlength = 15
internalip.datatype = "ipaddr"
internalip.default="0.0.0.0"

privatePort = s:option(Value, "dest_port", translate("Internal Port"))
privatePort.rmempty = false
privatePort.size = 5
privatePort.maxlength = 5
privatePort.datatype = "port"
privatePort.default="65535"

protocol = s:option(ListValue, "proto", translate("Protocol"))
protocol:value("all", translate("ALL"))
protocol:value("tcp", "TCP")
protocol:value("udp", "UDP")

externalip = s:option(Value, "src_ip", translate("External IP"))
externalip.rmempty = false
externalip.maxlength = 15
externalip.datatype = "ipaddr"
externalip.default="192.168.1.1"

publicPort = s:option(Value, "src_port", translate("External Port"))
publicPort.rmempty = false
publicPort.size = 5
publicPort.maxlength = 5
publicPort.datatype = "port"
publicPort.default="65535"

interface = s:option(ListValue, "iface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end
--interface:value("0", translate("1_INTERNET_R_VID_2"))
--interface:value("1", translate("2_TR069_R_VID_3"))

status = s:option(ListValue, "state", translate("Status"))
status:value("0", translate("Disable"))
status:value("1", translate("Enable"))
status.default="1"
return m
