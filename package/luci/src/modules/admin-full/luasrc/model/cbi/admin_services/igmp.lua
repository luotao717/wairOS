--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: igmp.lua 6065 2010-04-14 11:36:13Z ben $
]]--

local wanlnk= require "luci.model.wanlink".init()

m = Map("igmp", translate("IGMP Config"), "")
--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("Igmp", "snooping", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("Igmp", "snooping", nil, { 
			["snooping_en"]     = "0"})
		m.uci:save("Igmp")
	end

      has_section = false
	m.uci:foreach("Igmp", "proxy", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("Igmp", "proxy", nil, { 
			["proxy_en"]     = "0"})
		m.uci:save("Igmp")
	end
end
--]]
s = m:section(TypedSection, "IPTV", translate("IGMP Snooping"))
s.anonymous = true
s.addremove = false
local igmpsnp= s:option(ListValue, "SnoopingEnable", translate("Enable IGMP Snooping"))
igmpsnp:value("false",translate("Disable"))
igmpsnp:value("true",translate("Enable"))

s2 = m:section(TypedSection, "IPTV", translate("IGMP Proxy"))
s2.anonymous = true
s2.addremove = false

igmppry = s2:option(ListValue, "ProxyEnable", translate("Enable IGMP Proxy"))
igmppry:value("false",translate("Disable"))
igmppry:value("true",translate("Enable"))
--[[
interface = s2:option(MultiValue, "interface", translate("Interface"))
for k,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end
--interface:value("0", translate("1_INTERNET_R_VID_2"))
--interface:value("1", translate("2_TR069_R_VID_3"))

interface.rmempty = true
interface:depends("ProxyEnable","true")
]]--
return m
