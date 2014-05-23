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

s2 = m:section(TypedSection, "phyiface", translate("SIP Settings"))
s2.anonymous = true
s2.addremove = false


registerEnable=s2:option(Flag, "RegisterEnabled", translate("Register Enable"))
registerEnable.rmempty = false

reverseRegister=s2:option(Flag, "ReverseRegister", translate("ReverseRegister Enable"))
reverseRegister.rmempty = false



anonymousEnable=s2:option(Flag, "AnonymousEnabled", translate("Anonymous Call Enabled"))
anonymousEnable.rmempty = false

numberEnable=s2:option(Flag, "CalledNumberEnabled", translate("Called Number Display Enabled"))
numberEnable.rmempty = false

dailEnable=s2:option(Flag, "DailDReserveEnabled", translate("Dail Reserve Enabled"))
dailEnable.rmempty = false

waitEnable=s2:option(Flag, "DailDWaitEnabled", translate("Dail Wait Enabled"))
waitEnable.rmempty = false

divertEnable=s2:option(ListValue, "CallDivertEnabled", translate("Call Divert Mode"))
divertEnable:value("0", translate("Close"))
divertEnable:value("1", translate("Unconditional"))
divertEnable:value("2", translate("Busy"))
divertEnable:value("3", translate("No answer"))


divertNumber=s2:option(Value, "CallDivertNumber", translate("Call Divert Number"))


denyEnable=s2:option(Flag, "CallDenyEnabled", translate("Call Deny Enable"))
denyEnable.rmempty = false


callEnable=s2:option(Flag, "CallHeatLineEnabled", translate("HeatLine Enable"))
callEnable.rmempty = false

callNumber=s2:option(Value, "CallHeatLineNumber", translate("Call HeatLine Number"))


callDelay=s2:option(Value, "CallHeatLineDelay", translate("Call HeatLine Delay(s)"),translate("0~10"))
callDelay.datatype = "range(0,10)"



return m
