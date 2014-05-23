--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: index.lua 6719 2011-01-13 22:21:16Z jow $
]]--
module("luci.controller.admin.index", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate

	local root = node()
	if not root.target then
		root.target = alias("admin")
		root.index = true
	end

	local page   = node("admin")
	page.target  = alias("admin", "status")
	page.title   = i18n("Administration")
	page.order   = 10
	page.sysauth = "admin"
	page.sysauth_authenticator = "htmlauth"
	page.ucidata = true
	page.index = true

	entry({"admin", "help"}, alias("admin", "help", "help"), i18n("Help"), 6).index = true
	entry({"admin", "help", "help"}, template("admin_status/help"), i18n("Help"), 1)
	
end


