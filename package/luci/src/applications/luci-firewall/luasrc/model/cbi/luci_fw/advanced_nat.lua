--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: advanced_nat.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
m = Map("firewall", translate("Advance NAT Settings"), "")
m.redirect = dsp.build_url("admin", "services", "advanced_nat")
--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("firewall", "alg", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("firewall", "alg", nil, { 
			["H323En"]     = "0",
			["SIPEn"]     = "0",
			["RTSPEn"]     = "0",
			["L2TPEn"]     = "0",
			["IPSecEn"]     = "0",
			["FTPEn"]     = "0"})
		m.uci:save("firewall")
	end

	has_section = false
	m.uci:foreach("firewall", "dmz", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("firewall", "dmz", nil, { 
			["dmz"]     = "0"})
		m.uci:save("firewall")
	end
end
]]--
s = m:section(TypedSection, "algs", translate("ALG Settings"))
s.anonymous = true
s.addremove = false

H323=s:option(Flag, "H323En", translate("Enable H.323"))
H323.rmempty = false
RTP=s:option(Flag, "RTSPEn", translate("Enable RTSP"))
RTP.rmempty = false
L2TP=s:option(Flag, "L2TPEn", translate("Enable L2TP"))
L2TP.rmempty = false
IPS=s:option(Flag, "IPSecEn", translate("Enable IPSec"))
IPS.rmempty = false
RTP=s:option(Flag, "FTPEn", translate("Enable FTP"))
RTP.rmempty = false

s2 = m:section(TypedSection, "dmz", translate("DMZ Settings"))
s2.anonymous = true
s2.addremove = false

dmz=s2:option(Flag, "state", translate("Enable DMZ"))
dmz.rmempty = false

ip = s2:option(Value, "dest_ip", translate("DMZ IP Address"))

ip.maxlength = 15
ip.datatype = "ipaddr"

ip:depends("state","1")


iface = s2:option(ListValue, "iface", translate("Interface"))
iface.maxlength = 15
iface:depends("state","1")
for o,v in pairs(wanlnk.waninfo_get()) do
	iface:value(v.Interface,translate(v.ConnName))
end
iface:value("eth0",translate("eth0"))
function iface.validate(self, value, section)
		local valid = true
		local ipvalue = ip:formvalue(section)
		
		if ipvalue ~="" then
		else
			valid = false
		end	
		if valid then
			return value
		else
			return nil,  translate("DMZ IP Address is empty")
		end

end

return m
