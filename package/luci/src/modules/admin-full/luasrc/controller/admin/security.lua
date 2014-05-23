--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: security.lua 7017 2011-05-03 22:06:29Z jow $
]]--
module("luci.controller.admin.security", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	if not nixio.fs.access("/etc/config/firewall") then
	   return 
	end

	entry({"admin", "security"}, alias("admin", "security", "firewall"), i18n("Security"), 3).index = true

	entry({"admin", "security", "firewall"}, cbi("luci_fw/firewall"), i18n("Firewall"), 1)
	
	entry({"admin", "security", "urlf"}, alias("admin", "security", "urlf", "zones"), i18n("Url Filter"), 2)
	entry({"admin", "security", "urlf", "zones"}, arcombine(cbi("luci_fw/zones_url"), cbi("luci_fw/zones_url")), nil, 10).leaf = true
	entry({"admin", "security", "urlf", "rule1"}, arcombine(cbi("luci_fw/trule_url1"), cbi("luci_fw/trule_url1")), nil, 20).leaf = true
	entry({"admin", "security", "urlf", "rule2"}, arcombine(cbi("luci_fw/trule_url2"), cbi("luci_fw/trule_url2")), nil, 30).leaf = true

	entry({"admin", "security", "macf"}, alias("admin", "security", "macf", "zones"), i18n("MAC Filter"), 3)
	entry({"admin", "security", "macf", "zones"}, arcombine(cbi("luci_fw/zones_mac"), cbi("luci_fw/zones_mac")), nil, 10).leaf = true
	entry({"admin", "security", "macf", "rule1"}, arcombine(cbi("luci_fw/trule_mac1"), cbi("luci_fw/trule_mac1")), nil, 20).leaf = true
	entry({"admin", "security", "macf", "rule2"}, arcombine(cbi("luci_fw/trule_mac2"), cbi("luci_fw/trule_mac2")), nil, 30).leaf = true
	
	entry({"admin", "security", "ipportf"}, alias("admin", "security", "ipportf", "zones"), i18n("IP/Port Filter"), 4)
	entry({"admin", "security", "ipportf", "zones"}, arcombine(cbi("luci_fw/zones_ipport"), cbi("luci_fw/zones_ipport")), nil, 10).leaf = true
	entry({"admin", "security", "ipportf", "rule1"}, arcombine(cbi("luci_fw/trule_ipport1"), cbi("luci_fw/trule_ipport1")), nil, 20).leaf = true
	entry({"admin", "security", "ipportf", "rule2"}, arcombine(cbi("luci_fw/trule_ipport2"), cbi("luci_fw/trule_ipport2")), nil, 30).leaf = true

	if nixio.fs.access("/etc/config/uhttpd") then
		entry({"admin", "security", "remoteMgmt"}, cbi("admin_security/remoteMgmt"), i18n("Remote Management"), 10)
	end
end
