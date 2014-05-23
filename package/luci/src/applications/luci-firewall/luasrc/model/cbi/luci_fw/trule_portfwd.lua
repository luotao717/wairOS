--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_portfwd.lua 6983 2011-04-13 00:33:42Z soma $
]]--

--local has_v2 = nixio.fs.access("/lib/services/portmir.sh")
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local wanlnk= require "luci.model.wanlink".init()
local ip = require "luci.ip"

m = SimpleForm("firewall", translate("Port Forwarding"))


m.redirect = dsp.build_url("admin", "services", "portfwd")

nam = m:field(Value, "name", translate("Name"))
nam.rmempty = false
nam.default = "0000"
nam.datatype = "hostname"
function nam.validate(self, value,section)
	local valid = true
	uci:foreach("firewall", "redirect", function(section)
		if section.name == value    then
			valid = false
			m.errmessage = translate("Portfwd Name repeat,please re-enter other")
		end
	end)

	if valid then
		return value
	else
		return nil
	end
end


internalip = m:field(Value, "dest_ip", translate("Internal IP"))
internalip.rmempty = false
internalip.maxlength = 15
internalip.datatype = "ipaddr"
internalip.default="192.168.16.255"

function internalip.validate(self, value,section)
	local valid = true

	local privatePort = privatePort:formvalue(section)

	local publicPort = publicPort:formvalue(section)

	local dest = ip.IPv4(internalip:formvalue(section))
	local src = ip.IPv4(externalip:formvalue(section))
	
	
	uci:foreach("firewall", "redirect", function(section)
		local dest1 = section.dest_ip
		local src1 = section.src_ip

		local dest2 = ip.IPv4(dest1)
		local src2 =ip.IPv4(src1)
		
		if dest:equal(dest2) and  section.dest_port ==privatePort and src:equal(src2)and section.src_dport ==publicPort then 
			valid = false
			m.message = translate("Portfwd record repeat,please re-enter other ")
		end
	end)
	if valid then
		return value
	else
		return nil
	end
end 

privatePort = m:field(Value, "dest_port", translate("Internal Port"))
privatePort.rmempty = false
privatePort.size = 15
privatePort.maxlength = 15
privatePort.datatype = "port"


protocol = m:field(ListValue, "proto", translate("Protocol"))
protocol:value("TCPUDP", translate("TCP/UDP"))
protocol:value("TCP", "TCP")
protocol:value("UDP", "UDP")

externalip = m:field(Value, "src_ip", translate("Remote IP"))
externalip.rmempty = true
externalip.maxlength = 15
--externalip.datatype = "ipaddr"
externalip.default="192.168.1.1"

publicPort = m:field(Value, "src_dport", translate("External Port"))
publicPort.rmempty = false
publicPort.size = 15
publicPort.maxlength = 15
publicPort.datatype = "port"


interface = m:field(ListValue, "iface", translate("Interface"))
for o,v in pairs(wanlnk.waninfo_get()) do
	interface:value(v.Interface,translate(v.ConnName))
end


status = m:field(ListValue, "state", translate("Status"))
status:value("0", translate("Disable"))
status:value("1", translate("Enable"))
status.default="1"

function m.handle(self, state, data)
	if state == FORM_VALID then   
       local new_portfwd = uci:section("firewall","redirect")
       	if data.name then
			uci:set("firewall", new_portfwd, "name", data.name)
		end
		if data.dest_ip then
  			uci:set("firewall",new_portfwd,"dest_ip", data.dest_ip)
  		end
  		if data.dest_port then
  			uci:set("firewall",new_portfwd,"dest_port", data.dest_port)
  		end
  		if data.proto then
  			uci:set("firewall",new_portfwd, "proto", data.proto)
  		end
  		if data.src_ip then
  			uci:set("firewall",new_portfwd,"src_ip", data.src_ip)
  		end
  		if data.src_dport then
  			uci:set("firewall",new_portfwd,"src_dport", data.src_dport)
  		end
  		if data.iface then
  			uci:set("firewall",new_portfwd,"iface", data.iface)
  		end  
  		uci:set("firewall",new_portfwd,"state", data.state)
  	 	uci:save("firewall")

        luci.http.redirect(luci.dispatcher.build_url("admin/services/portfwd"))
	end
	
end

return m
