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

--arg[1] = arg[1] or ""

m = SimpleForm("ddns", translate("DDNS"))

m.redirect = dsp.build_url("admin", "services", "ddns")
--[[
if not m.uci:get(arg[1]) == "service" then
	luci.http.redirect(m.redirect)
	return
end
]]--
--[[
s = m:section(NamedSection, arg[1], "service", translate("DDNS Settings"))
s.anonymous = true
s.addremove = false
]]--
enable= m:field(Flag, "enabled", translate("Enabled"))
enable.rmempty=true

svc= m:field(ListValue, "service_name", translate("Service"))
svc.rmempty = false
--service.default = "dyndns.org"
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

servicep = m:field(Value, "service_port", translate("Service Port"))
servicep.rmempty = false
servicep.default = "80"
servicep.datatype = "port"

hostn = m:field(Value, "hostname", translate("Host Name"))
hostn.rmempty = false
hostn.datatype = "hostname"
hostn.default = "mypersonaldomain"

domain = m:field(Value, "domain", translate("Domain"))
domain.rmempty = false
domain.datatype = "hostname"
domain.default = "dyndns.org"
function domain.validate(self, value,section)
	
	
		local valid = true

			uci:foreach("ddns", "service", function(section)
				 if section.domain == value  then
					valid = false
					m.message = translate("Domain record repeat,please re-enter other domain ")
				end
			end)
	

		if valid then
			return value
		else
			return nil
		end

end

interface =m:field(ListValue, "iface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end


username=m:field(Value, "username", translate("Username"))
username.rmempty = false
username.default = "useradmin" 

passwd   = m:field(Value, "password", translate("Password"))
passwd.rmempty = false


function m.handle(self, state, data)
	if state == FORM_VALID then


  --cursor:load("firewall")
  	-- uci:section("firewall","mac",nil,{src_mac = data.src_mac, dir = data.dir})
   
       local newddns = uci:section("ddns","service")
	if data.service_name then
  		uci:set("ddns",newddns,"service_name", data.service_name)
  	end
  	if data.service_port then
  		uci:set("ddns",newddns,"service_port", data.service_port)
  	end
  	if data.hostname then
  		uci:set("ddns",newddns,"hostname", data.hostname)
  	end
  	if data.domain then
  		uci:set("ddns",newddns,"domain", data.domain)
  	end
  	if data.iface then
  		uci:set("ddns",newddns,"iface", data.iface)
  	end
  	if data.username then
  		uci:set("ddns",newddns,"username", data.username)
  	end
  	if data.password then
  		uci:set("ddns",newddns,"password", data.password)
  	end
	if data.enabled then
  		uci:set("ddns",newddns,"enabled", "1")
	end
  	uci:set("ddns",newddns,"ip_source", "interface")
  	 uci:save("ddns")

        luci.http.redirect(luci.dispatcher.build_url("admin/services/ddns"))

	
	end
end
return m
