--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wlan.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local uci = require "luci.model.uci".cursor()
local dsp = require "luci.dispatcher"
local os = require "os"

m = Map("voip", translate("VOIP"), "")

m.redirect = dsp.build_url("admin", "services", "voip")


s3 = m:section(TypedSection, "advancepolicy", translate("Port Advanced Settings"))
s3.anonymous = true
s3.addremove = false

--sPort=s3:option(Value, "Port", translate("Port"))

code1=s3:option(ListValue, "code1", translate("Language Encoder 1"))
code1:value("0", translate("G.711A"))
code1:value("1", translate("G.711U"))
code1:value("2", translate("G.729"))
code1:value("3", translate("G.723"))

code2=s3:option(ListValue, "code2", translate("Language Encoder 2"))
code2:value("0", translate("G.711A"))
code2:value("1", translate("G.711U"))
code2:value("2", translate("G.729"))
code2:value("3", translate("G.723"))

code3=s3:option(ListValue, "code3", translate("Language Encoder 3"))
code3:value("0", translate("G.711A"))
code3:value("1", translate("G.711U"))
code3:value("2", translate("G.729"))
code3:value("3", translate("G.723"))

code4=s3:option(ListValue, "code4", translate("Language Encoder 4"))
code4:value("0", translate("G.711A"))
code4:value("1", translate("G.711U"))
code4:value("2", translate("G.729"))
code4:value("3", translate("G.723"))

rtpPort=s3:option(Value, "RtpPort", translate("Rtp Port"))
rtpPort.datatype = "port"

muteTest=s3:option(Flag, "MuteTestEnabled", translate("Mute Test Enable"))
muteTest.rmempty = false

echoAvoid=s3:option(Flag, "EchoAvoidEnabled", translate("Echo Avoid Enable"))
echoAvoid.rmempty = false

packageTime = s3:option(ListValue, "PackageTimeCycle", translate("PackageTimeCycle(ms)"))
packageTime:value("10", translate("10"))
packageTime:value("20", translate("20"))
packageTime:value("30", translate("30"))
packageTime:value("40", translate("40"))


JitterBuffer=s3:option(Flag, "JitterBufferMode", translate("Jitter Buffer Mode"))
JitterBuffer.rmempty = false

lenMin= s3:option(Value, "JitterBufferLenMin", translate("JitterBuffer Len Min(ms)"),translate("0~50"))
lenMin.datatype = "range(0,50)"

lenMax=s3:option(Value, "JitterBufferLenMax", translate("JitterBuffer Len Max(ms)"),translate("0~200"))
lenMax.datatype = "range(0,200)"

inVolume= s3:option(Value, "InputVolume", translate("Input Volume"),translate("0~22"))
inVolume.datatype = "range(0,22)"


outVolume=s3:option(Value, "OutputVolume", translate("Output Volume"),translate("0~22"))
outVolume.datatype = "range(0,22)"



dtmfMode= s3:option(ListValue, "DtmfMode", translate("DTMF Mode"))
dtmfMode:value("0", translate("Band"))
dtmfMode:value("1", translate("2833"))
dtmfMode:value("2", translate("sipinfo"))

loadtype= s3:option(Value, "Load2833", translate("2833 Load Type"),translate("96~110"))
loadtype.datatype = "range(96,110)"

t38=s3:option(ListValue, "T38Enabled", translate("T38 Type"))
t38:value("0", translate("VOICE"))
t38:value("1", translate("Pass through"))
t38:value("2", translate("T.38"))

t38r=s3:option(ListValue, "T38Redundancy", translate("T38 Redundancy"))
t38r:value("0", translate("0"))
t38r:value("1", translate("1"))
t38r:value("2", translate("2"))

return m
