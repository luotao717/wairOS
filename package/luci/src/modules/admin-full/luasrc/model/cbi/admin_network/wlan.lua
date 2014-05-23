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

function wep_mode_validate(value,key_len,sel_idx,key_array)
	local valid = true
	local valid2 = true
	local err_key = 0

	for i, k in ipairs(key_array) do
	   err_key = i
	   if i == sel_idx then
		-- not allowed be null
		if string.len(k) == 0 then
			valid = false
			break
		end
	   end
	   if key_len == 40 then
		if string.len(k) ~= 0 and string.len(k) ~= 5 and string.len(k) ~= 10 then
			valid2 = false
			break
		end
	   elseif key_len == 104 then
		if string.len(k) ~= 0 and string.len(k) ~= 13 and string.len(k) ~= 26 then
			valid2 = false
			break
		end
	   else
		valid2 = false
		break
	   end
	end
	
	if not valid then
		return nil, translatef("WEP Key%d is empty, please input again !",err_key)
	elseif not valid2 then 
		return nil, translatef("WEP Key%d is invalid, please input again !",err_key)
  	else
		return value
	end		  
end

m = Map("wireless", translate("Wireless"), "")
m.pagewlanaction = true
m.redirect = dsp.build_url("admin", "network", "wlan")

local value_hmode 


s1 = m:section(TypedSection, "wifi_ctrl", translate("Wireless Settings"))
s1.anonymous = true
s1.addremove = false

s1:option(Flag6, "enabled", translate("Enable Wireless")).rmempty = false

s2 = m:section(TypedSection, "wifi_device", translate("Basic Settings"))
s2.anonymous = true
s2.addremove = false



hwmode = s2:option(ListValue, "hwmode", translate("Network Type"))
hwmode:value("11bgn", translate("802.11b/g/n Mixed"))
hwmode:value("11bg", translate("802.11b/g Mixed"))
hwmode:value("11n", translate("802.11n"))
hwmode:value("11g", translate("802.11g"))
hwmode:value("11b", translate("802.11b"))


channel = s2:option(ListValue, "channel", translate("Channel"))
channel:value("0", translate("Auto"))
channel:value("1", translate("1"))
channel:value("2", translate("2"))
channel:value("3", translate("3"))
channel:value("4", translate("4"))
channel:value("5", translate("5"))
channel:value("6", translate("6"))
channel:value("7", translate("7"))
channel:value("8", translate("8"))
channel:value("9", translate("9"))
channel:value("10", translate("10"))
channel:value("11", translate("11"))
channel:value("12", translate("12"))
channel:value("13", translate("13"))

htBw = s2:option(ListValue, "HtBw", translate("Channel Bandwidth"), translate("MHZ"))
htBw:value("0", translate("20"))
htBw:value("1", translate("20/40"))
htBw:depends({hwmode="11bgn"})
htBw:depends({hwmode="11n"})
ratel1 = s2:option(ListValue, "sn_rate", translate("Rate"))
ratel1:value("0", translate("Auto"))
ratel1:value("30", translate("30 Mbps"))
ratel1:value("60", translate("60 Mbps"))
ratel1:value("90", translate("90 Mbps"))
ratel1:value("120", translate("120 Mbps"))
ratel1:value("180", translate("180 Mbps"))
ratel1:value("240", translate("240 Mbps"))
ratel1:value("270", translate("270 Mbps"))
ratel1:value("300", translate("300 Mbps"))
ratel1:depends({HtBw="1", HtGi="1", hwmode="11bgn"})
ratel1:depends({HtBw="1", HtGi="1", hwmode="11n"})


	function ratel1.cfgvalue(self,s)
		local value = self.map:get(s,"abgn_rate")
		return value
	end
	function ratel1.write(self,s,value)
		return self.map:set(s,"abgn_rate",value)
	end

