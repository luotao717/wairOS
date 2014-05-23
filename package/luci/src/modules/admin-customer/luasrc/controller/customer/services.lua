--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: services.lua 7017 2011-05-03 22:06:29Z jow $
]]--
module("luci.controller.customer.services", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	
       if nixio.fs.access("/etc/config/ftp") then
	    entry({"customer", "services"}, alias("customer", "services", "normal"), i18n("Services"), 4).index = true
	    entry({"customer", "services", "normal"}, cbi("admin_services/normal",{autoapply=true}), i18n("Normal Use"), 6)
       end
end
