--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_internet.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local ds = require "luci.dispatcher"
local os  = require "os"
local string = require "string"
local wanlinkfs =  require "luci.model.wanlink".init()

local has_v2 = nixio.fs.access("/lib/network/internet.sh")

require("luci.tools.webadmin")
m = Map("wanctl", nil)
--m.apply_before_commit = true
--nw.init(m.uci)
m.redirect = ds.build_url("admin", "network", "internet")
--
-- Rules
--

s = m:section(TypedSection, "wanlink", translate("Internet List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_net"
s.extedit   = ds.build_url("admin", "network", "internet", "third", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	
	-- max limit the 4
        local count=0
	self.map.uci.foreach("wanctl","wanlink",function(s)
			count=count+1
		end)	
	if count >=4 then
		m.message =translate("There are no more than four wan connections !")
		return 
	end
	--local created = TypedSection.create(self, section)
	--m.uci:save("wanctl")

	luci.http.redirect(ds.build_url(
		"admin", "network", "internet", "rule", created
	))
	return
end
function s.parse(self, novld)
	local have_remove=nil
	local REMOVE_PREFIX = "cbi.rts."
	local crval = REMOVE_PREFIX .. self.config
	local name = self.map:formvaluetable(crval)
	for k,v in pairs(name) do
			if k:sub(-2) == ".x" then
				k = k:sub(1, #k - 2)
			end
			if self:cfgvalue(k) and self:checkscope(k) then
				have_remove = 1
			end		
	end
	if have_remove then
			-- handle  when the section is only one
		local count=0
		self.map.uci.foreach("wanctl","wanlink",function(s)
				count=count+1
			end)
		if count <= 1 then
			m.message =translate("There is at least one wan connection !")
			return 
		end
	end
	return TypedSection.parse(self,novld)
end
 
nm = s:option(DummyValue, "_name", translate("Internet Name"))
function nm.cfgvalue(self, s)

      local ssname = {
           [ "-1"] = "Unkown",
           [ "0"] = "INTERNET",
           [ "1"] = "OTHER",
           [ "2"] = "VOIP",
           [ "3"] = "TR069",
           [ "4"] = "INTERNET_VOIP",
           [ "5"] = "INTERNET_TR069",
           [ "6"] = "VOIP_TR069",
           [ "7"] = "INTERNET_VOIP_TR069"          
      }

	
	local linkId	
	local servicemode
	local vid
	local mode
	local md 

	--get the number of the linkID -- new add is end				
        local showlnkId={}
	local has_entry={}
	local count =0
	self.map.uci.foreach("wanctl","wanlink",function(section)
			local secname = section[".name"]
			local linktbl = wanlinkfs.wanlink_get(secname)
		
	 		if linktbl then
				local idx  = linktbl["WanID"] 
				if idx then
				   has_entry[tonumber(idx)] = 1	
				   showlnkId[secname] = idx
				end	
			end
		count = count + 1
	end)	
	self.map.uci.foreach("wanctl","wanlink",function(section)
		local item = section[".name"]
		if not showlnkId[item] then
	          for i=1,count do
		    if not has_entry[i] then
		       has_entry[i] = 1
		       showlnkId[item] = tostring(i)
		       break
		    end	
	         end	 
	       end
	end)	

	---
	local linktbl = wanlinkfs.wanlink_get(s)
	if linktbl then 
		linkId  = linktbl["WanID"] or "-1"
		servicemode = linktbl["Servicemode"]	or "-1"
		vid  = linktbl["VlanID"]
		mode = linktbl["Mode"]
		md = (mode == "1") and "R" or "B"
		vid = (vid == "-1") and "" or vid

	else -- test
		
	--	linkId  = self.map:get(s,"WanID") or "-"
		linkId  = showlnkId[s] or "-"
		servicemode = self.map:get(s,"Servicemode")	or "-1"

		vid  = self.map:get(s,"VlanID")
		mode = self.map:get(s,"Mode")
		md = (mode == "1") and "R" or "B"
		vid = (vid == "-1") and "" or vid
	end
	
	local strname 
	
	strname  = string.format("%s_%s_%s_VID_%s ",linkId or "",ssname[servicemode] or "",md or "" ,vid or "")
	return strname

end

--[[
interface = s:option(DummyValue, "_interface", translate("Interface"))
function interface.cfgvalue(self, s)

	local vid= self.map:get(s, "VlanID") or "-1"
	local ifas = "eth0"

	if vid ~= "-1" then
	   ifas = string.format("%s.%s",ifas,vid)
	end
	
	return ifas
end
]]--
local portbind = s:option(DummyValue,"PortMap",translate("PortMap"))
function portbind.cfgvalue(self,section)
	local map = self.map:get(section,"PortMap")
	
	map = map and string.gsub(map,"(%w+)%s+(%w+)","%1,%2")
	return map
end
return m
