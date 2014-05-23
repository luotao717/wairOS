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

local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"

m = SimpleForm("firewall", translate("MAC Filter"))

m.redirect = dsp.build_url("admin", "security", "macf")
--[[
if  not uci:get("firewall", arg[1]) == "macfilter" then
	luci.http.redirect(m.redirect)
	return
end

]]--
macadd = m:field(Value, "src_mac", translate("MAC Address"))
macadd.datatype = "macaddr"
macadd.rmempty = false
macadd.default ="00:00:00:00:00:01"
function macadd.validate(self, value,section)
		local valid = true

		uci:foreach("firewall", "macfilter", function(section)
				 if section.src_mac == value    then
					valid = false
					m.message = translate("MAC Address required")
				end
			end)
	

		if valid then
			return value
		else
			return nil,translate("MAC Address required")
		end

end
name = m:field(Value, "name", translate("Description"))
name.rmempty = true
name.size = 15
name.maxlength = 15
name.default = "MAC"
--[[
frameflow= m:field(ListValue, "dir", translate("Frame Flow"))
frameflow:value("lan->wan", translate("LAN -> WAN"))
frameflow:value("wan->lan", translate("WAN -> LAN"))
frameflow.default="lan->wan"
frameflow.rmempty = false
]]--

function m.handle(self, state, data)
	if state == FORM_VALID then


  --cursor:load("firewall")
  	-- uci:section("firewall","mac",nil,{src_mac = data.src_mac, dir = data.dir})
   
       local newmac = uci:section("firewall","macfilter")

  	uci:set("firewall",newmac,"src_mac", data.src_mac)
  	uci:set("firewall",newmac,"name", data.name)
  	 uci:save("firewall")

        luci.http.redirect(luci.dispatcher.build_url("admin/security/macf"))

	
	end
end

return m
