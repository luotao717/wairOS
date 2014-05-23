--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_ipport.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
local ip = require "luci.ip"

m = SimpleForm("firewall", translate("IP/Port Filter"))

m.redirect = dsp.build_url("admin", "security", "ipportf")

nam = m:field(Value, "name", translate("Name"))
nam.rmempty = false
nam.default = "0000"
nam.datatype = "hostname"
function nam.validate(self, value,section)
	local valid = true
	uci:foreach("firewall", "ipport_deny", function(section)
		if section.name == value    then
			valid = false
			m.errmessage = translate("IP/PORT Name repeat,please re-enter other")
		end
	end)

	if valid then
		return value
	else
		return nil
	end
end

sipstart = m:field(Value, "src_ipstart", translate("Source IP Start "))

sipstart.maxlength = 15
sipstart.datatype = "ipaddr"
sipstart.default = "192.168.1.1"

sipend = m:field(Value, "src_ipend", translate("Source IP End"))

sipend.maxlength = 15
sipend.datatype = "ipaddr"
sipend.default = "192.168.1.1"
function sipend.validate(self, value,section)
	local sipe = sipend:formvalue(section)
	local flagip =true
	if value then
		if sipe then
		else 
			m.errmessage = translate("End of the source IP has value, the start of source IP can not be empty")
			flagip =false
		end
	else 
		flagip =false
	end
	if flagip then
		return value
	else
		return nil
	end
end

function sipstart.validate(self, value,section)
	local sips = ip.IPv4(value)
	
	local sipe = ip.IPv4(sipend:formvalue(section))
	local dips = ip.IPv4(dipstart:formvalue(section))
	local dipe = ip.IPv4(dipend:formvalue(section))
	
	local sports = sports:formvalue(section)
	local sporte = sporte:formvalue(section)
	local dports = dports:formvalue(section)
	local dporte = dporte:formvalue(section)
	
	local valid = true
      if sipe and sips then
      	  if sips:higher(sipe) then
      		valid = false
      		m.message = translate("Soure IP Start is greater than Soure IP End ")
      	  end
      end

      	if dips and dipe then
		if dips:higher(dipe) then
			valid  = false
	   	 	m.message = translate("Dest IP Start is greater than Dest IP end ")
		end
	end

	--[[
	uci:foreach("firewall", "ipport_deny", function(section)
		local sipso1 = section.src_ipstart
		local sipeo1 = section.src_ipend
		local dipso1 = section.dest_ipstart
		local dipeo1 = section.dest_ipend
		
		local sipso = ip.IPv4(sipso1)
		local sipeo =ip.IPv4(sipeo1)
		local dipso = ip.IPv4(dipso1)
		local dipeo = ip.IPv4(dipeo1)
	
		if  sips:equal(sipso) and sipe:equal(sipeo) and  dips:equal(dipso) and dipe:equal(dipeo) and section.src_portstart == sports  and section.src_portend ==sporte and section.dest_portstart == dports and section.dest_portend == dporte then
			valid = false
			m.message = translate("IP/PORT record repeat,please re-enter other ")
		end
	end)
	]]--
	if valid then
		return value
	else

		return nil
	end
end


sports = m:field(Value, "src_portstart", translate("Source Port Start "))
sports.size = 5
sports.maxlength = 5
sports.datatype = "port"
sports.default = "65535"
function sports.validate(self, value,section)

	local valid = true
	local sports =tonumber(value)
	local sporte = tonumber(sporte:formvalue(section))
	local dports = tonumber(dports:formvalue(section))
	local dporte = tonumber(dporte:formvalue(section))
	
	if sports and sporte ~="" then
		if sports>sporte then
      			valid = false
      			m.message = translate("Soure Port Start is greater than Soure Port End ")
      		end
      	end

      	if dports and dporte ~=""then
		if dports > dporte  then
			valid  = false
	    		m.message = translate("Dest Port Start is greater than Soure Port end ")
	    
		end
	end
	

	if valid then
		return value
	else
		return nil,translate("Remote IP Address required")
	end
	
end


sporte= m:field(Value, "src_portend", translate("Source Port End"))
sporte.rmempty=false
sporte.size = 5
sporte.maxlength = 5
sporte.datatype = "port"
sporte.default = "65535"

dipstart = m:field(Value, "dest_ipstart", translate("Dest IP Start"))

