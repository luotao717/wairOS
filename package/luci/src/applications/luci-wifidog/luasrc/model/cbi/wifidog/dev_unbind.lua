--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("luci.sys")
require("luci.util")

m = SimpleForm("cloud", translate("Bind Device"),
		translate("Please get account and password from 'RippleTek Cloud Management Platform'->'device management', fill and operate device bind."))

s = m:field(DummyValue, "BindDevice", translate(""))

--s.template = "cbi/tblsection"

account = m:field(Value, "account", translate("Bind Device Account"))
account.rmempty  = true

passwd = m:field(Value, "password", translate("Bind Device Password"))
passwd.password = true
passwd.rmempty  = true

bind = m:field(Button, "BindDev", translate("Bind Device"))
function bind.write(self, s)
	local acc = account.formvalue(s)
	local pwd = passwd.formvalue(s)

	luci.sys.call("/usr/sbin/wifidog_devbind.sh bind %s %s" %{ acc, pwd })
end

return m
