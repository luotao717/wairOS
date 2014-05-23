--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: remote.lua 6065 2010-04-14 11:36:13Z ben $
]]--
m = Map("remote", translate("Remote Control"), "")

s = m:section(TypedSection, "tr069", translate("TR069 Settings"))
s.anonymous = true
s.addremove = false

s:option(Flag, "enable", translate("TR069 Status")).rmempty = true

s23 = s:option(Value, "url", translate("ACS URL"))
s23:depends("enable",1)
function s23.cfgvalue(self,s)
     return self.map:get(s,"url") 
end


s24 = s:option(Value, "username", translate("ACS Username"))
s24:depends("enable",1)
function s24.cfgvalue(self,s)
     return self.map:get(s,"username") 
end

s25 = s:option(Value, "password", translate("ACS Password"))
s25:depends("enable",1)
function s25.cfgvalue(self,s)
     return self.map:get(s,"password") 
end

s21 = s:option(Flag, "periodic_inform_enable", translate("Enable Periodic Inform"))
s21:depends("enable",1)

s22 = s:option(Value, "periodic_inform_interval", translate("Periodic Inform Interval"))
s22.size = 5
s22.datatype = "uinteger"
s22:depends("enable",1)
function s22.cfgvalue(self,s)
     return self.map:get(s,"periodic_inform_interval") 
end

s26 = s:option(Value, "connection_request_username", translate("CPE Username"))
s26:depends("enable",1)
function s26.cfgvalue(self,s)
     return self.map:get(s,"connection_request_username") 
end

s27 = s:option(Value, "connection_request_password", translate("CPE Password"))
s27:depends("enable",1)
function s27.cfgvalue(self,s)
     return self.map:get(s,"connection_request_password") 
end
--s:option(Flag, "ce_enable", translate("Certificate")).rmempty = true
ca_cert= s:option(FileUpload, "ca_cert", translate("Path to CA-Certificate"))
ca_cert:depends("enable",1)

s1 = m:section(TypedSection, "stun", translate("STUN Settings"))
s1.anonymous = true
s1.addremove = false

s1:option(Flag, "enable", translate("STUN Status")).rmempty = true

s11 = s1:option(Value, "server_address", translate("STUN Server"))
s11.rmempty = true
s11.datatype = "hostname"
s11:depends("enable",1)

s12 = s1:option(Value, "server_port", translate("STUN Server Port"))
s12.rmempty = true
s12.size = 5
s12.maxlength = 5
s12.datatype = "port"
s12:depends("enable",1)

s13 = s1:option(Value, "min_keep_alive_period", translate("Min Keep Alive Period"))
s13.rmempty = true
s13.size = 5
s13.datatype = "uinteger"
s13:depends("enable",1)

s14 = s1:option(Value, "username", translate("STUN Username"))
s14.rmempty = true
s14:depends("enable",1)

s15 = s1:option(Value, "password", translate("STUN Password"))
s15.rmempty = true
s15:depends("enable",1)

return m
