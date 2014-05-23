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

m = SimpleForm("qos", translate("Qos"))

m.redirect = dsp.build_url("admin", "network", "qos","rule")
 
appname = m:field(ListValue, "AppName", translate("AppName"))
appname:value("VOIP", translate("VOIP"))
--appname:value("TR069", translate("NMM"))
function appname.validate(self, value,section)
	local valid = true
	uci:foreach("qos", "App", function(section)
		if section.AppName == value    then
			valid = false
			m.errmessage = translate("AppName repeat,please re-enter other")
		end
	end)

	if valid then
		return value
	else
		return nil
	end
end
 
ClassQueue = m:field(ListValue, "ClassQueue", translate("ClassQueue"))
ClassQueue:value("1", translate("1"))
ClassQueue:value("2", translate("2"))
ClassQueue:value("3", translate("3"))
ClassQueue:value("4", translate("4"))
ClassQueue:value("5", translate("5"))
ClassQueue:value("6", translate("6"))

function m.handle(self, state, data)
	if state == FORM_VALID then


  --cursor:load("firewall")
  	-- uci:section("firewall","mac",nil,{src_mac = data.src_mac, dir = data.dir})
   
       local newqos = uci:section("qos","App")
	if data.AppName then
  		uci:set("qos",newqos,"AppName", data.AppName)
  	end
  	if data.ClassQueue then
  		uci:set("qos",newqos,"ClassQueue", data.ClassQueue)
  	end
  	 uci:save("qos")

        luci.http.redirect(luci.dispatcher.build_url("admin/network/qos/rule"))

	
	end
end

return m
