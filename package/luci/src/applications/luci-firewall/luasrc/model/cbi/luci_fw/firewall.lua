--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: firewall.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local ds = require "luci.dispatcher"

m = Map("firewall", translate("Firewall"), translate("Firewall"))

m.redirect = ds.build_url("admin", "security", "firewall")
--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("firewall", "default", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("firewall", "default", nil, { 
			["sys_flood"]     = "0"})
		m.uci:save("firewall")
	end
end
]]--
s = m:section(TypedSection, "ddos", translate("Firewall Settings"))
s.anonymous = true
s.addremove = false



pscan=s:option(Flag, "port_scan", translate("Port Scan Protection"))
pscan.rmempty = false
ping=s:option(Flag, "ping_of_death", translate("Ping of Death"))
ping.rmempty = false
synf=s:option(Flag, "synflood_protect", translate("SYN Flood"))
synf.rmempty = false
win=s:option(Flag, "winnuke", translate("Winnuke"))
win.rmempty = false
smurf=s:option(Flag, "smurf", translate("Smurf"))
smurf.rmempty = false
icmp=s:option(Flag, "icmp_redirect", translate("ICMP Redirection"))
icmp.rmempty = false
return m
