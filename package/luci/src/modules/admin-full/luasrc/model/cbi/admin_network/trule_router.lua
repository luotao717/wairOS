--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_router.lua 6983 2011-04-13 00:33:42Z soma $
]]--

--local has_v2 = nixio.fs.access("/lib/network/router.sh")
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
local ip = require "luci.ip"
local bit  = nixio.bit

m = SimpleForm("routes", translate("Router List"))

m.redirect = dsp.build_url("admin", "network", "router")

dip = m:field(Value, "target", translate("Dest IP"))
dip.rmempty = false
dip.maxlength = 15
dip.datatype = "ipaddr"
function dip.validate(self, value, section)

		local valid = true

		local fvalue = gw_en:formvalue(section)
		local fvalue1 = interface_en:formvalue(section)
		local fvalue2 = gw:formvalue(section)
		local fvalue3 = interface:formvalue(section)
		local netm = uci:get("network","lan","netmask")
		local ipaddr = uci:get("network","lan","ipaddr")
		
		local dip1 = ip.IPv4(value)
		local ipadd = ip.IPv4(ipaddr,netm)


		local iphostmin = ipadd:minhost()
		local iphostmax = ipadd:maxhost()
		
		if fvalue then
			local gwip = ip.IPv4(fvalue2,netm)
			if tostring(fvalue2)=="" then
				valid = false
				m.message = translate("Gateway IP address is empty, please input")
			else
				--check gateway ip and router ip
				
				local gwip = ip.IPv4(fvalue2,netm)
				if ipadd:network():equal(gwip:network()) == true then
					valid = false
					m.message = translate("Gateway IP and the router LAN IP are in the same subnet,please change")
				end
			
			end
		end
		
		if dip1:higher(iphostmin) and dip1:lower(iphostmax) then
			valid = false
			m.message = translate("Staic IP is in the same subnet as the router IP, please adjust")
		end
		
		if fvalue1 then
			if tostring(fvalue3)=="" then
				valid = false
				m.message = translate("Interface is empty, please input")
			end
		end
		if fvalue or fvalue1 then
		else
			valid = false
			m.message = translate("Enable gateway IP address and enable the interface to select at least one")
		end
		
		uci:foreach("routes", "route", function(section)
		local ipo = section.target 
		local ipo1 = ip.IPv4(ipo)
		 if dip1:equal(ipo1) and fvalue3 == section.interface then
				valid = false
				m.message = translate("IP record repeat,please re-enter other IP")
			end
		end)
		
		if valid then
			return value
		else
			return nil
		end

end


nm = m:field(Value, "netmask", translate("Subnet Mask"))
nm.rmempty = false
nm.maxlength = 15
nm.datatype = "ip4bitmask"
nm:value("255.255.255.0",translate("255.255.255.0"))
nm:value("255.255.0.0",translate("255.255.0.0"))
nm:value("255.0.0.0",translate("255.0.0.0"))

function nm.validate(self, value, section)
	local dipv = dip:formvalue(section)
	local ipaddr = uci:get("network","lan","ipaddr")
	local netm = uci:get("network","lan","netmask")
	local ipadd = ip.IPv4(ipaddr,netm)
	local dip_ip = ip.IPv4(dipv)
	local ip_net = ipadd:network()

	local valid = true
	
	local b1, b2, b3, b4 = value:match("^(%d+)%.(%d+)%.(%d+)%.(%d+)$")
	local c1, c2, c3, c4 = dipv:match("^(%d+)%.(%d+)%.(%d+)%.(%d+)$")
	

       local dipn = 0
       local nmn = 0
	b1 = tonumber(b1)
	b2 = tonumber(b2)
	b3 = tonumber(b3)
	b4 = tonumber(b4)

	if b1 and b1 <= 255 and
	   b2 and b2 <= 255 and
	   b3 and b3 <= 255 and
	   b4 and b4 <= 255 
	
	then
		nmn = b1 * 256*256*256 + b2*256*256 + b3 * 256 + b4
		
	end
	c1 = tonumber(c1)
	c2 = tonumber(c2)
	c3 = tonumber(c3)
	c4 = tonumber(c4)

	if c1 and c1 <= 255 and
	   c2 and c2 <= 255 and
	   c3 and c3 <= 255 and
	   c4 and c4 <= 255 
	then
		dipn= c1 * 256*256*256 + c2* 256*256+ c3 * 256 + c4 
		
	end
	local nmnf = bit.bnot(nmn)
--check staic routing ip and netmask
	if bit.band(nmnf,dipn) ~=0 or dip_ip:equal(ip_net) then
		m.message = translate("Static routing IP address is invalid, please adjust")
		valid = false
	end
	if valid then
		return value
	else
		return nil
	end
end


gw_en = m:field(Flag, "gw_en", translate("Enable Gateway"))
gw_en.rmempty = false


gw = m:field(Value, "gateway", translate("Gateway"))
gw.rmempty = true
gw.maxlength = 15
gw.datatype = "ipaddr"
gw:depends("gw_en",1)



interface_en = m:field(Flag, "interface_en", translate("Enable Interface"))
interface_en.rmempty = false

interface = m:field(ListValue, "interface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end

--interface:value("0", translate("1_INTERNET_R_VID_2"))
--interface:value("1", translate("2_TR069_R_VID_3"))
interface.rmempty = true
interface:depends("interface_en",1)

--save as
function m.handle(self, state, data)
	if state == FORM_VALID then
		local new_route = uci:section("routes", "route")
		if data.target then
			uci:set("routes", new_route, "target", data.target)
		end
		if data.netmask then
			uci:set("routes", new_route, "netmask", data.netmask)
		end
		if data.gw_en then
			uci:set("routes", new_route, "gw_en", data.gw_en)
		end
		if data.gateway then
			uci:set("routes", new_route, "gateway", data.gateway)
  		end
		
		if data.interface_en then
			uci:set("routes", new_route, "interface_en", data.interface_en)
		end
		if data.interface then
			uci:set("routes", new_route, "interface", data.interface)
  		end
		uci:save("routes")
		luci.http.redirect(luci.dispatcher.build_url("admin/network/router"))
	end
end

return m
