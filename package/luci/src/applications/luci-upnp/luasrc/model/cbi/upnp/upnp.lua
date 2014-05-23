--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: upnp.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local ds = require "luci.dispatcher"
m = Map("upnpd", translate("UPNP Config"), "")
m.redirect = ds.build_url("admin", "services", "upnp")

s = m:section(NamedSection, "config","upnpd", translate("UPNP Settings"))
s.anonymous = true
s.addremove = false

enupnp=s:option(Flag, "enabled", translate("Enable UPNP"))
enupnp.rmempty = false
return m