rates1 = s2:option(ListValue, "ln_rate", translate("Rate"))
rates1:value("0", translate("Auto"))
rates1:value("27", translate("27 Mbps"))
rates1:value("54", translate("54 Mbps"))
rates1:value("81", translate("81 Mbps"))
rates1:value("108", translate("108 Mbps"))
rates1:value("162", translate("162 Mbps"))
rates1:value("216", translate("216 Mbps"))
rates1:value("243", translate("243 Mbps"))
rates1:value("270", translate("270 Mbps"))
rates1:depends({HtBw="1", HtGi="0", hwmode="11bgn"})
rates1:depends({HtBw="1", HtGi="0", hwmode="11n"})

	function rates1.cfgvalue(self,s)
		local value = self.map:get(s,"abgn_rate")
		return value
	end
	function rates1.write(self,s,value)
		return self.map:set(s,"abgn_rate",value)
	end

ratel2 = s2:option(ListValue, "shn_rate", translate("Rate"))
ratel2:value("0", translate("Auto"))
ratel2:value("14.4", translate("14.4 Mbps"))
ratel2:value("28.9", translate("28.9 Mbps"))
ratel2:value("43.3", translate("43.3 Mbps"))
ratel2:value("57.8", translate("57.8 Mbps"))
ratel2:value("86.7", translate("86.7 Mbps"))
ratel2:value("115.6", translate("115.6 Mbps"))
ratel2:value("130", translate("130 Mbps"))
ratel2:value("144.4", translate("144.4 Mbps"))
ratel2:depends({HtBw="0", HtGi="1", hwmode="11bgn"})
ratel2:depends({HtBw="0", HtGi="1", hwmode="11n"})


function ratel2.cfgvalue(self,s)
	local value = self.map:get(s,"abgn_rate")
	return value
end
function ratel2.write(self,s,value)
	return self.map:set(s,"abgn_rate",value)
end


rates2 = s2:option(ListValue, "lhn_rate", translate("Rate"))
rates2:value("0", translate("Auto"))
rates2:value("13", translate("13 Mbps"))
rates2:value("26", translate("26 Mbps"))
rates2:value("39", translate("39 Mbps"))
rates2:value("52", translate("52 Mbps"))
rates2:value("78", translate("78 Mbps"))
rates2:value("104", translate("104 Mbps"))
rates2:value("117", translate("117 Mbps"))
rates2:value("130", translate("130 Mbps"))
rates2:depends({HtBw="0", HtGi="0", hwmode="11bgn"})
rates2:depends({HtBw="0", HtGi="0", hwmode="11n"})

function rates2.cfgvalue(self,s)
	local value = self.map:get(s,"abgn_rate")
	return value
end
function rates2.write(self,s,value)
	return self.map:set(s,"abgn_rate",value)
end

rate3= s2:option(ListValue, "g_rate", translate("Rate"))
rate3:value("0", translate("Auto"))
rate3:value("1", translate("1 Mbps"))
rate3:value("2", translate("2 Mbps"))
rate3:value("5", translate("5.5 Mbps"))
rate3:value("6", translate("6 Mbps"))
rate3:value("9", translate("9 Mbps"))
rate3:value("11", translate("11 Mbps"))
rate3:value("12", translate("12 Mbps"))
rate3:value("18", translate("18 Mbps"))
rate3:value("24", translate("24 Mbps"))
rate3:value("36", translate("36 Mbps"))
rate3:value("48", translate("48 Mbps"))
rate3:value("54", translate("54 Mbps"))
rate3:depends({hwmode="11bg"})
rate3:depends({hwmode="11g"})
function rate3.cfgvalue(self,s)
	local value = self.map:get(s,"abgn_rate")
	return value
end
function rate3.write(self,s,value)
	return self.map:set(s,"abgn_rate",value)
end

rate4 = s2:option(ListValue, "b_rate", translate("Rate"))
rate4:value("0", translate("Auto"))
rate4:value("1", translate("1 Mbps"))
rate4:value("2", translate("2 Mbps"))
rate4:value("5", translate("5.5 Mbps"))
rate4:value("11", translate("11 Mbps"))
rate4:depends({hwmode="11b"})
function rate4.cfgvalue(self,s)
	local value = self.map:get(s,"abgn_rate")
	return value
end	
function rate4.write(self,s,value)
	return self.map:set(s,"abgn_rate",value)
end

txpower = s2:option(ListValue, "txpower", translate("Tx Power"))
txpower:value("1", translate("100%"))
txpower:value("2", translate("75%"))
txpower:value("3", translate("50%"))
txpower:value("4", translate("25%"))
txpower:value("5", translate("15%"))

