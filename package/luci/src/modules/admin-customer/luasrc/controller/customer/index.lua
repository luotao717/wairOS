--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: index.lua 5485 2009-11-01 14:24:04Z jow $
]]--

module("luci.controller.customer.index", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate

	local root = node()
	if not root.lock then
		root.target = alias("customer")
		root.index = true
	end
	
	entry({"about"}, template("about"))
	
	local page   = entry({"customer"}, alias("customer", "status"), i18n("Customer"), 10)
	page.sysauth = "useradmin"
	page.sysauth_authenticator = "htmlauth"
	page.index = true
	
	entry({"customer", "help"}, alias("customer", "help", "help"), i18n("Help"), 6).index = true
	entry({"customer", "help", "help"}, template("customer_status/help"), i18n("Help"), 1)
	
end
--[[
function action_logout()
	local dsp = require "luci.dispatcher"
	local sauth = require "luci.sauth"
	if dsp.context.authsession then
		sauth.kill(dsp.context.authsession)
		dsp.context.urltoken.stok = nil
	end

	luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
	luci.http.redirect(luci.dispatcher.build_url())
end
]]--
