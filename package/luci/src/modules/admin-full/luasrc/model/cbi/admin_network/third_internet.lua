--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_internet.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local uci = require "luci.model.uci"
local cursor = uci.cursor()
local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"


local has_wifi = nixio.fs.access("/etc/config/wireless")
--local has_v2 = nixio.fs.access("/lib/network/internet.sh")
local dsp = require "luci.dispatcher"
local has_entry = {}
local wanid = nil
local wanifac = "eth0"
local ip = require"luci.ip"

function check_rang_value(ip1,ip2,ipmask)
	local valid = true
	local ips  = ip.IPv4(ip1,ipmask) 
	local ipe  = ip.IPv4(ip2,ipmask)
	local iphostmin 
	local iphostmax
	
	iphostmin = ips:minhost()
	iphostmax = ips:maxhost()

	if ips:lower(iphostmin) == true then
		valid = false
		ips = iphostmin
	end

	if ips:higher(iphostmax) ==  true then
		valid = false
		ips = iphostmax
	end

	if ipe:lower(iphostmin) == true then
		valid = false
		ipe = iphostmin
	end

	if ipe:higher(iphostmax) ==  true then
		valid = false
		ipe = iphostmax
	end	
	--[[
	if ips:higher(ipe) == true then
		valid = false
		ipe = ips
	end
	]]--	
	return valid,ips:string():match("([%d%.]+)"),ipe:string():match("([%d%.]+)")
end


arg[1] = arg[1] or ""

m = Map("wanctl", nil)
--m.apply_before_commit = true

m.redirect = dsp.build_url("admin", "network", "internet")

if not m.uci:get(arg[1]) == "wanlink" then
	luci.http.redirect(m.redirect)
	return
end


s = m:section(NamedSection, arg[1], "wanlink", translate("Internet Settings"))
s.anonymous = true
s.addremove = false

local vlan = s:option(Value, "VlanID", translate("VLAN ID"),translate("(-1,4093), -1: not set VLAN ID"))
vlan.rmempty = false
vlan.size = 4
vlan.datatype = "range(-1,4093)"
function vlan.validate(self,value,section)
	-- get the value now configuration
	local tvalid = true
	local old_value = self.map.uci:get("wanctl",section,"VlanID")
	
	if old_value == value then
		tvalid	= true
	else
		self.map.uci:foreach("wanctl", "wanlink", function(section)
			if  section.VlanID == value    then
				
				m.message = translate("ID repeat, please enter another Vlan ID")
				tvalid = false
			end
		end)	
	end




	if value == "0" then
		m.message =translate("0 ID can not be set, please enter another Vlan ID")
		tvalid = false
	end

	if tvalid then
		return Value:validate(value,section)
	else
		return nil,""
	end
	
end



local Cos = s:option(Value, "Cos", translate("802.1p"),"(0-7)")
Cos.rmempty = false
Cos.size = 4
Cos.datatype = "range(0,7)"
--Cos.default=0

name = s:option(ListValue, "Servicemode", translate("Internet Name"))
name:value("0", translate("INTERNET"))
name:value("1", translate("OTHER"))
name:value("2", translate("VOIP"))
name:value("4", translate("INTERNET_VOIP"))

name:depends("Mode","1")

br_name = s:option(ListValue, "Servicemode_br", translate("Internet Name"))
br_name:value("0", translate("INTERNET"))
br_name:value("1", translate("OTHER"))
br_name:depends("Mode","0")
function br_name.cfgvalue(self,s)
	local value = self.map:get(s,"Servicemode")
	return value
end
function br_name.write(self,s,value)
	 return self.map:set(s,"Servicemode",value)
end

interface = s:option(ListValue, "Mode", translate("Service Mode"))
interface:value("0", translate("Bridge"))
interface:value("1", translate("Route"))

-- dhcp passthrough function of internet bridge 
local passdhcp= s:option(Flag, "PassDhcp", translate("Enable DHCP Passthrough"))
passdhcp:depends({Mode = "0", Servicemode_br = "0"})
passdhcp.rmempty = true

nat = s:option(Flag, "Nat", translate("Enable NAT"))
nat:depends("Mode", "1")
--nat.default="1"

Workmode = s:option(ListValue, "Workmode", translate("Link Type"))
Workmode:value("0", translate("DHCP"))
Workmode:value("1", translate("Static IP"))
Workmode:value("2", translate("PPPoE"))
Workmode:depends("Mode", "1")

