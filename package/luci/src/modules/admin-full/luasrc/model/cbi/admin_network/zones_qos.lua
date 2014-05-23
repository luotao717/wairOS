--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones_qos.lua 6769 2011-01-20 12:38:32Z jow $
]]--

local nw = require "luci.model.network"
local ds = require "luci.dispatcher"

local has_v2 = nixio.fs.access("/lib/network/qos.sh")

require("luci.tools.webadmin")
m = Map("qos", nil)
m.pageqosaction = true

nw.init(m.uci)

--[[
function m.on_parse()
	local has_section = false

	m.uci:foreach("", "config", function(s)
		has_section = true
	end)

	if not has_section then
		m.uci:section("qos", "config", nil, { 
			["bw"]     = "0"})
		m.uci:save("qos")
	end
end
]]--
--
-- Rules
--

s1 = m:section(TypedSection, "config", translate("Basic Settings"))
s1.anonymous = true
s1.addremove = false

s1:option(Flag, "qos_en", translate("Enable Qos")).rmempty = true

uprate = s1:option(Value, "uprate", translate("Maximum rate of uplink"), translate("(Kbps)"))
uprate.rmempty = true
uprate.size = 5
uprate.datatype = "uinteger"

downrate = s1:option(Value, "downrate", translate("Maximum rate of downlink"), translate("(Kbps)"))
downrate.rmempty = true
downrate.size = 5
downrate.datatype = "uinteger"

basicflow = s1:option(ListValue2, "basicflow", translate("Basic Flow"))
basicflow:value("0", translate("Default"))
basicflow:value("1", translate("Layer2"))
basicflow:value("2", translate("Layer3"))

s4 = m:section(TypedSection, "p8021Tbl", translate("802.1p List"))
s4.anonymous = true
s4.addremove = false
s4.template = "cbi/tblsection_8021p"

p8021_0 = s4:option(ListValue, "p8021_0", translate("0"))
p8021_0:value("0", translate("0"))
p8021_0:value("1", translate("1"))
p8021_0:value("2", translate("2"))
p8021_0:value("3", translate("3"))

p8021_1 = s4:option(ListValue, "p8021_1", translate("1"))
p8021_1:value("0", translate("0"))
p8021_1:value("1", translate("1"))
p8021_1:value("2", translate("2"))
p8021_1:value("3", translate("3"))

p8021_2 = s4:option(ListValue, "p8021_2", translate("2"))
p8021_2:value("0", translate("0"))
p8021_2:value("1", translate("1"))
p8021_2:value("2", translate("2"))
p8021_2:value("3", translate("3"))

p8021_3 = s4:option(ListValue, "p8021_3", translate("3"))
p8021_3:value("0", translate("0"))
p8021_3:value("1", translate("1"))
p8021_3:value("2", translate("2"))
p8021_3:value("3", translate("3"))

p8021_4 = s4:option(ListValue, "p8021_4", translate("4"))
p8021_4:value("0", translate("0"))
p8021_4:value("1", translate("1"))
p8021_4:value("2", translate("2"))
p8021_4:value("3", translate("3"))

p8021_5 = s4:option(ListValue, "p8021_5", translate("5"))
p8021_5:value("0", translate("0"))
p8021_5:value("1", translate("1"))
p8021_5:value("2", translate("2"))
p8021_5:value("3", translate("3"))

p8021_6 = s4:option(ListValue, "p8021_6", translate("6"))
p8021_6:value("0", translate("0"))
p8021_6:value("1", translate("1"))
p8021_6:value("2", translate("2"))
p8021_6:value("3", translate("3"))

p8021_7 = s4:option(ListValue, "p8021_7", translate("7"))
p8021_7:value("0", translate("0"))
p8021_7:value("1", translate("1"))
p8021_7:value("2", translate("2"))
p8021_7:value("3", translate("3"))

s5 = m:section(TypedSection, "dscpTbl", translate("DSCP List"))
s5.anonymous = true
s5.addremove = false
s5.template = "cbi/tblsection_dscp"

dscp_0 = s5:option(ListValue, "dscp_0", translate("0-7"))
dscp_0:value("0", translate("0"))
dscp_0:value("1", translate("1"))
dscp_0:value("2", translate("2"))
dscp_0:value("3", translate("3"))

dscp_8 = s5:option(ListValue, "dscp_8", translate("8-15"))
dscp_8:value("0", translate("0"))
dscp_8:value("1", translate("1"))
dscp_8:value("2", translate("2"))
dscp_8:value("3", translate("3"))

dscp_16 = s5:option(ListValue, "dscp_16", translate("16-23"))
dscp_16:value("0", translate("0"))
dscp_16:value("1", translate("1"))
dscp_16:value("2", translate("2"))
dscp_16:value("3", translate("3"))

