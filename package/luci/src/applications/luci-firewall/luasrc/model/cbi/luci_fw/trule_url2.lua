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
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

m1 = SimpleForm("firewall", translate("Black Name"))
m1.redirect = dsp.build_url("admin", "security", "urlf")


flag = m1:field(ListValue, "policy", translate("Mode"))
flag:value("deny", translate("black list"))


m = SimpleForm("firewall", translate("Url Filter"))
m.redirect = dsp.build_url("admin", "security", "urlf")

url = m:field(Value, "url", translate("Url Address"))
url.rmempty = false
url.datatype = "ipandurl"
url.default = "www.google.com"

--validate: 	a validation function returning nil if the section is invalid
-- validate rule: the value can not be repeated
function url.validate(self, value,section)
	
	
		local valid = true
		uci:foreach("firewall", "url_deny", function(section)
				 if section.url == value    then
					valid = false
					m.message = translate("URL record repeat,please re-enter other URL ")
				end
			end)
		uci:foreach("firewall", "url_permit", function(section)
				 if section.url == value    then
					valid = false
					m.message = translate("URL has used by white list ,please re-enter other URL ")
				end
			end)
		
		if valid then
			return value
		else
			return nil
		end

end

host = m:field(Value, "host", translate("Host"))
host.rmempty = true
--host.size = 5
--host.maxlength = 5
host.default = "google"
host.datatype = "hostname"

name = m:field(Value, "name", translate("Description"))
name.rmempty = true
--name.size = 5
--name.maxlength = 5
name.default = "google"
name.datatype = "hostname"

function m.handle(self, state, data)
	if state == FORM_VALID then   
       	local new_url = uci:section("firewall","url_deny")
	if data.url then
  		uci:set("firewall",new_url,"url", data.url)
  	end
  	if data.host then
  		uci:set("firewall",new_url,"host", data.host)
  	end
  	if data.name then
  		uci:set("firewall",new_url,"name", data.name)
  	end
  	 uci:save("firewall")

        luci.http.redirect(luci.dispatcher.build_url("admin/security/urlf"))

	
	end
end

return m
