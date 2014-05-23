--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_portfwd.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local ds = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()

--local has_v2 = nixio.fs.access("/lib/services/portmir.sh")

require("luci.tools.webadmin")
m = Map("firewall", nil)
m.redirect = ds.build_url("admin", "services", "portfwd")
nw.init(m.uci)

--
-- Rules
--

s = m:section(TypedSection, "redirect", translate("Port Forwarding List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
--s.extedit   = ds.build_url("admin", "services", "portfwd", "third", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("services")
	luci.http.redirect(ds.build_url(
		"admin", "services", "portfwd", "rule", created
	))
	return
end
comment = s:option(DummyValue, "name", translate("Name"))
function comment.cfgvalue(self, s)
	return self.map:get(s, "name") 
end
privateIP = s:option(DummyValue, "dest_ip", translate("Internal IP"))
function privateIP.cfgvalue(self, s)
	return self.map:get(s, "dest_ip") 
end

privatePort = s:option(DummyValue, "dest_port", translate("Internal Port"))
function privatePort.cfgvalue(self, s)
	return self.map:get(s, "dest_port") 
end

publicIP = s:option(DummyValue, "src_ip", translate("Remote IP"))
function publicIP.cfgvalue(self, s)
	return self.map:get(s, "src_ip") 
end

publicPort = s:option(DummyValue, "src_dport", translate("External Port"))
function publicPort.cfgvalue(self, s)
	return self.map:get(s, "src_dport") 
end

protocol = s:option(DummyValue, "proto", translate("Protocol"))
function protocol.cfgvalue(self, s)
	local f = self.map:get(s, "proto")
	if f == "TCPUDP" then
		return "TCP/UDP"
	elseif f == "TCP" then
		return "TCP"
	elseif f == "UDP" then
		return "UDP"
	end
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

status = s:option(DummyValue, "state", translate("Status"))
function status.cfgvalue(self, s)
	local f = self.map:get(s, "state")
	if f == "0" then
		return translate("Disable")
	elseif f == "1" then
		return translate("Enable")
	end
end	

return m
