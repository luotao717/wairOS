--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trule_qos.lua 6983 2011-04-13 00:33:42Z soma $
]]--

local has_v2 = nixio.fs.access("/lib/network/qos.sh")
local dsp = require "luci.dispatcher"

arg[1] = arg[1] or ""

m = Map("qos", nil)

m.redirect = dsp.build_url("admin", "network", "qos")

if not m.uci:get(arg[1]) == "rule" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "rule", translate("Qos Settings"))
s.anonymous = true
s.addremove = false

s:option(Value, "policy_name", translate("Policy Name")).rmempty = false

s:option(Flag, "f_lan_vlanid_en", translate("Enable LAN-VLAN ID")).rmempty = true
f_lan_vlanid = s:option(Value, "f_lan_vlanid", translate("LAN-VLAN ID"), translate("(1-4094)"))
f_lan_vlanid.rmempty = true
f_lan_vlanid.size = 4
f_lan_vlanid.maxlength = 4
f_lan_vlanid.datatype = "uinteger"
f_lan_vlanid:depends("f_lan_vlanid_en",1)

s:option(Flag, "f_wan_vlanid_en", translate("Enable WAN-VLAN ID")).rmempty = true
f_wan_vlanid = s:option(Value, "f_wan_vlanid", translate("WAN-VLAN ID"), translate("(1-4094)"))
f_wan_vlanid.rmempty = true
f_wan_vlanid.size = 4
f_wan_vlanid.maxlength = 4
f_wan_vlanid.datatype = "uinteger"
f_wan_vlanid:depends("f_wan_vlanid_en",1)

s:option(Flag, "f_8021p_en", translate("Enable 802.1p")).rmempty = true
f_8021p = s:option(Value, "f_8021p", translate("802.1p"), translate("(0-7)"))
f_8021p.rmempty = true
f_8021p.size = 1
f_8021p.maxlength = 1
f_8021p.datatype = "uinteger"
f_8021p:depends("f_8021p_en",1)

s:option(Flag, "f_dscp_en", translate("Enable DSCP")).rmempty = true
f_dscp = s:option(Value, "f_dscp", translate("DSCP"), translate("8-63)"))
f_dscp.rmempty = true
f_dscp.size = 2
f_dscp.maxlength = 2
f_dscp.datatype = "uinteger"
f_dscp:depends("f_dscp_en",1)

s:option(Flag, "m_lan_vlanid_en", translate("Modify LAN-VLAN ID")).rmempty = true
m_lan_vlanid = s:option(Value, "m_lan_vlanid", translate("LAN-VLAN ID"), translate("(1-4094)"))
m_lan_vlanid.rmempty = true
m_lan_vlanid.size = 4
m_lan_vlanid.maxlength = 4
m_lan_vlanid.datatype = "uinteger"
m_lan_vlanid:depends("m_lan_vlanid_en",1)

s:option(Flag, "m_wan_vlanid_en", translate("Modify WAN-VLAN ID")).rmempty = true
m_wan_vlanid = s:option(Value, "m_wan_vlanid", translate("WAN-VLAN ID"), translate("(1-4094)"))
m_wan_vlanid.rmempty = true
m_wan_vlanid.size = 4
m_wan_vlanid.maxlength = 4
m_wan_vlanid.datatype = "uinteger"
m_wan_vlanid:depends("m_wan_vlanid_en",1)

s:option(Flag, "m_8021p_en", translate("Modify 802.1p")).rmempty = true
m_8021p = s:option(Value, "m_8021p", translate("802.1p"), translate("(0-7)"))
m_8021p.rmempty = true
m_8021p.size = 1
m_8021p.maxlength = 1
m_8021p.datatype = "uinteger"
m_8021p:depends("m_8021p_en",1)

s:option(Flag, "m_dscp_en", translate("Modify DSCP")).rmempty = true
m_dscp = s:option(Value, "m_dscp", translate("DSCP"), translate("8-63)"))
m_dscp.rmempty = true
m_dscp.size = 2
m_dscp.maxlength = 2
m_dscp.datatype = "uinteger"
m_dscp:depends("m_dscp_en",1)

s:option(Flag, "f_uprate_en", translate("Enable Uplink Rate")).rmempty = true
uplinkrate = s:option(Value, "uplinkrate", translate("Uplink Rate"), translate("(Kbps)"))
uplinkrate.rmempty = true
uplinkrate.datatype = "uinteger"
uplinkrate:depends("f_uprate_en",1)