dipstart.maxlength = 15
dipstart.datatype = "ipaddr"
dipstart.default = "192.168.1.254"

dipend = m:field(Value, "dest_ipend", translate("Dest IP End "))

dipend.maxlength = 15
dipend.datatype = "ipaddr"
dipend.default = "192.168.1.254"



dports = m:field(Value, "dest_portstart", translate("Dest Port Start "))

dports.size = 5
dports.maxlength = 5
dports.datatype = "port"
dports.default = "65535"

dporte = m:field(Value, "dest_portend", translate("Dest Port End"))

dporte.size = 5
dporte.maxlength = 5
dporte.datatype = "port"
dporte.default = "65535"

protocol = m:field(ListValue, "proto", translate("Protocol"))
protocol:value("TCPUDP", translate("TCP/UDP"))
protocol:value("TCP", "TCP")
protocol:value("UDP", "UDP")
protocol.default = "TCPUDP"


link= m:field(ListValue, "dir", translate("Link"))
link:value("uplink", translate("Uplink"))
link:value("downlink", "Downlink")
link.default = "uplink"
--[[
interface = m:field(ListValue, "iface", translate("Interface"))
interface.rmempty = false
interface.maxlength = 15
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end
--]]
--save as
function m.handle(self, state, data)


	if state == FORM_VALID then
	local new_ipport = uci:section("firewall", "ipport_deny")
	if data.src_ipstart then
		if data.src_ipend then
			data.src_ip= data.src_ipstart ..'-' .. data.src_ipend
		else
			data.src_ip= data.src_ipstart 
		end
	elseif data.src_ipend then
			data.src_ip= data.src_ipend
	end

	 if data.src_portstart then
		if data.src_portend then
			data.src_port= data.src_portstart ..'-' .. data.src_portend
		else
			data.src_port= data.src_portstart
		end
	elseif data.src_portend then
		data.src_port= data.src_portend
	end

	if data.dest_ipstart then
		if data.dest_ipend then
			data.dest_ip= data.dest_ipstart ..'-' .. data.dest_ipend
		else
			data.dest_ip= data.dest_ipstart 
		end
	elseif data.dest_ipend then
			data.dest_ip= data.dest_ipend
	end

	if data.dest_portstart then
		if data.dest_portend then
			data.dest_port= data.dest_portstart ..'-' .. data.dest_portend 
		else
			data.dest_port= data.dest_portstart 
		end
	elseif data.dest_portend then
		data.dest_port= data.dest_portend 
	end

	if  data.name then
  		uci:set("firewall", new_ipport, "name", data.name)
	end
	if data.src_ip then
		uci:set("firewall", new_ipport, "src_ip", data.src_ip)
	end
	if data.src_port then
		uci:set("firewall", new_ipport, "src_port", data.src_port)
	end
	if data.dest_ip then
		uci:set("firewall", new_ipport, "dest_ip", data.dest_ip)
	end
	if data.dest_port then
		uci:set("firewall", new_ipport, "dest_port", data.dest_port)
	end
	if data.src_ipstart then
		uci:set("firewall", new_ipport, "src_ipstart",  data.src_ipstart)
	end
	if data.src_ipend then
		uci:set("firewall", new_ipport, "src_ipend",  data.src_ipend)
	end
	if data.src_portstart then
		uci:set("firewall", new_ipport, "src_portstart", data.src_portstart)
	end
	if data.src_portend then
		uci:set("firewall", new_ipport, "src_portend", data.src_portend)
	end
	if data.dest_ipstart then
		uci:set("firewall", new_ipport, "dest_ipstart", data.dest_ipstart)
	end
	if data.dest_ipend then
		uci:set("firewall", new_ipport, "dest_ipend", data.dest_ipend)
	end
	if data.dest_portstart then
		uci:set("firewall", new_ipport, "dest_portstart", data.dest_portstart)
	end
	if data.dest_portend then
		uci:set("firewall", new_ipport, "dest_portend", data.dest_portend)
	end
	if data.proto then
		uci:set("firewall", new_ipport, "proto", data.proto)
	end
	if data.dir then
  		uci:set("firewall", new_ipport, "dir", data.dir)
  	end
	--[[
  	if data.iface then
		uci:set("firewall", new_ipport, "iface", data.iface)
	end
	]]--
  		uci:save("firewall")
  		
	luci.http.redirect(luci.dispatcher.build_url("admin/security/ipportf"))
	end
end

return m
