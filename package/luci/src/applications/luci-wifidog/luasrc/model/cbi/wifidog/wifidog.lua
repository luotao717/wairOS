local fs = require "nixio.fs"
local cServerip = "/etc/config/serverip.conf"
local cServerport = "/etc/config/serverip.conf"
local cAuthserver = "/etc/config/authserver.conf"
local cUsername = "/etc/config/username.conf"
local cUserpwd = "/etc/config/userpwd.conf"
f = SimpleForm("wifidogconf", translate("认证配置"), translate("配置管理员分配给您的信息"))

serverip = f:field(Value, "serverip",translate("服务器地址"))
serverport = f:field(Value, "serverport",translate("服务器端口"))
authserver = f:field(Value, "authserver",translate("认证页面地址"))
username = f:field(Value, "username",translate("用户名"))
userpwd = f:field(Value, "userpwd",translate("用户密码"))
serverip.rmempty = true
serverport.rmempty = true
authserver.rmempty = true
username.rmempty = true
userpwd.rmempty = true

function serverip.cfgvalue(...)
         local v =  fs.readfile(cServerip)
         return  v
end

function serverip.write(self, section, value)
        fs.writefile(cServerip, value)
end

function serverport.cfgvalue(...)
         local v =  fs.readfile(cServerport)
         return  v
end

function serverport.write(self, section, value)
        fs.writefile(cServerport, value)
end

function authserver.cfgvalue(...)
         local v =  fs.readfile(cAuthserver)
         return  v
end

function authserver.write(self, section, value)
        fs.writefile(cAuthserver, value)
end

function username.cfgvalue(...)
         local v =  fs.readfile(cUsername)
         return  v
end

function username.write(self, section, value)
        fs.writefile(cUsername, value)
end

function userpwd.cfgvalue(...)
         local v =  fs.readfile(cUserpwd)
         return  v
end

function userpwd.write(self, section, value)
        fs.writefile(cUserpwd, value)
end

--[[t.rmempty = true
t.rows = 10

function t.cfgvalue()
	return fs.readfile(cronfile) or ""
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		if data.crons then
			fs.writefile(cronfile, data.crons:gsub("\r\n", "\n"))
		end
	end
	return true
end
]]--

return f
