--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_url.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/firewall/fw.sh")
local dsp = require "luci.dispatcher"
local cursor = require "luci.model.uci".cursor()

arg[1] = arg[1] or ""

m = Map("firewall", nil)

m.redirect = dsp.build_url("customer", "security", "urlf")

if not m.uci:get(arg[1]) == "rule1" then
	luci.http.redirect(m.redirect)
	return
end

s1 = m:section(NamedSection, arg[1], "defaults", translate("white list"))
s1.anonymous = true
s1.addremove = false

flag = s1:option(ListValue, "policy", translate("Mode"))
flag:value("accept", translate("white list"))

s = m:section(NamedSection, arg[1], "whiterule", translate("Url Filter"))
s.anonymous = true
s.addremove = false

url = s:option(Value, "url", translate("Url Address"))
url.rmempty = false
url.datatype = "ipandurl"
url.default = "www.google.com"

--validate: 	a validation function returning nil if the section is invalid
-- validate rule: the value can not be repeated
function url.validate(self, value,section)
	
	
		local valid = true
		local old_value = self.map.uci:get("firewall",section,"url")
	
		
		if old_value == value then
			valid = true
		else
			self.map.uci:foreach("firewall", "whiterule", function(section)
				 if section.url == value    then
					valid = false
				end
			end)
		end

		if valid then
			return value
		else
			return nil,translate("URL required")
		end

end

host = s:option(Value, "host", translate("Host"))
host.rmempty = true
host.size = 5
host.maxlength = 5
host.default = "google"

name = s:option(Value, "name", translate("Description"))
name.rmempty = true
name.size = 5
name.maxlength = 5
name.default = "google"

return m
