--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_internet.lua 6983 2011-04-13 00:33:42Z soma $
wxb modify : bug of operation :
   1. option60 bug
   2. PortMap bug
   3. Idletime set bug
   4. Workmode bug
   2011.10.14
   屏蔽掉dialway: 应该为demand 方式，默认为
   自动刷新
   2013.2.3
   wifi 支持识别 
]]--

local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local has_entry = {}
local wanid = nil
local wanifac = "eth0"
local ip = require"luci.ip"
local has_wifi = nixio.fs.access("/etc/config/wireless")

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

--arg[1] = arg[1] or ""

m = SimpleForm("wanctl", translate("Internet Settings"))

m.redirect = dsp.build_url("admin", "network", "internet")
--[[
if not m.uci:get(arg[1]) == "wanlink" then
	luci.http.redirect(m.redirect)
	return
end

]]--

--[[
s = m:section(NamedSection, arg[1], "wanlink", translate("Internet Settings"))
s.anonymous = true
s.addremove = false
]]--

local vlan = m:field(Value, "VlanID", translate("VLAN ID"),translate("(-1,1~4093), -1: not set VLAN ID"))
vlan.rmempty = false
vlan.size = 4
vlan.datatype = "range(-1,4093)"

function vlan.validate(self,value,section)
	-- get the value now configuration
	local tvalid = true
	local old_value = uci:get("wanctl",section,"VlanID")
	
	if old_value == value then
		tvalid	= true
	else
		uci:foreach("wanctl", "wanlink", function(section)
			if  section.VlanID == value    then
				tvalid = false
				m.errmessage = translate("ID repeat, please enter another Vlan ID")
			end
		end)
	end
	if value == "0" then
		m.errmessage =translate("0 ID can not be set, please enter another Vlan ID")
		return nil
	end
	if tvalid == false then
		return nil		
	end 
	return Value:validate(value,section)	
end


Cos = m:field(Value, "Cos", translate("802.1p"),nil)
Cos.rmempty = false
Cos.size = 4
Cos.datatype = "range(0,7)"
Cos.default=0


name = m:field(ListValue, "Servicemode", translate("Internet Name"))
name:value("0", translate("INTERNET"))
name:value("1", translate("OTHER"))
name:value("2", translate("VOIP"))
name:value("4", translate("INTERNET_VOIP"))

name:depends("Mode","1")

br_name = m:field(ListValue, "Servicemode_br", translate("Internet Name"))
br_name:value("0", translate("INTERNET"))
br_name:value("1", translate("OTHER"))
br_name:depends("Mode","0")
--[[
function br_name.cfgvalue(self,s)
	local value = uci:get(s,"Servicemode")
	return value
end

function br_name.write(self,s,value)
	os.execute("echo value: " .. value .. " >>/tmp/brtest")
	 return uci:set(s,"Servicemode",value)
end
--]]
interface = m:field(ListValue, "Mode", translate("Service Mode"))
interface:value("0", translate("Bridge"))
interface:value("1", translate("Route"))

-- dhcp passthrough function of internet bridge 
local passdhcp= m:field(Flag, "PassDhcp", translate("Enable DHCP Passthrough"))
passdhcp:depends({Mode = "0", Servicemode_br = "0"})
passdhcp.rmempty = true

nat = m:field(Flag, "Nat", translate("Enable NAT"))
nat:depends("Mode", "1")
nat.rmempty = true
nat.default="1"  --暂时屏蔽掉nat 默认为1功能

Workmode =m:field(ListValue, "Workmode", translate("Link Type"))
Workmode:value("0", translate("DHCP"))
Workmode:value("1", translate("Static IP"))
Workmode:value("2", translate("PPPoE"))
Workmode:depends("Mode", "1")



static_ip = m:field(Value, "IP", translate("IP Address"))
static_ip.rmempty = false
static_ip.maxlength = 15
static_ip.datatype = "ipaddr"
static_ip:depends("Workmode","1")
static_ip.default = "192.168.2.1"
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

static_mask = m:field(Value, "Netmask", translate("Subnet Mask"))
static_mask.rmempty = false
static_mask.maxlength = 15
static_mask.datatype = "ip4mask"
static_mask:depends("Workmode","1")
static_mask.default = "255.255.255.0"
--[[
static_mask:value("255.255.255.0",translate("255.255.255.0"))
static_mask:value("255.255.0.0",translate("255.255.0.0"))
static_mask:value("255.0.0.0",translate("255.0.0.0"))
]]--
static_gw = m:field(Value, "Gateway", translate("Gateway"))
static_gw.rmempty = false
static_gw.maxlength = 15
static_gw.datatype = "ipaddr"
static_gw:depends("Workmode","1")
static_gw.default = "192.168.2.254"

ppp_account = m:field(Value, "UserName", translate("PPPoE Account"))
ppp_account.rmempty = false
ppp_account:depends("Workmode","2")
ppp_account.default = "username"

ppp_password = m:field(Value2, "Passwd", translate("PPPoE Password"))
ppp_password.rmempty = false
ppp_password:depends("Workmode","2")
ppp_password.password = true
ppp_password.default = "passwd"

ppp_redialtime = m:field(Value, "ServerName", translate("Server Name"))
ppp_redialtime.rmempty = true
ppp_redialtime.datatype = "hostname"
ppp_redialtime:depends("Workmode","2")
ppp_redialtime.default = "servername"

