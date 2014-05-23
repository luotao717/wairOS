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
m.redirect = dsp.build_url("admin", "network", "qos","qosf")



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

--[[


pminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
pminvalue:depends("type","2")

pmaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
pmaxvalue:depends("type","2")

pprotol = m:field(ListValue, "protol", translate("Protol"))



siminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
siminvalue:depends("type","3")

simaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
simaxvalue:depends("type","3")

siprotol = m:field(ListValue, "protol", translate("Protol"))



dipminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
dipminvalue:depends("type","4")

dipmaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
dipmaxvalue:depends("type","4")

dipprotol = m:field(ListValue, "protol", translate("Protol"))


sminvalue = m:field(Value, "minvalue", translate("Dscpmin"))   
sminvalue:depends("type","5")

smaxvalue = m:field(Value, "maxvalue", translate("Dscpmax")) 
smaxvalue:depends("type","5")

sprotol = m:field(ListValue, "protol", translate("Protol"))


dpminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
dpminvalue:depends("type","6")

dpmaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
dpmaxvalue:depends("type","6")

dpprotol = m:field(ListValue, "protol", translate("Protol"))

tminvalue = m:field(ListValue, "minvalue", translate("TOS Select"))   
tminvalue:value("1", translate("0"))
tminvalue:value("2", translate("2"))
tminvalue:value("3", translate("4"))
tminvalue:value("4", translate("8"))
tminvalue:value("5", translate("16"))
tminvalue:depends("type","7")


dsminvalue = m:field(Value, "minvalue", translate("Minvalue"))   
dsminvalue:depends("type","8")
dsmaxvalue = m:field(Value, "maxvalue", translate("Maxvalue")) 
dsmaxvalue:depends("type","8")





Interface = m:field(ListValue, "minvalue", translate("Interface Select"))   
for o,v in pairs(wanlnk.waninfo_get()) do
	Interface:value(v.Interface,translate(v.ConnName))
end
Interface:depends("type","9")



lanmin = m:field(ListValue, "minvalue", translate("Minvalue"))   
lanmin:value("1", translate("LAN1"))
lanmin:value("2", translate("LAN2"))
lanmin:value("3", translate("LAN3"))
lanmin:value("4", translate("LAN4"))
lanmin:value("5", translate("SSID1"))
lanmin:value("6", translate("SSID2"))
lanmin:value("7", translate("SSID3"))
lanmin:value("8", translate("SSID4"))
lanmin:depends("type","10")

lanmax = m:field(ListValue, "maxvalue", translate("Maxvalue")) 
lanmax:value("1", translate("LAN1"))
lanmax:value("2", translate("LAN2"))
lanmax:value("3", translate("LAN3"))
lanmax:value("4", translate("LAN4"))
lanmax:value("5", translate("SSID1"))
lanmax:value("6", translate("SSID2"))
lanmax:value("7", translate("SSID3"))
lanmax:value("8", translate("SSID4"))
lanmax:depends("type","10")
]]--
lanprotol = m:field(ListValue, "protol", translate("Protol"))
lanprotol:value("1", translate("TCP"))
lanprotol:value("2", translate("UDP"))
lanprotol:value("3", translate("TCP/UDP"))
lanprotol:value("4", translate("RTP"))
lanprotol:value("5", translate("ICMP"))
lanprotol:value("6", translate("ALL"))


function m.handle(self, state, data)
	if state == FORM_VALID then
       local typ = uci:get("qos",arg[1])
       local newqos = uci:section("qos","typ")
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
