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

f = SimpleForm("cloud", translate("Bind Device"),
		translate("Please get account and password from 'RippleTek Cloud Management Platform'->'device management', fill and operate device bind."))

s = f:field(DummyValue, "BindDevice", translate(""))

--s.template = "cbi/tblsection"

account = f:field(Value, "account", translate("Bind Device Account"))
account.rmempty  = false 

passwd = f:field(Value, "password", translate("Bind Device Password"))
passwd.password = true
passwd.rmempty  = false 
--[[
bind = f:field(Button, "BindDev", translate("Bind Device"))
function bind.write(self, s)
	local acc = account.formvalue(s)
	local pwd = passwd.formvalue(s)
end
--]]

function f.handle(self, state, data)

	if state == FORM_VALID then
		if data.account and data.password then

			luci.sys.call("/usr/sbin/wifidog_devbind.sh bind %s %s" %{ data.account, data.password })
		end
	end
	return true
end

return f
