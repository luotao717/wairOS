--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntp.lua 6065 2010-04-14 11:36:13Z ben $
]]--
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.fs")
-- local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"

local m = Map("ntpclient",nil, nil)
m:chain("system")

m.redirect = ds.build_url("admin", "network", "ntpc")

s = m:section(TypedSection, "ntpdrift", translate("System Time"))
s.anonymous = true
s.addremove = false

stime = s:option(DummyValue, "_time", translate("Current Time"))
stime.value = os.date("%c")
local tz = s:option(ListValue, "zonename", translate("Time Zone"))
tz:value("UTC")

for i, zone in ipairs(luci.sys.zoneinfo.TZ) do
	tz:value(zone[1])
end

function tz.write(self, section, value)
	local function lookup_zone(title)
		for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
			if zone[1] == title then return zone[2] end
		end
	end

--	AbstractValue.write(self, section, value)
 	local timezone = lookup_zone(value) or "GMT0"
	self.map.uci:foreach("system","system",function(sec)
			self.map.uci:set("system", sec[".name"], self.option, value)
			self.map.uci:set("system", sec[".name"], "timezone", timezone)
	--		luci.fs.writefile("/etc/TZ", timezone .. "\n")
		end)
end

function tz.cfgvalue(self,section)
	local rvalue 
	self.map.uci:foreach("system","system",function(sec)
			rvalue = self.map.uci:get("system", sec[".name"], self.option)
		end )
		
	return rvalue
end

refresh = s:option(Button, "_refresh", translate("Refresh"))
function refresh.write(self, s)
--	luci.http.redirect(ds.build_url("admin", "network", "ntpc"	))
	if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()
				
			nixio.exec("/bin/sh","-c", "/etc/init.d/ntpclient reload")				
			
		else		
			luci.http.redirect(ds.build_url("admin", "network", "ntpc"	))
			os.exit(0)
		end
end


s = m:section(TypedSection, "ntpclient", translate("Time Synchronisation"))
s.anonymous = true
s.addremove = false
m.redirect = ds.build_url("admin", "network", "ntpc")

local ntpserv = {}
local servcnt  = 0
function m.on_parse()
   m.uci:foreach("ntpclient","ntpserver",
          function(section)                
                ntpserv[servcnt+1]  = section['.name']
                servcnt = servcnt +1
             end
        )
 -- 只检查是否有两个ntp 服务器，一个为首选一个备用       
   for i = servcnt+1 ,2 do
     if ntpserv[i] == nil then
       ntpserv[i] =  
              m.uci:section("ntpclient","ntpserver",nil,
                    {
                       ["hostname"] = "time.windows.com",
                       ["port"] = "123"
                    }
                 )
     end
   end
end

ntp_enable = s:option(Flag, "enable", translate("Enable NTP"))
ntp_enable.rmempty = true
function ntp_enable.cfgvalue(self,s)
     return self.map:get(s,"enable") or "0"
end

--s:option(Flag, "auto_ntp_en", translate("Auto NTP Server")).rmempty = true

local pri_ntp = s:option(ListValue, "pri_server", translate("Primary NTP Server"))
pri_ntp:value("time.windows.com", translate("time.windows.com"))
pri_ntp:value("time-a.nist.gov", translate("time-a.nist.gov"))
pri_ntp:value("time-b.nist.gov", translate("time-b.nist.gov"))
pri_ntp:value("time-nw.nist.gov", translate("time-nw.nist.gov"))
pri_ntp:value("other", translate("other"))
pri_ntp:depends("enable","1")

function pri_ntp.write(self, section, value)
  AbstractValue.write(self, section, value)
  if value ~= "other" then 
      self.map.uci:set("ntpclient",ntpserv[1], "hostname", value)
  end
end

local manual_ntp1 = s:option(Value, "pri_server_set", translate("Manual NTP Server"))
manual_ntp1.rmempty = true
manual_ntp1:depends("pri_server","other")
manual_ntp1.datatype = "ipandurl"

function manual_ntp1.write(self, section, value)
  AbstractValue.write(self, section, value)
  self.map.uci:set("ntpclient",ntpserv[1], "hostname", value)
end

local sec_ntp2 = s:option(ListValue, "sec_server", translate("Secondary NTP Server"))
sec_ntp2:value("time.windows.com", translate("time.windows.com"))
sec_ntp2:value("time-a.nist.gov", translate("time-a.nist.gov"))
sec_ntp2:value("time-b.nist.gov", translate("time-b.nist.gov"))
sec_ntp2:value("time-nw.nist.gov", translate("time-nw.nist.gov"))
sec_ntp2:value("other", translate("other"))
sec_ntp2:depends("enable","1")

function sec_ntp2.write(self, section, value)
  AbstractValue.write(self, section, value)
  if value ~= "other" then 
     self.map.uci:set("ntpclient",ntpserv[2], "hostname", value)
  end
end

local manual_ntp2 = s:option(Value, "sec_server_set", translate("Manual NTP Server"))
manual_ntp2.rmempty = true
manual_ntp2:depends("sec_server","other")
--manual_ntp2.datatype = "ipandurl"

function manual_ntp2.write(self, section, value)
  AbstractValue.write(self, section, value)
  self.map.uci:set("ntpclient",ntpserv[2], "hostname", value)
end

local manual_time = s:option(Value, "manual_time", translate("Manual Time"))
manual_time.rmempty= true
manual_time:depends({enable = ""})

function manual_time.cfgvalue(self, section)
    local osdateshow = nil
    osdateshow  = os.date("%Y-%m-%d %H:%M:%S")
    return osdateshow
end
function manual_time.write(self, section, value)
   local date_cmd  = nil
   date_cmd = string.format("date -s %q ",value)   
   luci.sys.exec(date_cmd)
     --更新显示时间
   stime.value = os.date("%c") 
   
   self.map.uci:delete_all("ntpclient","ntpserver")

end

return m
