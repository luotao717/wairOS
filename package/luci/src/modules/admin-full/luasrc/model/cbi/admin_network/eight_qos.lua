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
local sid = arg[1]


local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()

m = SimpleForm("qos", translate("Qos"))
m.redirect = dsp.build_url("admin", "network", "qos","seven")
m.errmessage = "the section name  " .. arg[1] 


type = m:field(ListValue, "type", translate("Mode")) 
type:value("1", translate("SMAC"))
type:value("2", translate("8021P"))
type:value("3", translate("SIP"))
type:value("4", translate("DIP"))
type:value("5", translate("SPORT"))
type:value("6", translate("DPORT"))
type:value("7", translate("TOS"))
type:value("8", translate("DSCP"))
type:value("9", translate("WANInterface"))
type:value("10", translate("LANInterface"))


smminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
--smminvalue:depends("type","1")
smmaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
--smmaxvalue:depends("type","1")


lanprotol = m:field(ListValue, "protol", translate("Protol"))
lanprotol:value("1", translate("TCP"))
lanprotol:value("2", translate("UDP"))
lanprotol:value("3", translate("TCP/UDP"))
lanprotol:value("4", translate("RTP"))
lanprotol:value("5", translate("ICMP"))
lanprotol:value("6", translate("ALL"))


function m.handle(self, state, data)
	if state == FORM_VALID then
  --     local typ = uci:get("qos",arg[1])
       local newqos = uci:section("qos",arg[1])
	if data.type then
  		uci:set("qos",newqos,"type", data.type)
  	end
  	if data.minvalue then
  		uci:set("qos",newqos,"minvalue", data.minvalue)
  	end
  	 if data.maxvalue then
  		uci:set("qos",newqos,"maxvalue", data.maxvalue)
  	end
  	 if data.protol then
  		uci:set("qos",newqos,"protol", data.protol)
  	end
  	 uci:save("qos")
  	 
        luci.http.redirect(luci.dispatcher.build_url("admin/network/qos/seven"))
	end
end

return m
