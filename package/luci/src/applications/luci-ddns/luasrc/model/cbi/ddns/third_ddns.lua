--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_ddns.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/services/ddns.sh")
local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"

local wanlnk= require "luci.model.wanlink".init()

arg[1] = arg[1] or ""

m = Map("ddns", nil)

m.redirect = dsp.build_url("admin", "services", "ddns")

if not m.uci:get(arg[1]) == "service" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "service", translate("DDNS Settings"))
s.anonymous = true
s.addremove = false

svc=s:option(ListValue, "service_name", translate("Service"))
svc.rmempty = false 

local services = { }
local fd = io.open("/usr/lib/ddns/services", "r")
if fd then
	local ln
	repeat
		ln = fd:read("*l")
		local s = ln and ln:match('^%s*"([^"]+)"')
		if s then services[#services+1] = s end
	until not ln
	fd:close()
end

local v
for _, v in luci.util.vspairs(services) do
	svc:value(v)
end

domain = s:option(Value, "domain", translate("Domain"))
domain.rmempty = false
domain.datatype = "hostname"

interface = s:option(ListValue, "interface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end
--interface:value("0", translate("1_INTERNET_R_VID_2"))
--interface:value("1", translate("2_TR069_R_VID_3"))

s:option(Value, "username", translate("Username")).rmempty = true
s:option(Value, "password", translate("Password")).rmempty = true

return m
