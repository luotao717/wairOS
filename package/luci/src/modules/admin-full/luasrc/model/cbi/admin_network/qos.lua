--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: qos.lua 6784 2011-01-23 18:35:17Z jow $
]]--
local ds = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()

m = Map("qos", translate("QoS"))
m.pageqosaction = true

s1 = m:section(TypedSection, "UplinkQoS", translate("QoS"))
s1.anonymous = true
s1.addremove = false

mode = s1:option(ListValue8, "Mode", translate("Mode"))
mode:value("OTHER", translate("OTHER"))
--mode:value("INTERNET,TR069", translate("INTERNET+TR069"))
--mode:value("INTERNET,TR069,VOIP", translate("INTERNET+NMM+VOIP"))
--mode:value("INTERENT,TR069,IPTV", translate("INTERNET+NMM+IPTV"))
--mode:value("INTERNET,TR069,VOIP,IPTV", translate("INTERNET+NMM+VOIP+IPTV"))


ign=s1:option(Flag2, "Enable", translate("Enable"))
ign.enabled  = "true"
ign.disabled = "false"
ign.rmempty = false

bandwith = s1:option(Value3, "Bandwidth", translate("Bandwidth"),translate("(0-1000*1000*100)bps"))
bandwith.rmempty = false
bandwith.datatype = "range(0,100000000)"
bandwith.default = "0"

enable_DSCP_mark = s1:option(Flag8, "EnableDSCPMark", translate("Enable DSCP Mark"))
enable_DSCP_mark.rmempty = false
enable_DSCP_mark.enabled  = "true"
enable_DSCP_mark.disabled = "false"

enable802_1_p = s1:option(ListValue7, "Enable802_1_P", translate("Enable 802.1P"))
enable802_1_p:value("0", translate("Disable"))
enable802_1_p:value("1", translate("No ReMark"))
enable802_1_p:value("2", translate("ReMark"))

plan = s1:option(ListValue6, "Plan", translate("Plan"))
plan:value("priority", translate("SP"))
plan:value("weight", translate("DWRR"))
plan:value("car", translate("CAR"))

-- john add for ingress qos
local wanlnk= require "luci.model.wanlink".init()
iptv = s1:option(ListValue, "IPTV_iface", translate("IPTV Interface"))
iptv:value("none", translate("None"))
for o,v in pairs(wanlnk.waninfo_get()) do
	iptv:value(v.Interface, v.ConnName)
end
iptv.default = "none"


enable_force_weight = s1:option(Flag7, "EnableForceWeight", translate("Enable Force Weight"))
enable_force_weight.rmempty = true
enable_force_weight.enabled  = "true"
enable_force_weight.disabled = "false"

s = m:section(TypedSection, "PriorityQueue", translate("Qos List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_qos"

function s.create(self, section)
	luci.http.redirect(ds.build_url(
		"admin", "network", "qos", "rule", created
	))
	return
end

qid = s:option(DummyValue, "QID", translate("Queue"))

priority = s:option(DummyValue, "Priority", translate("Priority"))
priority.datatype = "uinteger"
priority.size = 5
function priority.cfgvalue(self, s)
	local f = self.map:get(s, "Priority")
	if f == "1" then
		return translate("Highest")
	elseif f == "2" then
		return translate("Higher")
	elseif f == "3" then
		return translate("High")
	elseif f == "4" then
		return translate("Middle")
	elseif f == "5" then
		return translate("Low")
	else
		return translate("Lowest")
	end
end

weight = s:option(Value, "Weight", translate("Weight(0-100)"))
weight.datatype = "range(0,100)"
weight.size = 5
function weight.validate(self, value,section)
	local q1w = s:formvalue("Q1","Weight")	
	local q1en = s:formvalue("Q1","Enable")
	
	local q2w = s:formvalue("Q2","Weight")			
	local q2en = s:formvalue("Q2","Enable")
	
	local q3w = s:formvalue("Q3","Weight")	
	local q3en = s:formvalue("Q3","Enable")
	
	local q4w = s:formvalue("Q4","Weight")			
	local q4en = s:formvalue("Q4","Enable")
	
		
	local q5w = s:formvalue("Q5","Weight")			
	local q5en = s:formvalue("Q5","Enable")

		
	local q6w = s:formvalue("Q6","Weight")			
	local q6en = s:formvalue("Q6","Enable")
	
	q1w = tonumber((q1en and q1w) or 0)
	q2w = tonumber((q2en and q2w) or 0)
	q3w = tonumber((q3en and q3w) or 0)
	q4w = tonumber((q4en and q4w) or 0)
	q5w = tonumber((q5en and q5w) or 0)
	q6w = tonumber((q6en and q6w) or 0)
	
	local qw = q1w + q2w + q3w + q4w + q5w + q6w 
	if  qw > 100 then
		m.message = translate("The effective weight of the sum can not exceed 100!")
		return nil,""
	end
	return value
	
	
end

pri_bandwidth = s:option(Value, "Bandwidth", translate("Bandwidth((0-1000*1000*1000)bps)"))
pri_bandwidth.datatype = "range(0,100000000)"


function pri_bandwidth.validate(self, value,section)
	local q1bw = s:formvalue("Q1","Bandwidth")	
	local q1en = s:formvalue("Q1","Enable")
	
	local q2bw = s:formvalue("Q2","Bandwidth")			
	local q2en = s:formvalue("Q2","Enable")
	
	local q3bw = s:formvalue("Q3","Bandwidth")	
	local q3en = s:formvalue("Q3","Enable")
	
	local q4bw = s:formvalue("Q4","Bandwidth")			
	local q4en = s:formvalue("Q4","Enable")

		
	local q5bw = s:formvalue("Q5","Bandwidth")			
	local q5en = s:formvalue("Q5","Enable")

		
	local q6bw = s:formvalue("Q6","Bandwidth")			
	local q6en = s:formvalue("Q6","Enable")
	
	local zqbw =self.map:formvalue("cbid.".."qos"..".".."UplinkQoS_0"..".".."Bandwidth")


	zqbwn =tonumber(zqbw)
	
	q1bw =tonumber((q1en and q1bw) or 0)
	q2bw =tonumber((q2en and q2bw) or 0)
	q3bw = tonumber((q3en and q3bw) or 0)
	q4bw = tonumber((q4en and q4bw) or 0)
	q5bw = tonumber((q5en and q5bw) or 0)
	q6bw = tonumber((q6en and q6bw) or 0)

	local qbw = q1bw + q2bw + q3bw + q4bw + q5bw + q6bw
	if  qbw and zqbw and (qbw > zqbwn) then
		m.message = translate("The effective bandwidth can not exceed the total bandwidth")
		return nil,""
	end
	return value
	
	
end
qen=s:option(Flag, "Enable", translate("Enable"))
qen.enabled  = "true"
qen.disabled = "false"
qen.rmempty = false

return m
