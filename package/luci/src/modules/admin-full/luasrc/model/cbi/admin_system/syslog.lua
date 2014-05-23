--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: syslog.lua 7013 2011-05-03 03:35:56Z jow $
]]--

local fs = require "nixio.fs"
local ds = require "luci.dispatcher"
--[[
function action_syslog()
	local syslog = luci.sys.syslog()
	luci.template.render("admin_system/syslog", {syslog=syslog})
end
]]--

m = Map("system", translate("System Log"), "")
m.redirect = ds.build_url("admin", "system", "syslog")
s = m:section(TypedSection, "system", translate("System Log"))
s.anonymous = true
s.addremove = false



flag = s:option(ListValue, "log_level", translate("Write Grade"))
flag:value("0", translate("Emergency"))
flag:value("1", translate("Alert"))
flag:value("2", translate("Critical"))
flag:value("3", translate("Error"))
flag:value("4", translate("Warning"))
flag:value("5", translate("Notice"))
flag:value("6", translate("Informational"))

o= s:option(TextValue1, "result")


return m
