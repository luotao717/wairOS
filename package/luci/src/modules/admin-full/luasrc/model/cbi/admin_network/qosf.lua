--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: qosf.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local ds = require "luci.dispatcher"
m = Map("qos", translate("Qos Class"))

s = m:section(TypedSection, "App", translate("AppName"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
--s.extedit   = ds.build_url("admin", "network", "qos", "fouth", "%s")

function s.create(self, section)
	--local created = TypedSection.create(self, section) 
	--m.uci:save("network")
	luci.http.redirect(ds.build_url(
		"admin", "network", "qos", "third", created
	))
	return
end

appname = s:option(DummyValue, "AppName", translate("Mode"))


ClassQueue = s:option(DummyValue, "ClassQueue", translate("ClassQueue"))


s = m:section(TypedSection, "Classification", translate("QoS List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_qos1"
--s.extedit   = ds.build_url("admin", "network", "qos", "six", "%s")
s.extedit1   = ds.build_url("admin", "network", "qos", "seven", "%s")
function s.create(self, section)
	--local created = TypedSection.create(self, section)
	--m.uci:save("network")
	luci.http.redirect(ds.build_url(
		"admin", "network", "qos", "five", created
	))
	return
end

Queue = s:option(DummyValue, "ClassQueue", translate("ClassQueue")) 

DSCPMarkValue = s:option(DummyValue, "DSCPMarkValue", translate("DSCPMarkValue"))
 

 P_Value = s:option(DummyValue, "P_Value", translate("802.1P"))
 

return m
