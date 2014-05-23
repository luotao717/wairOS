--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_router.lua 6983 2011-04-13 00:33:42Z soma $
]]--

--local has_v2 = nixio.fs.access("/lib/network/router.sh")
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()

arg[1] = arg[1] or ""

m = Map("routes", nil)

m.redirect = dsp.build_url("admin", "network", "router")

if not m.uci:get(arg[1]) == "route" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "route", translate("Router Filter"))
s.anonymous = true
s.addremove = false

dip = s:option(Value, "target", translate("Dest IP"))
dip.rmempty = false
dip.maxlength = 15
dip.datatype = "ipaddr"
dip.default="0.0.0.0"

nm = s:option(Value, "netmask", translate("Subnet Mask"))
nm.rmempty = false
nm.maxlength = 15
nm:value("255.255.255.0",translate("255.255.255.0"))
nm:value("255.255.0.0",translate("255.255.0.0"))
nm:value("255.0.0.0",translate("255.0.0.0"))
nm.default="255.255.255.0"


s:option(Flag, "gw_en", translate("Enable Gateway")).rmempty = true

gw = s:option(Value, "gateway", translate("Gateway"))
gw.rmempty = false
gw.maxlength = 15
gw.datatype = "ipaddr"
gw:depends("gw_en",1)
gw.default = "192.168.1.1"

s:option(Flag, "interface_en", translate("Enable Interface")).rmempty = true
interface = s:option(ListValue, "interface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end

--interface:value("0", translate("1_INTERNET_R_VID_2"))
--interface:value("1", translate("2_TR069_R_VID_3"))
interface.rmempty = true
interface:depends("interface_en",1)

return m
