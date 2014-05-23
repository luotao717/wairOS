--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: status.lua 7025 2011-05-04 21:23:55Z jow $
]]--
module("luci.controller.customer.status", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	local has_wifi = nixio.fs.stat("/etc/config/wireless")
	local page

	entry({"customer", "status"}, alias("customer", "status", "index"), i18n("Status"), 1).index = true
	entry({"customer", "status", "index"}, template("admin_status/index"), i18n("Status"), 1)	
	entry({"customer", "status", "wan"}, template("customer_status/wan"), i18n("WAN Status"), 2)
	entry({"customer", "status", "lan"}, template("customer_status/lan"), i18n("LAN Status"), 3)
	entry({"customer", "status", "lan"}, template("customer_status/lan"), i18n("LAN Information"), 3)
	page = entry({"customer", "status", "iface_status"}, call("iface_status"), nil)
	page.leaf = true
	page = entry({"customer", "status", "iface_reconnect"}, call("iface_reconnect"), nil)
	page.leaf = true

	page = entry({"customer", "status", "iface_shutdown"}, call("iface_shutdown"), nil)
	page.leaf = true

	page = entry({"customer", "status", "iface_wanlink_status"}, call("iface_wanlink_status"), nil)
	page.leaf = true

	page = entry({"customer", "status", "autolog"}, call("autolog_status"), nil)
	page.leaf = true
	

end

function autolog_status()
	local rv   = { }
	
	local data ={
		status = 1  
	}
	if next(data) then
		rv[#rv+1] = data
	end
	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "Error auto logout Status Parse")
end

function iface_wanlink_status()
	local path = luci.dispatcher.context.requestpath
	local netm = require "luci.model.wanlink".init()
	local uci = require "luci.model.uci".cursor()
	local rv   = { }

	local iface
	for iface in path[#path]:gmatch("[%w%.%-_]+") do
		local net = netm.waninfo_get_extern(iface)
		if net then
			local info
			local dev  = net.Interface
			local data = {
				id       = net.ConnName,
				proto    = net.Protocol,
				ifname = net.Interface,
				status  = net.Status,
				connname = net.ConnName,
				ipaddr   = net.IP,
				netmask = net.netmask or "-",
				gateway = net.gateway or "-",
				pridns 	= net.pridns or "-",
				secdns    = net.secdns or "-"				
			}
			-- get the message of wanctl interface

			if next(data) then
				rv[#rv+1] = data
			end
		end
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

function iface_status()
	local path = luci.dispatcher.context.requestpath
	local netm = require "luci.model.network".init()
	local uci = require "luci.model.uci".cursor()

	local rv   = { }

	

	local iface
	for iface in path[#path]:gmatch("[%w%.%-_]+") do
	      -- lan interface set
		local net = netm:get_network(iface)
		if net then
			local info
			local dev  = net:ifname()
			local data = {
				id       = iface,
				proto    = net:proto(),
				uptime   = net:uptime(),
				gwaddr   = net:gwaddr(),
				dnsaddrs = net:dnsaddrs(),
				ifname = dev
			}
			for _, info in ipairs(nixio.getifaddrs()) do
				local name = info.name:match("[^:]+")
				if name == dev then
					if info.family == "packet" then
						data.flags   = info.flags
						data.stats   = info.data
						data.macaddr = info.addr
				--		data.ifname  = name
					elseif info.family == "inet" then
						data.ipaddrs = data.ipaddrs or { }
						data.ipaddrs[#data.ipaddrs+1] = {
							addr      = info.addr,
							broadaddr = info.broadaddr,
							dstaddr   = info.dstaddr,
							netmask   = info.netmask,
							prefix    = info.prefix
						}
					elseif info.family == "inet6" then
						data.ip6addrs = data.ip6addrs or { }
						data.ip6addrs[#data.ip6addrs+1] = {
							addr    = info.addr,
							netmask = info.netmask,
							prefix  = info.prefix
						}
					end
				end
			end

			if next(data) then
				rv[#rv+1] = data
			end
		else
			-- wlan interface set
			
			   
				local info
				--local dev  = net:name()
				local data = {
					id       = "wifi0",
					proto    = "none", --net:proto(),
					uptime   = 0, --net:uptime(),
				--	gwaddr   = net:gwaddr(),
				--	dnsaddrs = net:dnsaddrs(),
					ifname = "wifi0"
				}
				for _, info in ipairs(nixio.getifaddrs()) do
					local name = info.name:match("[^:]+")
					if name == "wifi0" then
						if info.family == "packet" then
							data.flags   = info.flags
							data.stats   = info.data
							data.macaddr = info.addr
					--		data.ifname  = name
						elseif info.family == "inet" then
							data.ipaddrs = data.ipaddrs or { }
							data.ipaddrs[#data.ipaddrs+1] = {
								addr      = info.addr,
								broadaddr = info.broadaddr,
								dstaddr   = info.dstaddr,
								netmask   = info.netmask,
								prefix    = info.prefix
							}
						elseif info.family == "inet6" then
							data.ip6addrs = data.ip6addrs or { }
							data.ip6addrs[#data.ip6addrs+1] = {
								addr    = info.addr,
								netmask = info.netmask,
								prefix  = info.prefix
							}
						end
					end
				end

				if next(data) then
					rv[#rv+1] = data
				end
			end
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end
function iface_reconnect()
	local path  = luci.dispatcher.context.requestpath
	local iface = path[#path]
	local netmd = require "luci.model.network".init()


	local net = netmd:get_network(iface)
	if net then
		local ifn
		for _, ifn in ipairs(net:get_interfaces()) do
			local wnet = ifn:get_wifinet()
			if wnet then
				local wdev = wnet:get_device()
				if wdev then
					luci.sys.call(
						"env -i /sbin/wifi up %q >/dev/null 2>/dev/null"
							% wdev:name()
					)

					luci.http.status(200, "Reconnected")
					return
				end
			end
		end

		luci.sys.call("env -i /sbin/ifup %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Reconnected")
		return
	end

	luci.http.status(404, "No such interface")
end

function iface_shutdown()
	local path  = luci.dispatcher.context.requestpath
	local iface = path[#path]
	local netmd = require "luci.model.network".init()

	local net = netmd:get_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Shutdown")
		return
	end

	luci.http.status(404, "No such interface")
end


