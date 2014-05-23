--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: passwd.lua 5448 2009-10-31 15:54:11Z jow $
]]--
local ds = require "luci.dispatcher"
f = SimpleForm("password", translate("User Manage"))
f.pageadminaction = true;
f.redirect = ds.build_url("admin", "system", "admin")
user = f:field(ListValue, "user", translate("User Name"))
user.rmempty = false
user:value("useradmin")
user:value("admin")

pw1 = f:field(Value2, "pw1", translate("New Password"))
pw1.password = true
pw1.rmempty = false

pw2 = f:field(Value2, "pw2", translate("New Password Confirmation"))
pw2.password = true
pw2.rmempty = false

function pw2.validate(self, value, section)
	return pw1:formvalue(section) == value and value
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		local stat = luci.sys.user.setpasswd(data.user, data.pw1) == 0
		
		if stat then
			f.message = translate("Password successfully changed")
		else
			f.errmessage = translate("Unknown Error")
		end
		
		data.pw1 = nil
		data.pw2 = nil
	end
	return true
end

return f
