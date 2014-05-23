--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_ipport.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ds = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")

require("luci.tools.webadmin")
m = Map("firewall", nil)
m.pageipportaction = true
m.redirect = ds.build_url("admin", "security", "ipportf")
fw.init(m.uci)
nw.init(m.uci)

--
-- Rules
--


s1 = m:section(TypedSection, "IPPORT", translate("White/Black Name"))
s1.anonymous = true
s1.addremove = false

state =s1:option(Flag5, "state", translate("Enable IP/Port Filter"))
state.rmempty = false

flag = s1:option(ListValue5, "policy", translate("Mode"))
flag:value("permit", translate("white list"))
flag:value("deny", translate("black list"))
--flag:depends("state",1)

s = m:section(TypedSection, "ipport_permit", translate("IP/Port Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_ipw"
--s.extedit   = ds.build_url("admin", "security", "ipportf", "third", "%s")
--s.defaults.target = "ACCEPT"
function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "ipportf", "rule1", created
	))
	return
end

comment = s:option(DummyValue, "name", translate("Name"))
function comment.cfgvalue(self, s)
	return self.map:get(s, "name") 
end

sip = s:option(DummyValue, "src_ip", translate("Source IP"))
function sip.cfgvalue(self, s)
	return self.map:get(s, "src_ip") 
end
--[[
sip = s:option(DummyValue, "src_ipend", translate("Source IP"))
function sip.cfgvalue(self, s)
	return self.map:get(s, "src_ip") 
end
]]--
sport = s:option(DummyValue, "src_port", translate("Source Port"))
function sport.cfgvalue(self, s)
	return self.map:get(s, "src_port") 
end

dip = s:option(DummyValue, "dest_ip", translate("Dest IP"))
function dip.cfgvalue(self, s)
	return self.map:get(s, "dest_ip") 
end
--[[
dip = s:option(DummyValue, "dest_ipend", translate("Dest IP End"))
function dip.cfgvalue(self, s)
	return self.map:get(s, "dest_ip") 
end
]]--

dport = s:option(DummyValue, "dest_port", translate("Dest Port"))
function dport.cfgvalue(self, s)
	return self.map:get(s, "dest_port") 
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

link = s:option(DummyValue, "dir", translate("Link"))
function link.cfgvalue(self, s)
	local f = self.map:get(s, "dir")
	if f == "uplink" then
		return "Uplink"
	elseif f == "downlink" then
		return "Downlink"
	end
end
--[[
interface = s:option(DummyValue, "iface", translate("Interface"))
function interface.cfgvalue(self, s)
	local f = self.map:get(s, "iface")
	for o,v in pairs(wanlnk.waninfo_get()) do
		if f == v.Interface  then
			return o
		end
	end		
end
]]--
s = m:section(TypedSection, "ipport_deny", translate("IP/Port Filter List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_ipb"
--s.extedit   = ds.build_url("admin", "security", "ipportf", "third", "%s")
--s.defaults.target = "ACCEPT"
function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("firewall")
	luci.http.redirect(ds.build_url(
		"admin", "security", "ipportf", "rule2", created
	))
	return
end

comment = s:option(DummyValue, "name", translate("Name"))
function comment.cfgvalue(self, s)
	return self.map:get(s, "name") 
end

sip = s:option(DummyValue, "src_ip", translate("Source IP"))
function sip.cfgvalue(self, s)
	return self.map:get(s, "src_ip") 
end

sport = s:option(DummyValue, "src_port", translate("Source Port"))
function sport.cfgvalue(self, s)
	return self.map:get(s, "src_port") 
end

dip = s:option(DummyValue, "dest_ip", translate("Dest IP"))
function dip.cfgvalue(self, s)
	return self.map:get(s, "dest_ip") 
end

dport = s:option(DummyValue, "dest_port", translate("Dest Port"))
function dport.cfgvalue(self, s)
	return self.map:get(s, "dest_port") 
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

link = s:option(DummyValue, "dir", translate("Link"))
function link.cfgvalue(self, s)
	local f = self.map:get(s, "dir")
	if f == "uplink" then
		return "Uplink"
	elseif f == "downlink" then
		return "Downlink"
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


return m
