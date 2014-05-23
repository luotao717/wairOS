--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_mac.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ds = require "luci.dispatcher"

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")

require("luci.tools.webadmin")
m = Map("firewall", nil)
m.pagemacaction = true
m.redirect = ds.build_url("admin", "security", "macf")

fw.init(m.uci)
nw.init(m.uci)

--
-- Rules
--

s1 = m:section(TypedSection, "MAC", translate("White/Black Name"))
s1.anonymous = true
s1.addremove = false

state =s1:option(Flag4, "state", translate("Enable MAC Filter"))
state.rmempty = false

flag = s1:option(ListValue4, "policy", translate("Mode"))
flag:value("permit", translate("white list"))
flag:value("deny", translate("black list"))
--flag:depends("state",1)

s = m:section(TypedSection, "mac_permit", translate("MAC Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_macw"
--s.extedit   = ds.build_url("admin", "security", "urlf", "third1", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "macf", "rule1", created
	))
	return
end

mac = s:option(DummyValue, "src_mac", translate("MAC Address"))
function mac.cfgvalue(self, s)
	return self.map:get(s, "src_mac") 
end

name = s:option(DummyValue, "name", translate("Description"))
function name.cfgvalue(self,s)
     return self.map:get(s,"name") 
end

s = m:section(TypedSection, "mac_deny", translate("MAC Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_macb"
--s.extedit   = ds.build_url("admin", "security", "urlf", "third2", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "macf", "rule2", created
	))
	return
end

mac = s:option(DummyValue, "src_mac", translate("MAC Address"))
function mac.cfgvalue(self, s)
	return self.map:get(s, "src_mac") 
end

name = s:option(DummyValue, "name", translate("Description"))
function name.cfgvalue(self,s)
     return self.map:get(s,"name") 
end

return m
