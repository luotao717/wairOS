--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wifidog.lua 5448 2009-10-31 15:54:11Z jow $
]]--
module("luci.controller.wifidog", package.seeall)

function index()
	require("luci.i18n")
	luci.i18n.loadc("wifidog")
	if not nixio.fs.access("/etc/config/authserver.conf") then
		return
	end
	
	local page = entry({"admin", "network", "wifidog"}, cbi("wifidog/wifidog"), luci.i18n.translate("wifidog"), 50)
	page.i18n = "wifidog"
	page.dependent = true
	
end
