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

local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
m = SimpleForm("firewall", translate("IP/Port Filter"))

m.redirect = dsp.build_url("admin", "security", "ipportf")

nam = m:field(Value, "name", translate("Name"))
nam.rmempty = false
nam.default = "0000"
nam.datatype = "hostname"

sip = m:field(Value, "src_ip", translate("Source IP"))
sip.rmempty = false
sip.maxlength = 15
sip.datatype = "ipaddr"
sip.default = "192.168.1.1"

--[[
function sip.validate(self, value,section)
	local valid = true
	uci:foreach("firewall", "filter", function(section)
		if section.src_ip == value    then
			valid = false
			m.message = translate("Remote IP Address required")
		end
	end)

	if valid then
		return value
	else
		return nil,translate("Remote IP Address required")
	end
end
]]--

sport = m:field(Value, "src_port", translate("Source Port"))
sport.rmempty = false
sport.size = 5
sport.maxlength = 5
sport.datatype = "port"
sport.default = "65535"

dip = m:field(Value, "dest_ip", translate("Dest IP"))
dip.rmempty = false
dip.maxlength = 15
dip.datatype = "ipaddr"
dip.default = "192.168.1.254"

--[[
function dip.validate(self, value,section)
	local valid = true
	uci:foreach("firewall", "ipport", function(section)
		if section.dest_ip == value    then
			valid = false
			m.message = translate("Dest IP Address required")
		end
	end)

	if valid then
		return value
	else
		return nil,translate("Dest IP Address required")
	end
end
]]--

dport = m:field(Value, "dest_port", translate("Dest Port"))
dport.rmempty = false
dport.size = 5
dport.maxlength = 5
dport.datatype = "port"
dport.default = "65535"

protocol = m:field(ListValue, "proto", translate("Protocol"))
protocol:value("all", translate("TCP/UDP"))
protocol:value("tcp", "TCP")
protocol:value("udp", "UDP")
protocol.default = "all"

interface = m:field(ListValue, "iface", translate("Interface"))
interface.rmempty = false
interface.maxlength = 15
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end

--save as
function m.handle(self, state, data)
	if state == FORM_VALID then
		local new_ipport = uci:section("firewall", "filter")
  		uci:set("firewall", new_ipport, "name", data.name)
		uci:set("firewall", new_ipport, "src_ip", data.src_ip)
		uci:set("firewall", new_ipport, "src_port", data.src_port)
		uci:set("firewall", new_ipport, "dest_ip", data.dest_ip)
		uci:set("firewall", new_ipport, "dest_port", data.dest_port)
  		uci:set("firewall", new_ipport, "proto", data.proto)
		uci:set("firewall", new_ipport, "iface", data.iface)
  		uci:save("firewall")
		luci.http.redirect(luci.dispatcher.build_url("admin/security/ipportf"))
	end
end

return m
