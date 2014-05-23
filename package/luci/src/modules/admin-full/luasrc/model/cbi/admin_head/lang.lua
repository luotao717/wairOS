--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: system.lua 6718 2011-01-13 22:13:35Z jow $
]]--

require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.fs")
require("luci.config")
local ds = require "luci.dispatcher"

m = Map("system", translate("System"))
m:chain("luci")
m.redirect = ds.build_url("admin", "head", "language")



s = m:section(TypedSection, "system", translate("Language Settings"))
s.anonymous = true
s.addremove = false



--
-- Langauge
--

o = s:option( ListValue, "language", translate("Language"))
o:value("auto","Auto")

local i18ndir = luci.i18n.i18ndir .. "base."
for k, v in luci.util.kspairs(luci.config.languages) do
	local file = i18ndir .. k:gsub("_", "-")
	if k:sub(1, 1) ~= "." and luci.fs.access(file .. ".lmo") then
		o:value(k, v)
	end
end

function o.cfgvalue(...)
	return m.uci:get("luci", "main", "lang")
end

function o.write(self, section, value)
	m.uci:set("luci", "main", "lang", value)
	luci.http.redirect( luci.dispatcher.build_url("admin/head/language"))
end




return m
