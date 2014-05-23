--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: network.lua 7017 2011-05-03 22:06:29Z jow $
]]--
module("luci.controller.admin.network", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	local has_wifi = nixio.fs.stat("/etc/config/wireless")
	local has_acs = nixio.fs.stat("/etc/config/remote")	-- tr069 support
	
	
	if nixio.fs.access("/etc/config/wanctl") then
		entry({"admin", "network"}, alias("admin", "network", "internet"), i18n("Network"), 2).index = true

		entry({"admin", "network", "internet"}, alias("admin", "network", "internet","zones"), i18n("Broadband Settings"), 1)	
		entry({"admin", "network", "internet", "zones"}, arcombine(cbi("admin_network/zones_internet"), cbi("admin_network/zones_internet")), nil, 10).leaf = true
		entry({"admin", "network", "internet", "rule"}, arcombine(cbi("admin_network/trule_internet"), cbi("admin_network/trule_internet")), nil, 20).leaf = true
		entry({"admin", "network", "internet", "third"}, arcombine(cbi("admin_network/third_internet"), cbi("admin_network/third_internet")), nil, 30).leaf = true
	else
		entry({"admin", "network"}, alias("admin", "network", "lan"), i18n("Network"), 2).index = true
		entry({"admin", "network", "internet"}, cbi("admin_network/network_wan",{autoapply=true}), i18n("Broadband Settings"), 3)
		entry({"admin", "network", "dhcp"}, cbi("admin_network/dhcp_lan",{autoapply=true}), i18n("DHCP"), 3)
	end
--	entry({"admin", "network", "lan"}, cbi("admin_network/lan",{autoapply=true}), i18n("LAN Settings"), 3)
	entry({"admin", "network", "lan"}, cbi("admin_network/network_lan",{autoapply=true}), i18n("LAN Settings"), 3)

	if nixio.fs.access("/etc/config/qos") then	
		entry({"admin", "network", "qos"}, alias("admin", "network", "qos", "zones"), i18n("QoS"),4)
		entry({"admin", "network", "qos", "zones"}, arcombine(cbi("admin_network/qos"), cbi("admin_network/qos")), nil, 10).leaf = true
		entry({"admin", "network", "qos", "rule"}, arcombine(cbi("admin_network/qosf"), cbi("admin_network/qosf")), nil, 20).leaf = true
	
		entry({"admin", "network", "qos", "third"}, arcombine(cbi("admin_network/third_qos"), cbi("admin_network/third_qos")), nil, 30).leaf = true
		entry({"admin", "network", "qos", "fouth"}, arcombine(cbi("admin_network/fouth_qos"), cbi("admin_network/fouth_qos")), nil, 40).leaf = true
	
		entry({"admin", "network", "qos", "five"}, arcombine(cbi("admin_network/five_qos"), cbi("admin_network/five_qos")), nil, 50).leaf = true
		entry({"admin", "network", "qos", "six"}, arcombine(cbi("admin_network/six_qos"), cbi("admin_network/six_qos")), nil, 60).leaf = true
	
		entry({"admin", "network", "qos", "seven"}, arcombine(cbi("admin_network/seven_qos"), cbi("admin_network/seven_qos")), nil, 70).leaf = true
		entry({"admin", "network", "qos", "eight"}, arcombine(cbi("admin_network/eight_qos"), cbi("admin_network/eight_qos")), nil, 80).leaf = true
   		entry({"admin", "network", "qos", "nine"}, arcombine(cbi("admin_network/nine_qos"), cbi("admin_network/nine_qos")), nil, 90).leaf = true
   	end
	
	if has_wifi then
		entry({"admin", "network", "wlan"}, cbi("admin_network/wlan"), i18n("WLAN"), 5) 
	end

	if has_acs then
		entry({"admin", "network", "remote"}, cbi("admin_network/remote"), i18n("Remote Control"), 6)
	end 
	
	--entry({"admin", "network", "router"}, alias("admin", "network", "router", "zones"), i18n("Routes"), 9)
	--entry({"admin", "network", "router", "zones"}, arcombine(cbi("admin_network/zones_router"), cbi("admin_network/zones_router")), nil, 10).leaf = true
	--entry({"admin", "network", "router", "rule"}, arcombine(cbi("admin_network/trule_router"), cbi("admin_network/trule_router")), nil, 20).leaf = true
	--entry({"admin", "network", "router", "third"}, arcombine(cbi("admin_network/third_router"), cbi("admin_network/third_router")), nil, 30).leaf = true
end
