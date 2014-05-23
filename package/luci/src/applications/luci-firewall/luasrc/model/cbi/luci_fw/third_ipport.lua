--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_ipport.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
arg[1] = arg[1] or ""

m = Map("firewall", nil)
--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("firewall", "filter", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("firewall", "filter", nil, nil)
		m.uci:save("firewall")
	end
end
]]--
m.redirect = dsp.build_url("admin", "security", "ipportf")

if not m.uci:get(arg[1]) == "ipport" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "filter", translate("IP/Port Filter"))
s.anonymous = true
s.addremove = false


nam=s:option(Value, "name", translate("Name"))
nam.rmempty = true
nam.default="0000"
nam.datatype = "hostname"


sip = s:option(Value, "src_ip", translate("Remote IP"))
sip.rmempty = false
sip.maxlength = 15
sip.datatype = "ipaddr"
sip.default="192.168.1.1"

sport = s:option(Value, "src_port", translate("External  Port"))
sport.rmempty = false
sport.size = 5
sport.maxlength = 5
sport.datatype = "port"
sport.default="65535"

dip = s:option(Value, "dest_ip", translate("Dest IP"))
dip.rmempty = false
dip.maxlength = 15
dip.datatype = "ipaddr"
dip.default="192.168.1.254"

dport = s:option(Value, "dest_port", translate("Dest Port"))
dport.rmempty = false
dport.size = 5
dport.maxlength = 5
dport.datatype = "port"
dport.default="65535"

function dport.validate(self, value,section)	
	local valid = true
	local old_value = self.map.uci:get("firewall",section,"dest_port")
	
		
	if old_value == value then
		valid = true
	else
		self.map.uci:foreach("firewall", "filter", function(section)
			if section.dport == value    then
				valid = false
			end
		end)
	end

	if valid then
		return value
	else
		return nil,translate("Dest Port required")
	end
end

protocol = s:option(ListValue, "proto", translate("Protocol"))
protocol:value("all", translate("ALL"))
protocol:value("tcp", "TCP")
protocol:value("udp", "UDP")
protocol.default="all"

interface = s:option(ListValue, "iface", translate("Interface"))
interface.rmempty = false
interface.maxlength = 15
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end
return m
