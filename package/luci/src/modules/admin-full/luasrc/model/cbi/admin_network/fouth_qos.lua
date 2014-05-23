--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_mac.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")
local dsp = require "luci.dispatcher"

arg[1] = arg[1] or ""

m = Map("qos", nil)

m.redirect = dsp.build_url("admin", "network", "qos","rule")
--[[
if not m.uci:get(arg[1]) == "qos" then
	luci.http.redirect(m.redirect)
	return
end
]]--
s = m:section(NamedSection, arg[1], "App", translate("Qos"))
s.anonymous = true
s.addremove = false

appname = s:option(ListValue, "AppName", translate("Application"))
appname:value("tr069", translate("TR069"))
appname:value("iptv", translate("IPTV"))
 
ClassQueue = s:option(ListValue, "ClassQueue", translate("ClassQueue"))
ClassQueue:value("1", translate("1"))
ClassQueue:value("2", translate("2"))
ClassQueue:value("3", translate("3"))
ClassQueue:value("4", translate("4"))

return m

