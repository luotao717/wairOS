--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_url.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ds = require "luci.dispatcher"

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")

require("luci.tools.webadmin")
m = Map("firewall", nil)
m.pageurlaction = true
m.redirect = ds.build_url("admin", "security", "urlf")

fw.init(m.uci)
nw.init(m.uci)

--
-- Rules
--


s1 = m:section(TypedSection, "URL", translate("White/Black Name"))
s1.anonymous = true
s1.addremove = false

state =s1:option(Flag3, "state", translate("Enable URL Filter"))
state.rmempty = false

flag = s1:option(ListValue3, "policy", translate("Mode"))
flag:value("permit", translate("white list"))
flag:value("deny", translate("black list"))
--flag:depends("state",1)

--/////////////////////////////////////////////white

s = m:section(TypedSection, "url_permit", translate("Url Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_white"
--s.extedit   = ds.build_url("admin", "security", "urlf", "third1", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "urlf", "rule1", created
	))
	return
end

url = s:option(DummyValue, "url", translate("Url Address"))
function url.cfgvalue(self, s)
	return self.map:get(s, "url")
end

host = s:option(DummyValue, "host", translate("Host"))
function host.cfgvalue(self,s)
     return self.map:get(s,"host")
end

name = s:option(DummyValue, "name", translate("Description"))
function name.cfgvalue(self,s)
     return self.map:get(s,"name")
end
--/////////////////////////////////////////////black

s = m:section(TypedSection, "url_deny", translate("Url Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_black"
--s.extedit   = ds.build_url("admin", "security", "urlf", "third2", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "urlf", "rule2", created
	))
	return
end

url = s:option(DummyValue, "url", translate("Url Address"))
function url.cfgvalue(self, s)
	return self.map:get(s, "url")
end

host = s:option(DummyValue, "host", translate("Host"))
function host.cfgvalue(self,s)
     return self.map:get(s,"host")
end

name = s:option(DummyValue, "name", translate("Description"))
function name.cfgvalue(self,s)
     return self.map:get(s,"name")
end

return m
