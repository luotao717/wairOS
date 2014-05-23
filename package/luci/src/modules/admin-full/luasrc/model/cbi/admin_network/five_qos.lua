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
 




ClassQueue = m:field( ListValue, "ClassQueue", translate("ClassQueue"))
ClassQueue:value("1", translate("1"))
ClassQueue:value("2", translate("2"))
ClassQueue:value("3", translate("3"))
ClassQueue:value("4", translate("4"))
ClassQueue:value("5", translate("5"))
ClassQueue:value("6", translate("6"))

DSCPMarkValue = m:field(Value, "DSCPMarkValue", translate("DSCPMarkValue"),translate("(0~63)")) 
DSCPMarkValue.datatype =  "range(0,63)"
DSCPMarkValue.rmempty = false

P_Value = m:field(Value, "P_Value", translate("802.1P Value"),translate("(0~7)"))
P_Value.datatype =  "range(0,7)"
P_Value.rmempty = false


function m.handle(self, state, data)
	if state == FORM_VALID then
 	
       local newqos = uci:section("qos","Classification")

	if data.ClassQueue then
  		uci:set("qos",newqos,"ClassQueue", data.ClassQueue)
  	end
  	if data.DSCPMarkValue then
  		uci:set("qos",newqos,"DSCPMarkValue", data.DSCPMarkValue)
  	end
  	 if data.P_Value then
  		uci:set("qos",newqos,"P_Value", data.P_Value)
  	end

  	 uci:save("qos")
        luci.http.redirect(luci.dispatcher.build_url("admin/network/qos/rule"))

	
	end
end
 
return m
