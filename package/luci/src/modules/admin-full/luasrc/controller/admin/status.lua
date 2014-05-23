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
module("luci.controller.admin.status", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate
	local has_wifi = nixio.fs.stat("/etc/config/wireless")
	local has_acs = nixio.fs.stat("/etc/config/remote")	
	local has_voip = nixio.fs.stat("/etc/config/voip")	
	local page

	entry({"admin", "status"}, alias("admin", "status", "index"), i18n("Status"), 1).index = true
	entry({"admin", "status", "index"}, template("admin_status/index"), i18n("Device Information"), 1)	
--	entry({"admin", "status", "wan"}, template("admin_status/wan"), i18n("WAN Status"), 2)
--	entry({"admin", "status", "wan"}, template("admin_status/wan"), i18n("WAN Information"), 2)
--	entry({"admin", "status", "lan"}, template("admin_status/lan"), i18n("LAN Status"), 3)
	entry({"admin", "status", "lan"}, template("admin_status/lan"), i18n("LAN Information"), 3)
	
	if has_voip then
		entry({"admin", "status", "voip"}, template("admin_status/voip"), i18n("VOIP Information"), 4)
	end
	
	if has_acs then 
--		entry({"admin", "status", "remote"}, template("admin_status/acs"), i18n("Remote Management Status"), 5)
	end
	page = entry({"admin", "status", "iface_status"}, call("iface_status"), nil)
	page.leaf = true
	page = entry({"admin", "status", "iface_reconnect"}, call("iface_reconnect"), nil)
	page.leaf = true

	page = entry({"admin", "status", "iface_shutdown"}, call("iface_shutdown"), nil)
	page.leaf = true
	
	page = entry({"admin", "status", "iface_wanlink_status"}, call("iface_wanlink_status"), nil)
	page.leaf = true
	
	if has_voip then
		page = entry({"admin", "status", "voip_status"}, call("voip_status"), nil)
		page.leaf = true
	end
	
	page = entry({"admin", "status", "autolog"}, call("autolog_status"), nil)
	page.leaf = true
	
	-- tr069 support
	if has_acs  then
		page = entry({"admin", "status", "acs_status"}, call("acs_status"), nil)
		page.leaf = true
	end 
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

function acs_status()
	local uci = require "luci.model.uci".cursor()
	local filename = "/var/.tr069/tr_status"
	local rv   = { }
	local config_result=99
	--get acs status 
	local fs = require "nixio.fs"
	
	local Inform_status = ""
	local Reason = ""
	local ITMS_status =""
	local count =1
	local line
	if fs.access("/tmp/.tr069/tr_status", "r") then
		for line in io.lines("/tmp/.tr069/tr_status") do
			
			if count ==1 then
				Inform_status = line:match("Inform_status%s*:%s*([^\n]+)")
			end
			if count ==2 then
				Reason	= line:match("Reason%s*:%s*([^\n]+)")	
			end
			if count ==3 then
				ITMS_status	= line:match("ITMS_status%s*:%s*([^\n]+)")
			end
			count = count +1
		end
	end
	--get Result
	uci:foreach("userinfo","UserInfo",function(s) 
				config_result = tonumber(s.Result)
			end)
	if config_result ==  0 then
		config_result = "0"
	elseif config_result ==  1 then
		config_result = "1"
	elseif config_result ==  2 then
		config_result = "2"
	else
		config_result = "3"
	end
	--write to web data
--[[
	local data ={
		inform = luci.i18n.translate(inform_status),
		informerr = luci.i18n.translate(inform_err),
		requrest = luci.i18n.translate(IMTS_status),
		config   = luci.i18n.translate(config_result)
	}
--]]



	if Reason =="no_wan" then
		Reason = "No remote management of WAN connections"
	elseif  Reason =="wan_disabled" then
		Reason = "WAN connection is not effective remote management"
	elseif  Reason =="no_dns" then
		Reason = "No DNS information management channel" 
	elseif  Reason =="no_acs" then
		Reason = "ACS configuration parameters without" 
	elseif  Reason =="dns_fail" then
			Reason = "ACS DNS failures" 
	elseif  Reason =="booting" then
		Reason = "Home gateway is starting" 
	end

	if ITMS_status =="no_request" then
		ITMS_status ="no request"
	end

	if Inform_status =="no_report" then
		Inform_status = ("no report".."("..Reason..")") 
	elseif  Inform_status =="no_response" then
		Inform_status = "no response" 
	end
		
	local data ={
		inform = Inform_status,
		requrest = ITMS_status,
		config   = config_result
	}
	if next(data) then
		rv[#rv+1] = data
	end
	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "Error ACS Status Parse")
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

function voip_status()
	local uci = require "luci.model.uci".cursor()
	local fs = require "nixio.fs"
	local string = require "string"
	local rv   = { }


	

	local ip_addr= luci.util.exec("voipcfg get 3.0")
 	local voip_ver =luci.util.exec("voipcfg get 3.1")
 	local voip_status =luci.util.exec("voipcfg get 3.2")
 	
	if ip_addr == "FAILED"then
	
		ip_addr = "no connect"

	else
		ip_addr = string.sub(ip_addr,4,-1)

	end

	if voip_ver== "FAILED" then
		voip_ver = "error"
	else
		voip_ver = string.sub(voip_ver,4,-1)

	end

	if voip_status== "FAILED"  then
		voip_status = "no report"

	else
		voip_status = string.sub(voip_status,4,-1)
	
	end
	
	if voip_status ==  "0" then
		voip_status = "Unfixed"
	elseif voip_status ==  "1" then
		voip_status = "No Connection"
	elseif voip_status ==  "2" then
		voip_status = "Idle"
	elseif voip_status ==  "3" then
		voip_status = "Ring"
	else
		voip_status = "Busy"
	end
	local data ={
		ip = ip_addr,
		ver = voip_ver,
		status = voip_status
	}
	if next(data) then
		rv[#rv+1] = data
	end
	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "Error ACS Status Parse")
end

function iface_status()
	local path = luci.dispatcher.context.requestpath
	local netm = require "luci.model.network".init()
	local uci = require "luci.model.uci".cursor()

	local rv   = { }
    local tool= require "luci.util"
	

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
            --tool.exec("echo " .. iface .. " >> /tmp/testluci")
			
			   
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
                    --tool.exec("echo packet >> /tmp/testluci")
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


