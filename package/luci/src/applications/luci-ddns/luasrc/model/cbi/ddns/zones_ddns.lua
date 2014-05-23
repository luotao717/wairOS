--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_ddns.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local ds = require "luci.dispatcher"

local wanlnk= require "luci.model.wanlink".init()

local has_v2 = nixio.fs.access("/lib/services/ddns.sh")

require("luci.tools.webadmin")

m = Map("ddns", nil)
m.redirect = ds.build_url("admin", "services", "ddns")
nw.init(m.uci)

--
-- Rules
--

s = m:section(TypedSection, "service", translate("DDNS List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
--s.extedit   = ds.build_url("admin", "services", "ddns", "third", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("services")
	luci.http.redirect(ds.build_url(
		"admin", "services", "ddns", "rule", created
	))
	return
end

penabled = s:option(DummyValue, "enabled", translate("Enabled"))
function penabled.cfgvalue(self, s)
	local enable = self.map:get(s, "enabled") 
	return (enable == "1") and "Enable" or "Disable"
end
providers = s:option(DummyValue, "service_name", translate("Service"))
function providers.cfgvalue(self, s)
	return self.map:get(s, "service_name") 
end
--[[
serport = s:option(DummyValue, "service_port", translate("Service Port"))
function serport.cfgvalue(self, s)
	return self.map:get(s, "service_port") 
end
]]--
hostn = s:option(DummyValue, "hostname", translate("Host Name"))
function hostn.cfgvalue(self, s)
	return self.map:get(s, "hostname") 
end

domain = s:option(DummyValue, "domain", translate("Domain"))
function domain.cfgvalue(self, s)
	return self.map:get(s, "domain") 
end

account = s:option(DummyValue, "username", translate("Account"))
function account.cfgvalue(self, s)
	return self.map:get(s, "username") 
end


password = s:option(DummyValue, "password", translate("Password"))
function password.cfgvalue(self, s)
	return self.map:get(s, "password") 
end


interface = s:option(DummyValue, "iface", translate("Interface"))
function interface.cfgvalue(self, s)
	local f = self.map:get(s, "iface")
	for o,v in pairs(wanlnk.waninfo_get()) do
		if f == v.Interface  then
			return o
		end
	end	
end

return m