s:option(Flag, "f_downrate_en", translate("Enable Downlink Rate")).rmempty = true
downloadlinkrate = s:option(Value, "downloadlinkrate", translate("Downlink Rate"), translate("(Kbps)"))
downloadlinkrate.rmempty = true
downloadlinkrate.datatype = "uinteger"
downloadlinkrate:depends("f_downrate_en",1)

protocol = s:option(ListValue, "protocol", translate("Protocol"))
protocol:value("0", translate("All"))
protocol:value("1", translate("TCP"))
protocol:value("2", translate("UDP"))

sip_type = s:option(ListValue, "sip_type", translate("Source IP Type"))
sip_type:value("0", translate("All"))
sip_type:value("1", translate("IP"))
sip_type:value("2", translate("IP Range"))
sip_type:value("3", translate("MAC"))

sip = s:option(Value, "sip", translate("Source IP"))
sip.rmempty = true
sip.maxlength = 15
sip.datatype = "ipaddr"
sip:depends("sip_type",1)

sip1 = s:option(Value, "sip1", translate("Source IP 1"))
sip1.rmempty = true
sip1.maxlength = 15
sip1.datatype = "ipaddr"
sip1:depends("sip_type",2)

sip2 = s:option(Value, "sip2", translate("Source IP 2"))
sip2.rmempty = true
sip2.maxlength = 15
sip2.datatype = "ipaddr"
sip2:depends("sip_type",2)

smac = s:option(Value, "smac", translate("Source MAC"))
smac.rmempty = true
smac.maxlength = 17
smac.datatype = "macaddr"
smac:depends("sip_type",3)

sport_type = s:option(ListValue, "sport_type", translate("Source Port Type"))
sport_type:value("0", translate("All"))
sport_type:value("1", translate("Port"))
sport_type:value("2", translate("Port Range"))
sport_type:depends("protocol",1)
sport_type:depends("protocol",2)

sport = s:option(Value, "sport", translate("Source Port"))
sport.rmempty = true
sport.size = 5
sport.maxlength = 5
sport.datatype = "port"
sport:depends("sport_type",1)

sport1 = s:option(Value, "sport1", translate("Source Port 1"))
sport1.rmempty = true
sport1.size = 5
sport1.maxlength = 5
sport1.datatype = "port"
sport1:depends("sport_type",2)

sport2 = s:option(Value, "sport2", translate("Source Port 2"))
sport2.rmempty = true
sport2.size = 5
sport2.maxlength = 5
sport2.datatype = "port"
sport2:depends("sport_type",2)

dip_type = s:option(ListValue, "dip_type", translate("Dest IP Type"))
dip_type:value("0", translate("All"))
dip_type:value("1", translate("IP"))
dip_type:value("2", translate("IP Range"))
dip_type:value("3", translate("MAC"))

dip = s:option(Value, "dip", translate("Dest IP"))
dip.rmempty = true
dip.maxlength = 15
dip.datatype = "ipaddr"
dip:depends("dip_type",1)

dip1 = s:option(Value, "dip1", translate("Dest IP 1"))
dip1.rmempty = true
dip1.maxlength = 15
dip1.datatype = "ipaddr"
dip1:depends("dip_type",2)

dip2 = s:option(Value, "dip2", translate("Dest IP 2"))
dip2.rmempty = true
dip2.maxlength = 15
dip2.datatype = "ipaddr"
dip2:depends("dip_type",2)

dmac = s:option(Value, "dmac", translate("Dest MAC"))
dmac.rmempty = true
dmac.maxlength = 17
dmac.datatype = "macaddr"
dmac:depends("dip_type",3)

dport_type = s:option(ListValue, "dport_type", translate("Dest Port Type"))
dport_type:value("0", translate("All"))
dport_type:value("1", translate("Port"))
dport_type:value("2", translate("Port Range"))
dport_type:depends("protocol",1)
dport_type:depends("protocol",2)

dport = s:option(Value, "dport", translate("Dest Port"))
dport.rmempty = true
dport.size = 5
dport.maxlength = 5
dport.datatype = "port"
dport:depends("dport_type",1)

dport1 = s:option(Value, "dport1", translate("Dest Port 1"))
dport1.rmempty = true
dport1.size = 5
dport1.maxlength = 5
dport1.datatype = "port"
dport1:depends("dport_type",2)

dport2 = s:option(Value, "dport2", translate("Dest Port 2"))
dport2.rmempty = true
dport2.size = 5
dport2.maxlength = 5
dport2.datatype = "port"
dport2:depends("dport_type",2)

return m
