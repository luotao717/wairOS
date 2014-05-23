--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wlan.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local os = require "os"

m = Map("voip", translate("VOIP"), "")

m.redirect = dsp.build_url("admin", "services", "voip")

s1 = m:section(TypedSection, "sip", translate("SIP Settings"))
s1.anonymous = true
s1.addremove = false


leaseTime = s1:option(Value, "Lease", translate("Leasetime (s)"),translate("60~3600"))
leaseTime.datatype = "range(60,3600)"

packEnable=s1:option(Flag, "PackEnabled", translate("Prack Enable"))
packEnable.rmempty = false

sessionEnable=s1:option(Flag, "SessionEnabled", translate("Session Enable"))
sessionEnable.rmempty = false

updataTime=s1:option(Value, "SessionUpdate", translate("Session Update Time(s)"),translate("60~3600"))
updataTime.datatype = "range(60,3600)"

paraEnable=s1:option(Flag, "UserParaEnabled", translate("User Para Enable"))
paraEnable.rmempty = false




return m
