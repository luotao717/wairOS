--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: network.lua 5485 2009-11-01 14:24:04Z jow $
]]--

module("luci.controller.customer.network", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
        local has_wifi = nixio.fs.stat("/etc/config/wireless")
    if has_wifi then
	entry({"customer", "network"}, alias("customer", "network", "wifi"), i18n("Network"), 2).index = true
	entry({"customer", "network", "wifi"}, cbi("customer_network/wlan"), i18n("Wlan"), 1)
    end
end