GuardInterval = s2:option(ListValue, "HtGi", translate("Guard Interval"))
GuardInterval:value("0", translate("Long"))
GuardInterval:value("1", translate("Short"))
GuardInterval:depends({hwmode="11bgn"})
GuardInterval:depends({hwmode="11n"})

--country_id = s2:option(Value, "countryid", translate("Country Code"))
country_id = s2:option(ListValue, "countryid", translate("Country Code"))
country_id:value("32", translate("Argentina")) --Argentina
country_id:value("156", translate("China"))
country_id:value("344", translate("HongKong"))
country_id:value("360", translate("Indonesia")) --Indonesia
country_id:value("356", translate("India")) --India
country_id:value("158", translate("Taiwan"))
country_id:value("764", translate("Thailand")) --Thailand
country_id:value("458", translate("Malaysia")) --Malaysia
country_id:value("586", translate("Pakistan")) --Pakistan
country_id:value("608", translate("Philippines")) --Philippines
country_id:value("702", translate("Singapore")) --Singapore
country_id:value("724", translate("Spain")) --Spain
country_id:value("840", translate("United States"))
country_id:value("704", translate("Vietnam"))
country_id.default = "156"

--////////////////////////
s = m:section(TypedSection, "wifi_iface", translate("Security Settings"))
s.anonymous = true
s.addremove = false
--all section of same type show title
s.allshowtitle = true

idx = s:option(DummyValue, "idx", translate("SSID Index"))

ssid = s:option(Value, "ssid", translate("SSID"))
ssid.rmempty = false

s:option(Flag, "enabled", translate("Enable SSID")).rmempty = false
s:option(Flag, "hidden", translate("Hidden SSID")).rmempty = false
s:option(Flag, "isolate", translate("AP Isolation")).rmempty = false

encryption = s:option(ListValue, "securityMode", translate("Encrypt Type"))
encryption:value("NONE", translate("None"))
encryption:value("WEP", translate("WEP"))
encryption:value("WPAPSK", translate("WPA-PSK"))
encryption:value("WPA2PSK", translate("WPA2PSK"))
encryption:value("WPAPSKWPA2PSK", translate("WPA-PSK/WPA2-PSK"))

wep_mode = s:option(ListValue, "wepMode", translate("WEP Mode"))
wep_mode:value("OPEN", translate("Open"))
wep_mode:value("SHARED", translate("Share"))
wep_mode:value("WEPAUTO", translate("Open+Share"))
wep_mode:depends("securityMode","WEP")

function wep_mode.validate(self, value,section)
	local valid = true
	local valid2 = true
	
	local k1 = tostring(w1_wep_key1:formvalue(section))
	local k2 = tostring(w1_wep_key2:formvalue(section))
	local k3 = tostring(w1_wep_key3:formvalue(section))
	local k4 = tostring(w1_wep_key4:formvalue(section))
	local valuek = tonumber(w1_key_idx:formvalue(section))
	local key_length = tonumber(key_lengths:formvalue(section))
	local key_array = {k1,k2,k3,k4}

	return wep_mode_validate(value,key_length,valuek,key_array)
end


key_lengths = s:option(ListValue, "keyLevel", translate("WEP Key Length"),
		translate("40-bit: the key is 5 characters or 10 hex-digits;<br />" ..
			   "104-bit: the key is 13 characters or 26 hex-digits"))
key_lengths.orientation = "vertical"
key_lengths:value("40", translate("40-bit"))
key_lengths:value("104", translate("104-bit"))
key_lengths:depends("securityMode","WEP")

w1_key_idx = s:option(ListValue, "keyIdx", translate("WEP Key Index"))
w1_key_idx:value("1", translate("1"))
w1_key_idx:value("2", translate("2"))
w1_key_idx:value("3", translate("3"))
w1_key_idx:value("4", translate("4"))
w1_key_idx:depends("securityMode","WEP")


w1_wep_key1 = s:option(Value, "key1", translate("WEP Key 1"))
w1_wep_key1.rmempty = true
w1_wep_key1.datatype = "wepkey"
w1_wep_key1.password = true
w1_wep_key1:depends("securityMode","WEP")