dscp_24 = s5:option(ListValue, "dscp_24", translate("24-31"))
dscp_24:value("0", translate("0"))
dscp_24:value("1", translate("1"))
dscp_24:value("2", translate("2"))
dscp_24:value("3", translate("3"))

dscp_32 = s5:option(ListValue, "dscp_32", translate("32-39"))
dscp_32:value("0", translate("0"))
dscp_32:value("1", translate("1"))
dscp_32:value("2", translate("2"))
dscp_32:value("3", translate("3"))

dscp_40 = s5:option(ListValue, "dscp_40", translate("40-47"))
dscp_40:value("0", translate("0"))
dscp_40:value("1", translate("1"))
dscp_40:value("2", translate("2"))
dscp_40:value("3", translate("3"))

dscp_48 = s5:option(ListValue, "dscp_48", translate("48-55"))
dscp_48:value("0", translate("0"))
dscp_48:value("1", translate("1"))
dscp_48:value("2", translate("2"))
dscp_48:value("3", translate("3"))

dscp_56 = s5:option(ListValue, "dscp_56", translate("56-63"))
dscp_56:value("0", translate("0"))
dscp_56:value("1", translate("1"))
dscp_56:value("2", translate("2"))
dscp_56:value("3", translate("3"))

queue = s1:option(ListValue, "queue", translate("Queue"))
queue:value("0", translate("0"))
queue:value("1", translate("1"))
queue:value("2", translate("2"))
queue:value("3", translate("3"))

queuePolicy = s1:option(ListValue2, "queuePolicy", translate("Queue Policy"))
queuePolicy:value("0", translate("SP"))
queuePolicy:value("1", translate("WFQ"))

s3 = m:section(TypedSection, "wfqTbl", translate("WFQ List"))
s3.anonymous = true
s3.addremove = false
s3.template = "cbi/tblsection_wfq"

wfq0 = s3:option(ListValue, "wfq0", translate("0"))
wfq0:value("0", translate("0"))
wfq0:value("1", translate("1"))
wfq0:value("2", translate("2"))
wfq0:value("3", translate("3"))
wfq0:value("4", translate("4"))
wfq0:value("5", translate("5"))
wfq0:value("6", translate("6"))
wfq0:value("7", translate("7"))

wfq1 = s3:option(ListValue, "wfq1", translate("1"))
wfq1:value("0", translate("0"))
wfq1:value("1", translate("1"))
wfq1:value("2", translate("2"))
wfq1:value("3", translate("3"))
wfq1:value("4", translate("4"))
wfq1:value("5", translate("5"))
wfq1:value("6", translate("6"))
wfq1:value("7", translate("7"))

wfq2 = s3:option(ListValue, "wfq2", translate("2"))
wfq2:value("0", translate("0"))
wfq2:value("1", translate("1"))
wfq2:value("2", translate("2"))
wfq2:value("3", translate("3"))
wfq2:value("4", translate("4"))
wfq2:value("5", translate("5"))
wfq2:value("6", translate("6"))
wfq2:value("7", translate("7"))

wfq3 = s3:option(ListValue, "wfq3", translate("3"))
wfq3:value("0", translate("0"))
wfq3:value("1", translate("1"))
wfq3:value("2", translate("2"))
wfq3:value("3", translate("3"))
wfq3:value("4", translate("4"))
wfq3:value("5", translate("5"))
wfq3:value("6", translate("6"))
wfq3:value("7", translate("7"))

s = m:section(TypedSection, "rule", translate("Qos List"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
s.extedit   = ds.build_url("admin", "network", "qos", "rule", "%s")
--s.defaults.target = "ACCEPT"

function s.create(self, section)
	local created = TypedSection.create(self, section)
	m.uci:save("network")
	luci.http.redirect(ds.build_url(
		"admin", "network", "qos", "rule", created
	))
	return
end

policy_name = s:option(DummyValue, "policy_name", translate("Policy Name"))
function policy_name.cfgvalue(self, s)
	return self.map:get(s, "policy_name") or "-"
end

protocol = s:option(DummyValue, "protocol", translate("Protocol"))
function protocol.cfgvalue(self, s)
	local f = self.map:get(s, "protocol")
	if f == "0" then
		return "All"
	elseif f == "1" then
		return "TCP"
	elseif f == "2" then
		return "UDP"
	end
end

uplinkrate = s:option(DummyValue, "uplinkrate", translate("Uplink Rate"))
function uplinkrate.cfgvalue(self, s)
	return self.map:get(s, "uplinkrate") or "-"
end

downloadlinkrate = s:option(DummyValue, "downloadlinkrate", translate("Downlink Rate"))
function downloadlinkrate.cfgvalue(self, s)
	return self.map:get(s, "downloadlinkrate") or "-"
end

return m
