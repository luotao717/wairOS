--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: portbind.lua 6065 2010-04-14 11:36:13Z ben $
]]--
m = Map("network", translate("Port Bind"), "")

function m.on_parse()
	local has_section = false

	m.uci:foreach("network", "portbind", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("network", "portbind", nil, { 
			["wan"]     = "0"})
		m.uci:save("firewall")
	end
end

s = m:section(TypedSection, "portbind", translate("Port Bind"))
s.anonymous = true
s.addremove = false

s1 = s:option(ListValue, "lan1", translate("LAN 1"))
s1:value("0", translate("1_INTERNET_R_VID_2"))
s1:value("1", translate("2_TR069_R_VID_3"))

s2 = s:option(ListValue, "lan2", translate("LAN 2"))
s2:value("0", translate("1_INTERNET_R_VID_2"))
s2:value("1", translate("2_TR069_R_VID_3"))

s3 = s:option(ListValue, "lan3", translate("LAN 3"))
s3:value("0", translate("1_INTERNET_R_VID_2"))
s3:value("1", translate("2_TR069_R_VID_3"))

s4 = s:option(ListValue, "lan4", translate("LAN 4"))
s4:value("0", translate("1_INTERNET_R_VID_2"))
s4:value("1", translate("2_TR069_R_VID_3"))

s5 = s:option(ListValue, "wan", translate("WAN"))
s5:value("0", translate("1_INTERNET_R_VID_2"))
s5:value("1", translate("2_TR069_R_VID_3"))

return m
