--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

module("luci.controller.admin.wireless", package.seeall)
function index()
	local has_wifi = nixio.fs.stat("/etc/config/wireless")

	page = node("admin", "wireless")
	page.target = firstchild()
	page.title  = _("AP Management")
	page.order  = 45 
	page.index  = true

	if has_wifi and has_wifi.size > 0 then
		entry({"admin", "wireless", "wlan"}, cbi("admin_network/wlan"), _("Wifi"), 15)  	
	end
end

