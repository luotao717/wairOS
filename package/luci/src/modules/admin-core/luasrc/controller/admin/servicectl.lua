--[[
LuCI - Lua Configuration Interface

Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: servicectl.lua 6747 2011-01-18 18:01:53Z jow $
]]--

module("luci.controller.admin.servicectl", package.seeall)

local os = require "os"
local fs = require "nixio.fs"
local sys= require "luci.sys"
local uci= require "luci.model.uci".cursor()

local wanlink = require "luci.model.wanlink".init()

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate

--	entry({"servicectl"}, alias("servicectl", "status"), nil, 1).sysauth = "root"
	entry({"servicectl"}, alias("servicectl", "status"), nil, 1)
	entry({"servicectl", "status"}, call("action_status"), nil, 2).leaf = true
	entry({"servicectl", "restart"}, call("action_restart"), nil, 3).leaf = true
end

function action_status()
	local data = nixio.fs.readfile("/var/run/luci-reload-status")
	if data then
--		luci.http.write("/etc/config/")
		luci.http.write(data)
	else
		local sauth = require "luci.sauth"
		luci.http.write("finish")
		sauth.updateall()
	end
end

function action_restart()
	if luci.dispatcher.context.requestpath[3] then
		local service
		local services = { }

		for service in luci.dispatcher.context.requestpath[3]:gmatch("%w+") do
			services[#services+1] = service
		end
		
		if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

			if  luci.dispatcher.context.requestpath[4] == "true" then 
				-- now just support internet configuration
				if(services[1] == "wanctl") then
					wanlink.wanlink_app()
				else
					os.execute(string.format("echo servictl requestpath %q>> /tmp/wanserv",services[1]))		
				end	
			else
				-- 
	
				for i,v in ipairs(services) do
					
					if v == "system" then
					    require("luci.sys")
					    require("luci.sys.zoneinfo")
					    require("luci.fs")
					    
					    local uci = require "luci.model.uci".cursor()
					    local zonename = "UTC"
					    local function lookup_zone(title)
						for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
							if zone[1] == title then return zone[2] end
						end
					    end
					    uci:foreach("system" , "system",function(sec)
					    		zonename = sec.zonename
					        end)
					    local timezone = lookup_zone(zonename) or "GMT0"
				
					    luci.fs.writefile("/etc/TZ", timezone .. "\n")	
					end	
				end
				-- find pid to send ,use signum user2(12)
			--[[
				local pid = fs.readfile("/var/run/tr069.pid")
                       		if pid and #pid > 0 and tonumber(pid) ~= nil then
		        		sys.process.sigqueue(pid,12,table.concat(services,","));	
				end
			--]]				
				uci:apply(services)						
--				nixio.exec("/bin/sh", "/sbin/luci-reload", unpack(services))
			
			end
		else
			luci.http.write("OK")
			os.exit(0)
		end

		
		  
	end
end
