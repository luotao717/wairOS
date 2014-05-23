--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_router.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local fw = require "luci.model.firewall"
local nw = require "luci.model.network"
local ds = require "luci.dispatcher"

local wanlnk= require "luci.model.wanlink".init()

--local has_v2 = nixio.fs.access("/lib/network/router.sh")

require("luci.tools.webadmin")
m = Map("routes", nil)

fw.init(m.uci)
nw.init(m.uci)
m.redirect = ds.build_url("admin", "network", "router")
--
-- Rules
--

s = m:section(TypedSection, "route", translate("Router List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
--s.extedit   = ds.build_url("admin", "network", "router", "third", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("routes")
	luci.http.redirect(ds.build_url(
		"admin", "network", "router", "rule", created
	))
	return
end

dip = s:option(DummyValue, "target", translate("Dest IP"))
function dip.cfgvalue(self, s)
	return self.map:get(s, "target")
end

mask = s:option(DummyValue, "netmask", translate("Subnet Mask"))
function mask.cfgvalue(self, s)
	return self.map:get(s, "netmask") 
end

gw = s:option(DummyValue, "gateway", translate("Gateway"))
function gw.cfgvalue(self, s)
	return self.map:get(s, "gateway") 
end

interface = s:option(DummyValue, "interface", translate("Interface"))
function interface.cfgvalue(self, s)
	local f = self.map:get(s, "interface")

	for o,v in pairs(wanlnk.waninfo_get()) do
		if f == v.Interface  then
			return o
		end
	end		
end

return m