static_ip = s:option(Value, "IP", translate("IP Address"))
static_ip.rmempty = false
static_ip.maxlength = 15
static_ip.datatype = "ipaddr"
static_ip:depends("Workmode","1")
--static_ip.default = "192.168.2.1"
function static_ip.validate(self,value,section)
	local valid = true
	local ipaddr = value
	local ipmask = static_mask:formvalue(section)
	local gwaddr = static_gw:formvalue(section)

	valid = check_rang_value(ipaddr,gwaddr,ipmask)

	if valid  ~= true then
		m.message = translate("IP address and gateway are not in the same subnet !")
		return nil , ""
	end
	local b1, b2, b3, b4 = value:match("^(%d+)%.(%d+)%.(%d+)%.(%d+)$")

	b1 = tonumber(b1)
	if b1 > 0 then
		return value
	else
		m.message = translate("IP is Invalid!")
		return nil,""
	end
	
	return value
end

static_mask = s:option(Value, "Netmask", translate("Subnet Mask"))
static_mask.rmempty = false
static_mask.maxlength = 15
static_mask.datatype = "ipaddr"
static_mask:depends("Workmode","1")
--static_mask.default = "255.255.255.0"

static_gw = s:option(Value, "Gateway", translate("Gateway"))
static_gw.rmempty = false
static_gw.maxlength = 15
static_gw.datatype = "ipaddr"
static_gw:depends("Workmode","1")
--static_gw.default = "192.168.2.254"

ppp_account = s:option(Value, "UserName", translate("PPPoE Account"))
ppp_account.rmempty = false
ppp_account:depends("Workmode","2")
--ppp_account.default = "username"

ppp_password = s:option(Value2, "Passwd", translate("PPPoE Password"))
ppp_password.rmempty = false
ppp_password:depends("Workmode","2")
ppp_password.password = true
--ppp_password.default = "passwd"

ppp_redialtime = s:option(Value, "ServerName", translate("Server Name"))
ppp_redialtime.rmempty = true
ppp_redialtime.datatype = "hostname"
ppp_redialtime:depends("Workmode","2")
--ppp_redialtime.default = "servername"

ppp_proxy = s:option(Value, "ProxyNum", translate("Proxy Num"),translate("(1-8)"))
ppp_proxy.rmempty = true
ppp_proxy:depends("Workmode","3")
--ppp_proxy.default = 1
ppp_proxy.size=4
ppp_proxy.placeholder=1
ppp_proxy.datatype = "range(1,8)"

--[[
ppp_dialway= s:option(Flag, "DialWay", translate("Manual DialWay"))
ppp_dialway.rmempty = true
ppp_dialway:depends("Workmode","2")
ppp_dialway:depends("Workmode","3")
ppp_dialway:depends("Workmode","4")
--]]
IdleTime= s:option(Value, "IdleTime", translate("IdleTime"),translate("seconds"))
IdleTime.rmempty = true
IdleTime:depends("Workmode","2")
IdleTime.size=4
--IdleTime.default = 15
IdleTime.datatype = "uinteger"


pridns = s:option(Value, "Dns1", translate("Primary DNS"))
pridns.rmempty = false
pridns.maxlength = 15
pridns.datatype = "ipaddr"
pridns:depends("Workmode",1)

secdns = s:option(Value, "Dns2", translate("Secondary DNS"))
secdns.rmempty = true
secdns.maxlength = 15
secdns.datatype = "ipaddr"
secdns:depends("Workmode",1)
--secdns:depends("dnstype",0)

portbind = s:option(MultiValue, "PortMap", translate("Port Bind"))
portbind.rmempty = true
portbind.orientation = "horizontal"
portbind.size = 4
portbind:value("lan1",translate("LAN 1"))
portbind:value("lan2",translate("LAN 2"))
portbind:value("lan3",translate("LAN 3"))
portbind:value("lan4",translate("LAN 4"))
if has_wifi then
	portbind:value("wlan1",translate("WLAN 1"))
	portbind:value("wlan2",translate("WLAN 2"))
end


function portbind.validate(self,val,section)
	local tpval = {}
	
	self.map.uci:foreach("wanctl", "wanlink", function(s)
		if  s['.name'] ~= section    then
			local lantbl = self:valuelist(s['.name'])
			for i,lanv in ipairs(lantbl) do
			tpval[lanv] = 1
			end
		end
	end)
	
	val = (type(val) == "table") and val or {val}
	for i, value in ipairs(val) do
		if tpval[value] then
			return nil
		end
	end
	
	return MultiValue.validate(self,val)	
end


return m
