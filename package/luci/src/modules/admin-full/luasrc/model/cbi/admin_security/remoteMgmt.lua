
require("luci.tools.webadmin")
local ds = require "luci.dispatcher"

m = Map("uhttpd")
m.redirect = ds.build_url("admin", "security", "remoteMgmt")
s = m:section(TypedSection, "uhttpd", translate("Remote Management"))
s.anonymous = true
s.addremove = false
wp = s:option(Value, "listen_http", translate("Web Management Port"))
wp.rmempty = false
wp.maxlength = 25
wp.datatype = "range(0,65535)"
wp.default = "0.0.0.0:80"

function wp.cfgvalue(self, s)
	local ip_port = m.uci:get("uhttpd","main",self.option)
	return string.sub(ip_port, (string.find(ip_port, ":")+1));
end

function wp.write(self, s,value)
	return m.uci:set("uhttpd","main",self.option,"0.0.0.0:"..value)
end

remoteip = s:option(Value, "remote_ip", translate("Remote Management IP"), translate("(Enter 255.255.255.255 for all)"))
remoteip.rmempty = false
luci.tools.webadmin.cbi_add_knownips(remoteip)

return m

