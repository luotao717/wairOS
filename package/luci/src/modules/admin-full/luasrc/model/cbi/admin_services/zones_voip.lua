--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wlan.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local os = require "os"

m = Map("voip", translate("VOIP"), "")

m.redirect = dsp.build_url("admin", "services", "voip")

s1 = m:section(TypedSection, "sip", translate("SIP Settings"))
s1.anonymous = true
s1.addremove = false

sipServer = s1:option(Value, "Server", translate("SIP Server"))
sipServer.datatype = "ipandurl"

sipServerPort=s1:option(Value, "ServerPort", translate("SIP Server Port"))
sipServerPort.datatype = "port"

userDomain=s1:option(Value, "UserAgentDomain", translate("User Agent Domain"))
userDomain.datatype = "hostname"

domainPort=s1:option(Value, "UserAgentPort", translate("User Agent Port"))
domainPort.datatype = "port"

button1 = s1:option(Button, "_redirect", translate("VOIP Settings"))
function button1.write(self, s1)

	luci.http.redirect(dsp.build_url("admin", "services", "voip","rule"))
end

s2 = m:section(TypedSection, "phyiface", translate("SIP Basic Settings"))
s2.anonymous = true
s2.addremove = false


portEnable=s2:option(Flag, "Enabled", translate("SIP Account Enable"))
portEnable.rmempty = false

portcountrymode=s2:option(ListValue,"_PortCountryMode",translate("PortCountryMode"))
portcountrymode.rmempty=false
portcountrymode.default = "2"
portcountrymode:value("0", translate("US"))
portcountrymode:value("1", translate("Jpan"))
portcountrymode:value("2", translate("China"))
portcountrymode:value("3", translate("India"))
portcountrymode:value("4", translate("Germany"))
portcountrymode:value("5", translate("UK"))
portcountrymode:value("6", translate("Australia"))
portcountrymode:depends("Enabled","1")

function portcountrymode.cfgvalue(self,section)
	return m.uci:get("voip", "advancepolicy_0", "PortCountryMode")
end

function portcountrymode.write(self, section, value)
	m.uci:set("voip", "advancepolicy_0", "PortCountryMode", value)
end


portAccount=s2:option(Value, "AuthUserID", translate("SIP Account"))
portAccount:depends("Enabled",1)

portName=s2:option(Value, "AuthUserName", translate("SIP Registration Certificate Name"))
portName:depends("Enabled",1)

portPassword=s2:option(Value2, "AuthUserPassword", translate("SIP Registration Certificate Password"))
portPassword.password = true
portPassword:depends("Enabled",1)


button2 = s2:option(Button, "_redirect", translate("VOIP Basic Settings"))
function button2.write(self, s2)

	luci.http.redirect(dsp.build_url("admin", "services", "voip","third"))
end

s4 = m:section(TypedSection, "dialdpolicy", translate("Dialed Policy"))
s4.anonymous = true
s4.addremove = false

--[[
dailedKey1=s4:option(Flag, "key_enable", translate("Dialed Enabled"))
dailedKey1.rmempty = false
]]--
dailedKey2=s4:option(Value, "key_dailrule", translate("Dialed Rule"))
dailedKey2.rmempty = false

dailedKey3=s4:option(Value, "key_serverip", translate("Dialed Server IP"))
dailedKey3.rmempty = false
dailedKey3.datatype = "ipaddrformat"

dailedKey4=s4:option(Value, "key_serverport", translate("Dialed Server Port"))
dailedKey4.rmempty = false
dailedKey4.datatype = "port"

dailedKey5=s4:option(Value, "key_number", translate("Dialed Number Translation"))
dailedKey5.rmempty = false

dailedKey6=s4:option(Flag, "key_num_en", translate("Dialed Number Translation Enable"))
dailedKey6.rmempty = false
function dailedKey6.write(self, section, value)
	--local val1=dailedKey1:formvalue(section)
	local val2=dailedKey2:formvalue(section)
	local val3=dailedKey3:formvalue(section)
	local val4=dailedKey4:formvalue(section)
	local val5=dailedKey5:formvalue(section)
	--	local val =val1..'&'..val2..'&'..val3..'&'..val4..'&'..val5..'&'..value 
	local val ='1'..'&'..val2..'&'..val3..'&'..val4..'&'..val5..'&'..value
	 self.map:set(section, "Key", val)
	return  self.map:set(section,self.option,value) 
end


button3 = s4:option(Button, "_redirect", translate("VOIP Advanced Settings"))

function button3.write(self, s4)
	luci.http.redirect(dsp.build_url("admin", "services", "voip","forth"))
end

s5 = m:section(TypedSection, "voip_stun", translate("STUN Settings"))
s5.anonymous = true
s5.addremove = false

stunen=s5:option(Flag, "enabled", translate("STUN Enabled"))

priip = s5:option(Value, "primary_serverip", translate("Primary STUN Server IP"))
priip.rmempty = true
priip.maxlength = 15
priip.datatype = "ipaddr"
priip:depends({ enabled = "1" })

secip = s5:option(Value, "secondary_serverip", translate("Secondary STUN Server IP"))
secip.rmempty = true
secip.maxlength = 15
secip.datatype = "ipaddr"
secip:depends({ enabled = "1" })


return m
