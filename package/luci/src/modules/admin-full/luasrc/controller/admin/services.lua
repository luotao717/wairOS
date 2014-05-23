--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: services.lua 7017 2011-05-03 22:06:29Z jow $
]]--
module("luci.controller.admin.services", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	
	entry({"admin", "services"}, alias("admin", "services", "ddns"), i18n("Services"), 4).index = true

	entry({"admin", "services", "advanced_nat"}, cbi("luci_fw/advanced_nat"), i18n("Advanced NAT"), 3)
	entry({"admin", "services", "portfwd"}, alias("admin", "services", "portfwd", "zones"), i18n("Port Forwarding"), 4)
	entry({"admin", "services", "portfwd", "zones"}, arcombine(cbi("luci_fw/zones_portfwd"), cbi("luci_fw/zones_portfwd")), nil, 10).leaf = true
	entry({"admin", "services", "portfwd", "rule"}, arcombine(cbi("luci_fw/trule_portfwd"), cbi("luci_fw/trule_portfwd")), nil, 20).leaf = true
	entry({"admin", "services", "portfwd", "third"}, arcombine(cbi("luci_fw/third_portfwd"), cbi("luci_fw/third_portfwd")), nil, 30).leaf = true

       if nixio.fs.access("/etc/config/igmp") then
	   entry({"admin", "services", "igmp"}, cbi("admin_services/igmp"), i18n("IGMP"), 5)
	end	
	
       if nixio.fs.access("/etc/config/ftpclient") then
	    entry({"admin", "services", "normal"}, cbi("admin_services/normal"), i18n("Normal Use"), 6)
       end

      if nixio.fs.stat("/etc/config/voip") then
        entry({"admin", "services", "voip"}, alias("admin", "services", "voip", "zones"), i18n("VOIP"), 2)
	   entry({"admin", "services", "voip", "zones"}, arcombine(cbi("admin_services/zones_voip"), cbi("admin_services/zones_voip")), nil, 10).leaf = true
	   entry({"admin", "services", "voip", "rule"}, arcombine(cbi("admin_services/trule_voip"), cbi("admin_services/trule_voip")), nil, 20).leaf = true
	   entry({"admin", "services", "voip", "third"}, arcombine(cbi("admin_services/third_voip"), cbi("admin_services/third_voip")), nil, 30).leaf = true
	   entry({"admin", "services", "voip", "forth"}, arcombine(cbi("admin_services/forth_voip"), cbi("admin_services/forth_voip")), nil, 40).leaf = true
	 -- entry({"admin", "services", "voip"}, cbi("admin_services/voip"), i18n("VOIP"), 7)
      end
end
