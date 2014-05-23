--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: security.lua 7017 2011-05-03 22:06:29Z jow $
]]--
module("luci.controller.customer.security", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	if not nixio.fs.access("/etc/config/firewall") then
	   return 
	end

	entry({"customer", "security"}, alias("customer", "security", "firewall"), i18n("Security"), 3).index = true

	entry({"customer", "security", "firewall"}, cbi("customer/firewall"), i18n("Firewall"), 1)
	
	entry({"customer", "security", "urlf"}, alias("customer", "security", "urlf", "zones"), i18n("Url Filter"), 2)
	entry({"customer", "security", "urlf", "zones"}, arcombine(cbi("customer/zones_url"), cbi("customer/zones_url")), nil, 10).leaf = true
	entry({"customer", "security", "urlf", "rule1"}, arcombine(cbi("customer/trule_url1"), cbi("customer/trule_url1")), nil, 20).leaf = true
	entry({"customer", "security", "urlf", "rule2"}, arcombine(cbi("customer/trule_url2"), cbi("customer/trule_url2")), nil, 30).leaf = true
	
	entry({"customer", "security", "macf"}, alias("customer", "security", "macf", "zones"), i18n("MAC Filter"), 3)
	entry({"customer", "security", "macf", "zones"}, arcombine(cbi("customer/zones_mac"), cbi("customer/zones_mac")), nil, 10).leaf = true
	entry({"customer", "security", "macf", "rule1"}, arcombine(cbi("customer/trule_mac1"), cbi("customer/trule_mac1")), nil, 20).leaf = true
	entry({"customer", "security", "macf", "rule2"}, arcombine(cbi("customer/trule_mac2"), cbi("customer/trule_mac2")), nil, 30).leaf = true
end