--[[
ppp_dialway= m:field(Flag, "DialWay", translate("Manual DialWay"))
ppp_dialway.rmempty = true
ppp_dialway:depends("Workmode","2")
ppp_dialway:depends("Workmode","3")
ppp_dialway:depends("Workmode","4")
--]]
IdleTime= m:field(Value, "IdleTime", translate("Idle Time"),translate("seconds"))
IdleTime.rmempty = true
IdleTime:depends("Workmode","2")
IdleTime.size=4
IdleTime.default = 15
IdleTime.datatype = "uinteger"


pridns = m:field(Value, "Dns1", translate("Primary DNS"))
pridns.rmempty = false
pridns.maxlength = 15
pridns.datatype = "ipaddr"
pridns:depends("Workmode",1)
pridns.default = "192.168.2.254"

secdns = m:field(Value, "Dns2", translate("Secondary DNS"))
secdns.rmempty = true
secdns.maxlength = 15
secdns.datatype = "ipaddr"
secdns:depends("Workmode",1)
secdns.default = "192.168.2.254"

portbind = m:field(MultiValue, "PortMap", translate("Port Bind"))
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
function portbind.valuelist(self, section)
	local val = uci:get("wanctl",section, "PortMap")
	if not(type(val) == "string") then
		return {}
	end
	return luci.util.split(val, self.delimiter)
end
function portbind.validate(self,val,section)
	local tpval = {}
	uci:foreach("wanctl", "wanlink", function(s)
	--	if  s['.name'] ~= section    then
			local lantbl = self:valuelist(s['.name'])
			for i,lanv in ipairs(lantbl) do
			tpval[lanv] = 1
			end
	--	end
	end)
	
	val = (type(val) == "table") and val or {val}
	for i, value in ipairs(val) do
		if tpval[value] then
			m.errmessage = "Port: " .. value .. " has been bound"
			return nil
		end
	end
	
	return MultiValue.validate(self,val)	
end


function m.handle(self, state, data)
	if state == FORM_VALID then


       --cursor:load("firewall")
  	-- uci:section("firewall","mac",nil,{src_mac = data.src_mac, dir = data.dir})
   
       local new_wan = uci:section("wanctl","wanlink")
       
		if data.Mode =="1" then
			uci:set("wanctl", new_wan, "Nat", "1")
		end
  		if data.VlanID then
			uci:set("wanctl", new_wan, "VlanID", data.VlanID)
  		end
  		if data.Cos then
			uci:set("wanctl", new_wan, "Cos", data.Cos)
  		end
  		if data.Servicemode then
			uci:set("wanctl", new_wan, "Servicemode", data.Servicemode)
  		end
  		if data.Servicemode_br then
			uci:set("wanctl", new_wan, "Servicemode", data.Servicemode_br)
  		end

  		if data.PassDhcp then
			uci:set("wanctl", new_wan, "PassDhcp", data.PassDhcp)
  		end
  		if data.Nat then  
			uci:set("wanctl", new_wan, "Nat", data.Nat)
  		end
  		 if data.Mode then
			uci:set("wanctl", new_wan, "Mode", data.Mode)
  		end
  		if data.Workmode then
			uci:set("wanctl", new_wan, "Workmode", data.Workmode)
		--[[  --wxb modify -- delete the pppoe mode for nat not enable
			-- pppoe mix and pppoe proxy only use for nat
			if tonumber(data.Workmode) >= 3 then 
			   	uci:set("wanctl", new_wan, "Nat", 1)	
			end
		--]]
  		end
  		
  		if data.Option60 then
			if type(data.Option60)== "table" and #data.Option60 > 0 then
				uci:set("wanctl", new_wan, "Option60", data.Option60)
			end
  		end
  		
  		if data.IP then
			uci:set("wanctl", new_wan, "IP", data.IP)
  		end
  		if data.Netmask then
			uci:set("wanctl", new_wan, "Netmask", data.Netmask)
  		end
  		if data.Gateway then
			uci:set("wanctl", new_wan, "Gateway", data.Gateway)
  		end

  		if data.UserName then
			uci:set("wanctl", new_wan, "UserName", data.UserName)
  		end
  		if data.Passwd then
			uci:set("wanctl", new_wan, "Passwd", data.Passwd)
  		end
  		if data.ServerName then
			uci:set("wanctl", new_wan, "ServerName", data.ServerName)
  		end
  		if data.ProxyNum then
			uci:set("wanctl", new_wan, "ProxyNum", data.ProxyNum)
  		end
  	--[[
  		if data.DialWay then
			uci:set("wanctl", new_wan, "DialWay", data.DialWay)
  		end
  	--]]
  		if data.IdleTime then
			uci:set("wanctl", new_wan, "IdleTime", data.IdleTime)
  		end
  		if data.Dns1 then
			uci:set("wanctl", new_wan, "Dns1", data.Dns1)
  		end
  		if data.Dns2 then
			uci:set("wanctl", new_wan, "Dns2", data.Dns2)
  		end

  		 if data.PortMap then
			uci:set("wanctl", new_wan, "PortMap", data.PortMap)
  		end

  		uci:save("wanctl")
        luci.http.redirect(luci.dispatcher.build_url("admin/network/internet"))

	
	end
end


return m
