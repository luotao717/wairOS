--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ftp.lua 6065 2010-04-14 11:36:13Z ben $
]]--
local fs     = require "nixio.fs"
local sys= require "luci.sys"
local statusfiles="/tmp/ftpwget_status"
local historyfiles="/tmp/ftpwget_history"

m = Map("ftpclient", translate("Home Storage"), "")
local usbtable = sys.usb_info()

s = m:section(TypedSection, "ftpclient", translate("FTP Client Configuration"))
s.anonymous = true
s.addremove = false
ftp_account = s:option(Value, "ftp_account", translate("FTP Account"))
ftp_account.rmempty=true

ftp_password = s:option(Value2, "ftp_password", translate("FTP Password"))
ftp_password.rmempty=true
ftp_password.password=true

down_url = s:option(Value, "down_url", translate("Download URL"))
down_url.default = "ftp://"

ftp_port=s:option(Value, "ftp_port", translate("FTP Port"))
ftp_port.default=21


if type(usbtable) == "table" and #usbtable > 0 then
seldevice = s:option(ListValue, "device", translate("Storage Device"))
	for o,v in pairs(usbtable) do
		seldevice:value(v,translate(v))
	end
end
savepath=s:option(Value,"localpath",translate("Local Path"))
savepath.rmempty=true
downevent= s:option(Button,"Download",translate("Download"))
--[[
function downevent.render(self, s, scope)
	self.template="cbi/pvalue"
end
--]]

function downevent.write(self, section)
	local account_val=ftp_account:formvalue(section) 
	local passwd_val= ftp_password:formvalue(section) 
	local url_val =down_url:formvalue(section) 
	local ftp_port = ftp_port:formvalue(section) 
	local savedev= (seldevice ~= nil ) and seldevice:formvalue(section) or nil
	local savefile= savepath:formvalue(section) 
	local ftp_mark
	local remote_path,remote_file

	luci.sys.call("rm %s"%statusfiles)
	-- parse ftp url name	
	_,_,remote_path,remote_file = url_val:find("^ftp://(.+)/(%w+)$")
	if not remote_path or not remote_file then
		luci.sys.call("echo  down url error ! not the ftp server url >>%s"%statusfiles)
		return false
	end
	--parse and create authentication
	local ftp_authen
	if not account_val or account_val =="" then
		ftp_authen=""
	else
		ftp_authen=string.format("%s:%s@",account_val,passwd_val)
	end
	--parse savedev
	local localsavepath
	if not savedev or savedev=="" then
		localsavepath="/tmp/"
	else
		localsavepath=string.format("/mnt/%s/",savedev)
	end
	--parse the savefile
	if not savefile  or savefile== "" then
		savefile = remote_file
	end
	
	local ftpstr=string.format("ftp://%s%s:%s/%s",ftp_authen,remote_path,ftp_port,remote_file)

	local historycmd=string.format("FTP download: %s to %s%s",url_val,localsavepath,savefile)
	luci.sys.call("wget %s  -O  %s%s  >> %s 2>&1 && echo  %s >>%s &"%{
			ftpstr,
			localsavepath,
			savefile,
			statusfiles,
			historycmd,
			historyfiles
			})
end

--status of ftp download
ftphistory = s:option(TextValue,"_history",translate("Current FTP download status"))
ftphistory.rmempty = true
ftphistory.rows = 8


function ftphistory.cfgvalue(self,section)
	return fs.readfile(statusfiles) or ""
end
--history of ftp download 
ftphistory = s:option(TextValue,"_history",translate("FTP download history "))
ftphistory.rmempty = true
ftphistory.rows = 10


function ftphistory.cfgvalue(self,section)
	return fs.readfile(historyfiles) or ""
end

return m 
