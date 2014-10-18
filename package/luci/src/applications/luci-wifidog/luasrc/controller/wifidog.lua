--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wifidog.lua 5448 2009-10-31 15:54:11Z jow $
]]--
module("luci.controller.wifidog", package.seeall)

function index()
	require("luci.i18n")
--	luci.i18n.loadc("wifidog")
	
	entry({"admin", "cloud"}, alias("admin", "cloud", "dev_bind"), _("Cloud"), 25).index = true 

	--local devbind = entry({"admin", "cloud", "dev_bind"}, cbi("wifidog/dev_bind"), _("dev_bind"), 10)
	local devbind = entry({"admin", "cloud", "dev_bind"}, call("dev_bind"), _("Bind Device"), 10)
--	devbind.i18n = "wifidog"
	
	--local devunbind = entry({"admin", "cloud", "dev_unbind"}, cbi("wifidog/dev_unbind"), _("dev_unbind"), 20)
	local devunbind = entry({"admin", "cloud", "dev_unbind"}, call("dev_unbind"), _("Unbind Device"), 20)
--	devunbind.i18n = "wifidog"

--[[	
	--
	if not nixio.fs.access("/etc/config/authserver.conf") then
		return
	end
	local page = entry({"admin", "network", "wifidog"}, cbi("wifidog/wifidog"), _("wifidog"), 50)
--	page.i18n = "wifidog"
	page.dependent = true
--]]

end

function dev_bind()
	local account = luci.http.formvalue("account")
	local password= luci.http.formvalue("password")
--	local submit= luci.http.formvalue("submit")
	
	local bind = nil
	local Status_Message = "";

	if account and account ~= "" then
		if password and password ~= "" then 
			bind = "1";
			-- running the script
			luci.sys.call("/usr/sbin/wifidog_devbind.sh bind %s %s" %{ account, password })
		end
	end

	if not nixio.fs.access("/etc/wifidog/bindstatus" ) then
		bind = "2"; 
		luci.template.render("wifidog/dev_bind", {bind=bind, Status_Message = Status_Message})
		return
	end

	local info = nixio.fs.readfile("/etc/wifidog/bindstatus")
	local status = info:match("status:([^\n]+)")

	if status == "1" then -- Bind
		Status_Message = luci.i18n.translate("Device Binded")
	elseif status == "2" then
		Status_Message = luci.i18n.translate("Device Unbinded")
	elseif status == "0" then --running
		bind = "3"
	else
		Status_Message = luci.i18n.translate("Status Unknown")
	end
	
	luci.template.render("wifidog/dev_bind", {bind=bind, status=status, Status_Message = Status_Message})
end

function dev_unbind()
	local account = luci.http.formvalue("account")
	local password= luci.http.formvalue("password")
--	local submit= luci.http.formvalue("submit")
	
	local bind = nil
	local Status_Message = "";

	if account and account ~= "" then
		if password and password ~= "" then 
			bind = "1";
			-- running the script
			luci.sys.call("/usr/sbin/wifidog_devbind.sh unbind %s %s" %{ account, password })
		end
	end

	if not nixio.fs.access("/etc/wifidog/bindstatus" ) then
		bind = "2"; 
		luci.template.render("wifidog/dev_unbind", {bind=bind, Status_Message = Status_Message})
		return
	end

	local info = nixio.fs.readfile("/etc/wifidog/bindstatus")
	local status = info:match("status:([^\n]+)")

	if status == "1" then -- Bind
		Status_Message = luci.i18n.translate("Device Binded")
	elseif status == "2" then
		Status_Message = luci.i18n.translate("Device Unbinded")
	elseif status == "0" then --running
		bind = "3"
	else
		Status_Message = luci.i18n.translate("Status Unknown")
	end
	
	luci.template.render("wifidog/dev_unbind", {bind=bind, status=status, Status_Message = Status_Message})
end