w1_wep_key2 = s:option(Value, "key2", translate("WEP Key 2"))
w1_wep_key2.rmempty = true
w1_wep_key2.datatype = "wepkey"
w1_wep_key2.password = true
w1_wep_key2:depends("securityMode","WEP")

w1_wep_key3 = s:option(Value, "key3", translate("WEP Key 3"))
w1_wep_key3.rmempty = true
w1_wep_key3.datatype = "wepkey"
w1_wep_key3.password = true
w1_wep_key3:depends("securityMode","WEP")

w1_wep_key4 = s:option(Value, "key4", translate("WEP Key 4"))
w1_wep_key4.rmempty = true
w1_wep_key4.datatype = "wepkey"
w1_wep_key4.password = true
w1_wep_key4:depends("securityMode","WEP")

wpa_alg = s:option(ListValue, "wpaAlg", translate("WPA Cipher"))
wpa_alg:value("TKIP", translate("TKIP"))
wpa_alg:value("AES", translate("AES"))
wpa_alg:value("TKIPAES", translate("AES+TKIP"))
wpa_alg:depends("securityMode","WPAPSK")
wpa_alg:depends("securityMode","WPA2PSK")
wpa_alg:depends("securityMode","WPAPSKWPA2PSK")
function wpa_alg.validate(self, value,section)
	local valid = true
	local kwpa = tostring(key:formvalue(section))

	if kwpa ~=""then
	else
      		valid = false
      	end
	
	if valid then
		return value
	else
		return nil,translate("WPA Key is empty, please input again !")
	end
	
end

key = s:option(Value, "wpapsk", translate("WPA Key"),translate("the length of key is no less than 8 and no more than 64"))
key.rmempty = true
key:depends("securityMode","WPAPSK")
key:depends("securityMode","WPA2PSK")
key:depends("securityMode","WPAPSKWPA2PSK")
key.datatype = "wpakey"
key.password = true
key.orientation = "vertical"

wpsenable=s:option(Flag, "WPSEnable", translate("Enable WPS"))
wpsenable.rmempty = false
wpsenable:depends({idx="SSID1",securityMode="WPAPSK"})
wpsenable:depends({idx="SSID1",securityMode="WPA2PSK"})
wpsenable:depends({idx="SSID1",securityMode="WPAPSKWPA2PSK"})

pbc = s:option(Button, "wps_pbc", translate("Start PBC"))
function pbc.validate(self,s)
	local wps = luci.util.exec("cat /etc/config/wireless|grep WPSEnable|grep 1")
	if string.len(wps) == 0 then
		return nil, translate("Please apply to save wps configuration first!")
	else
		return true
	end
end
function pbc.write(self,s)
	os.execute("/sbin/hostapd_cli -i ath0 wps_pbc &")
	luci.http.redirect(dsp.build_url("admin", "network", "wlan"))
end
pbc:depends("WPSEnable", "1")


local fs = require "nixio.fs"
pincode = s:option(Value, "pinCode", translate("PIN"),translate("the value of PIN is number and the length is 8"))
pincode.datatype = "wpskey"
pincode.orientation = "vertical"
pincode:depends("WPSEnable","1")
function pincode.write(self,section,value)
	os.execute("echo " .. value .. " >/tmp/.Enrollee_pin")
end
function pincode.cfgvalue()
	local pin=fs.readfile("/tmp/.Enrollee_pin") or ""
	pin=pin:gsub("\n","")
	return pin
end

pin = s:option(Button, "wps_pin", translate("Start PIN"))
function pin.validate(self,s)
	local wps = luci.util.exec("cat /etc/config/wireless|grep WPSEnable|grep 1")
	if string.len(wps) == 0 then
		return nil, translate("Please apply to save wps configuration first!")
	else
		return true
	end
end
function pin.write(self,s)
	local pin=fs.readfile("/tmp/.Enrollee_pin") or ""
	pin=pin:gsub("\n","")
	os.execute("/sbin/hostapd_cli -i ath0 wps_pin any " .. pin .. " 300 &")
	luci.http.redirect(dsp.build_url("admin", "network", "wlan"))
end
pin:depends("WPSEnable", "1")
return m
