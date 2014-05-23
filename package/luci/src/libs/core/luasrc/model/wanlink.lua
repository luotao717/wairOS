--[[
--	LuCI - Wanlink model
--	2011 .richerlink
]]--
local type , pairs,ipairs,table,luci,math ,tostring
	= type,pairs,ipairs,table,luci,math ,tostring
local os  = require "os"
local nixio  = require "nixio"
local fs     = require "nixio.fs"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local string = require "string"


module "luci.model.wanlink"

-- contain full information user for module
-- uci or special config for wanctl function module
local wanlinktbl 
local wanlinkinfor 

local WANSPATH  = "/var/.wanctl/wanctl.conf"
local WANCONSTR ="/tmp/wanconf"
local WANCONFS  = "wanctl"

--[[
--  解析xml 格式的配置，将其参数
--  作为table的元素
--  return table
]]--
function _wanlink_parse(xmlstr)
	local conftbl = {}
        if not xmlstr then
           return nil
        end	
	-- parse WanID
	_,_,conftbl["WanID"] = xmlstr:find("<WanID(%d+)%s")
	for k,v in xmlstr:gmatch("(%.?%w+)=(%-?%w+)") do
		conftbl[k] = v
	end
	for k,v in xmlstr:gmatch("(%.?%w+)=(\".-\")") do
		conftbl[k] = v
	end

	return conftbl 
end
--[[
--实现基于section name 的wanlink 配置载入
--section 为空时，实现整个配置的载入
--return table
]]--
function _wanklink_reload()
-- reload the config from function
   local xmlfls = fs.readfile(WANSPATH)
   local xmlconfstr = {}
    local xmlconfstrtmp = {}
   local vlanId
   local strconfig 

   if  not xmlfls  then
     return nil 
   end
   
   --??
   for v in xmlfls:gmatch("<WanID%d+%s.-/>") do
      xmlconfstr[#xmlconfstr+1] = v
   end
   for k,v in ipairs(xmlconfstr) do
   	_,_,vlanId = v:find("VlanID=(%-?%d+)")	
	_wanlink_app_debug(k .. " " .. v)
	uci:foreach("wanctl","wanlink",function (s)
		_wanlink_app_debug(string.format("name :%s vlan %q vs %q ",s[".name"],tostring(s["VlanID"]),tostring(vlanId) ))
		if tostring(s["VlanID"]) == tostring(vlanId) then
			xmlconfstrtmp[#xmlconfstrtmp+1] = v:gsub("(VlanID=%-?%d*)","%1 .name=" .. s[".name"])
		end
	end)	
   end
	 _wanlink_app_debug("_wanklink_reload xmlconfstrtmp  " .. #xmlconfstrtmp)

    os.execute(string.format("rm %q",WANCONSTR))
    for k,v in ipairs(xmlconfstrtmp) do
    	 os.execute(string.format("echo %q  >> %q ",v,WANCONSTR))
    end   
   xmlfls = fs.readfile(WANCONSTR)
   return xmlfls
end
function _wanklink_get(section)

-- reload the config from function
   local xmlfls = fs.readfile(WANCONSTR)
   local xmlconfstr = {}
   local wanconftbl = {}
   local sectionidx = nil
   
--reload the file everytime
--   if  not xmlfls  then 
     xmlfls = _wanklink_reload()
     if not xmlfls then
     		return nil
     end
--   end
   --??
   for v in xmlfls:gmatch("<WanID%d+%s.-/>") do
      xmlconfstr[#xmlconfstr+1] = v

    end

    for k,v in ipairs(xmlconfstr) do
       wanconftbl[k] = _wanlink_parse(v)
    end

-- section deteal
   if section then
   	for  k,v in wanconftbl do
   		if section  == v[".name"] then
   		   sectionidx = k
   		end
   	end 	
   	return sectionidx and wanconftbl[sectionidx]  or nil
   else       
   	return wanconftbl
   end
end

--[[
-- test
]]--
function wantest()
   local wantbl = {}


   wantbl = _wanklink_get()

   os.execute("echo ok > /tmp/wantest")
   for k,v in ipairs(wantbl)  do
   	local tmpstr = string.format("echo \" %d %s name:%s vlan = %s  %s \" >> /tmp/wantest",k,type(v),type(v)=="table" and v[".name"] or "nothing",tostring(v["VlanID"]),v["WanID"] or "www")
   	os.execute(tmpstr)
   	for o,w in pairs(v) do
   		os.execute(string.format("echo  \" %q =  %q \" >>/tmp/wantest",o,w))
   	end   	
   end

   uci:foreach("wanctl","wanlink",function (s)
   	    os.execute("echo \" ok 2 \" >> /tmp/wantest")
   	    os.execute(string.format("echo \" section name: %s \" >>/tmp/wantest",s[".name"] or "nil"))
   	    for o,w in pairs(s) do
   	    	os.execute(string.format("echo  \" %s =  %s \" >>/tmp/wantest",o,tostring(w)))
   	    end
     end)

   os.execute("echo \"finished\" >> /tmp/wantest")
end
function waninfotest()
  local waninfo={}
  
  waninfo = _waninfo_reload()
  os.execute("echo \" test \" > /tmp/waninfo")
  for k,v in pairs(waninfo) do
  	os.execute("echo " ..  k .. " >> /tmp/waninfo")
  end  
end  
--[[
--  实现wan ifor 信息的载入
]]--
function _waninfo_reload()
   local winfo = {}
   local b
   
   for l in utl.execi("wanctl -t all") do 
		if not (l:match("ConnName") or l:match("List") or not l:match("%S")) then
			local r = utl.split(l, "%s+", nil, true)
--			if #r == 7 then
			if #r >= 5 then
				b = {
					ID    = r[1],
					ConnName      = r[2],
					Interface     = r[3] ,
					Protocol = r[4],
					Servicemode = r[5],
--					IP = r[6],
--					Status = r[7]
				}
			-- no wacntl show bug now
	--			if #r == 6 then
	--				b.IP = "0.0.0.0"
	--				b.Status = r[6]
	--			elseif #r == 7 then
				   b.IP = r[6]
				   b.Status = r[7] 
	--			end
				winfo[r[2]] = b			
			end
		end
	end
   return winfo
end

function init()

	wanlinktbl = {}
	wanlinkinfor = {}

	-- get wanlinktbl 
	wanlinktbl =  _wanklink_get()
	
	wanlinkinfor = _waninfo_reload()
	return _M
end

--[[
function wanlink_reload(self,section)
	local sidx = nil	
	local strname = nil


	if not section then 
		wanlinktbl = {}
	else
		for k,v in ipairs(wanlinktbl) do
		
			if v[".name"] == section then
				sidx = k				 
				break
			end
		end
		if not sidx  then
			return nil
		end
		wanlinktbl[sidx] = {}
	end
	wanlinktbl  = _wanklink_get(section)
end
]]--
function wanlink_get(section)	
	local sidx = nil
	
	if not wanlinktbl then
		return nil
	end
	if not section then
		return wanlinktbl
	else
		  _wanlink_app_debug("wanlink_get " ..  type(wanlinktbl))
		for k,v in ipairs(wanlinktbl) do
		        _wanlink_app_debug(string.format("type name: %s  vs %s",tostring(v[".name"]) or "nil" ,tostring(section)))
			if v[".name"] == tostring(section) then
--				_wanlink_app_debug("here" .. tostring(k))
				sidx = k
				break
			end
		end
		if not sidx  then
--			_wanlink_app_debug("here nil")
			return nil
		end
--		_wanlink_app_debug("here " .. type(wanlinktbl[sidx]))
		return wanlinktbl[sidx]
	end	
end


function waninfo_reload(section)
	if not section then
		wanlinkinfor = {}
	else
		wanlinkinfor[section] = {}
	end
	wanlinkinfor = _waninfo_reload(section)
end
function waninfo_get(section)
	if not section then
		return wanlinkinfor
	else
		return wanlinkinfor[section]
	end
end
--get network message when connect active
local WANINFO_PATH = "/var/run/netconfig/"
function _waninfo_net(connection)


	local iface = connection and connection.Interface or nil
	local waninfotbl={}
	local filename
	local fsinfo	
	
	waninfotbl = connection
	if type(iface) ~= "string" then
		return waninfotbl
	end
	
	filename  = WANINFO_PATH .. iface .. "/" .. "netcfg.conf"
	if not fs.access(filename)  then
		return waninfotbl
	end
	fsinfo = fs.readfile(filename)
	
	waninfotbl.Interface=fsinfo:match("dev=([^\n]+)") or "Unkown"
	waninfotbl.IP=fsinfo:match("ip=([^\n]+)") or " "
	waninfotbl.netmask=fsinfo:match("netmask=([^\n]+)")  or "-"
	waninfotbl.gateway=fsinfo:match("gateway=([^\n]+)")  or "-"
	waninfotbl.pridns=fsinfo:match("dns1=([^\n]+)")  or "-"
	waninfotbl.secdns=fsinfo:match("dns2=([^\n]+)")  or "-"

	return waninfotbl
end
-- get about gateway , dns ...

function waninfo_get_extern(section)
	local wanifctbl={}
	local wanbasetbl={}
	if not section then
		wanbasetbl = wanlinkinfor
	else
		wanbasetbl[section] = wanlinkinfor[section]
	end	
	for n,v in pairs(wanbasetbl) do
		local item =_waninfo_net(v)
		if item then
			wanifctbl[#wanifctbl+1] = item
		end
	end
	return  section  and wanifctbl[1] or wanifctbl  
end

--[[
--  应用wan连接功能的接口函数
--
]]--
function _wanlink_app_debug(strcmd)
--	os.execute(string.format( "echo %q  >> /tmp/wancmddebug",strcmd))	
end
function _wanlink_app_err(strcmd)	
	os.execute(string.format( "echo %q  >> /tmp/wancmderr",strcmd))	
end
function _wanlink_app_exe(strcmd)
	os.execute(strcmd .. " >/dev/null 2>&1")

	os.execute(string.format( "echo %q  >> /tmp/wancmd",strcmd))
end
function _wanlink_app_add(section)
	local strcmd 
	local VlanID  
	local Mode = section["Mode"] or "0"
	
	local Servicemode = section["Servicemode"] or "0"
	local lan1 ,lan2,lan3,lan4  = 0,0,0,0
	local PortMap	

	_wanlink_app_debug(string.format("_wanlink_app_add name:%s",section[".name"]))

	for o,v in pairs(section) do
		_wanlink_app_debug(string.format(" %q =   %q ",tostring(o),tostring(v)))
	end
	
	-- get the port map
	if section["PortMap"] then 
		_,_,lan1 = string.find(section["PortMap"],"(lan1)") 
		    lan1 = lan1 and 1 or 0
		_,_,lan2 = string.find(section["PortMap"],"(lan2)") 
		    lan2 =  lan2 and 1 or 0
		_,_,lan3 = string.find(section["PortMap"],"(lan3)") 
		    lan3 =  lan3 and 1 or 0
		_,_,lan4 = string.find(section["PortMap"],"(lan4)") 
		    lan4 =  lan4 and 1 or 0
	 end
	 	

	PortMap = string.format("%d%d%d%d",lan1,lan2,lan3,lan4)

	VlanID  = section["VlanID"] or -1
	_wanlink_app_debug(string.format("_wanlink_app_add Mode %d %s",Mode,type(Mode)))
	-- route or bridge
	if Mode == "0"  then -- bridge
		strcmd = string.format("wanctl bridge %d %d %q ",VlanID,Servicemode,PortMap)			
	else -- route
		local Nat = section["Nat"] or 0
		local MTU  = 1500
		
		_wanlink_app_debug(string.format("_wanlink_app_add Workmode %d %s",section["Workmode"],type(section["Workmode"])))
	
		-- dhcp  Workmode = 0
		if section["Workmode"] == "0"  then 
			local VenderID = section["VenderID"] or "Computer"	
			strcmd =  string.format("wanctl dhcp %d %d %d %q %d %q",VlanID,MTU,Nat,VenderID,Servicemode,PortMap)
		-- static Workmode = 1
		elseif section["Workmode"] == "1" then 
			local IP = section["IP"] or ""
			local Netmask = section["Netmask"] or ""
			local Gateway = section["Gateway"]  or ""
			local Dns1 = section["Dns1"] or ""
			local Dns2 = section["Dns2"] or ""
			strcmd =  string.format("wanctl static %d %d %d %q %q %q %q %q %d %q",
						VlanID,MTU,Nat,IP,Netmask,Gateway,Dns1,Dns2,Servicemode,PortMap)
		
		else
			local UserName =  section["UserName"] or ""
			local Passwd = section["Passwd"] or ""
			local ServerName = section["ServerName"] or ""
 			local DialWay  = section["DialWay"] or 0
			local IdleTime =  section["IdleTime"] or 0
			-- pppoe  Workmode = 2
			if section["Workmode"] == "2" then 
				strcmd =  string.format("wanctl ppp %d %d %d %q %q %q %d %d %d %q",
								VlanID,MTU,Nat,UserName,Passwd,ServerName,DialWay,IdleTime,Servicemode,PortMap)
			-- pppoeproxy  Workmode = 3
			elseif section["Workmode"] == "3" then 
				local ProxyNum = section["ProxyNum"] or 1
				strcmd =  string.format("wanctl pppproxy %d %d %d %d %q %q %q %d %d %d %q",
								VlanID,MTU,Nat,ProxyNum,UserName,Passwd,ServerName,DialWay,IdleTime,Servicemode,PortMap)
			-- pppoemix  Workmode = 4
			else --if section["Workmode"] == "4" then 
				strcmd =  string.format("wanctl pppmix %d %d %d %q %q %q %d %d %d %q",
								VlanID,MTU,Nat,UserName,Passwd,ServerName,DialWay,IdleTime,Servicemode,PortMap)
			end
			
			_wanlink_app_debug(string.format("_wanlink_app_add route static %s",strcmd))
		end
		
	end
	_wanlink_app_exe(strcmd )
end
function _wanlink_app_del(section)
	local strcmd 
	local configtbl = {}
	local WanID 
	local VlanID  
	local Mode 

	configtbl = wanlink_get(section[".name"])
	_wanlink_app_debug("_wanlink_app_del" .. type(configtbl))
	if not configtbl  then
		_wanlink_app_err(string.format("_wanlink_app_del get configuration error %s ",section[".name"] or "nil"))
		return nil
	end

	WanID = configtbl["WanID"] 
	VlanID = configtbl["VlanID"]
	Mode = configtbl["Mode"]

	if not WanID or not VlanID or not Mode then 
		_wanlink_app_err(string.format("_wanlink_app_del WanID:%d VlanID :%d  Mode:%d ",WanID or -100,VlanID or -100,Mode or -100))
		return nil
	end
-- wxb modify for wanctl del modify ,only delete by del command
--	strcmd = string.format("wanctl del %d %d %d",WanID,VlanID,Mode)
	strcmd = string.format("wanctl del %d %d %d",WanID)
--	os.execute(strcmd .. " >/dev/null 2>&1")
	_wanlink_app_exe(strcmd )

	return configtbl
end
-- oper : add , delete or modify
function _wanlink_app(section , oper)
	local strcmd 
	local VlanID  	
	local configtbl = {}
	
	if oper == "add" then		
		_wanlink_app_add(section)
	elseif oper == "del" then
		_wanlink_app_del(section)
	else -- oper modify
		configtbl = _wanlink_app_del(section)		
		
		if not configtbl  then
			return false
		end
		for o ,v in utl.kspairs(section) do
			configtbl[o] = v
		end
		_wanlink_app_add(configtbl)
	  
	end
end
function wanlink_app()
	-- 获取变更的信息(section)
	local chgtbl = {}
	local configfs  = WANCONFS

--	init()
	
--	chgtbl = uci:changes(configfs)
  	chgtbl = uci:changes(configfs)
		
	if not chgtbl then
	   _wanlink_app_err("app wanlink error")
	   return false
	end


	_wanlink_app_debug(string.format("app wanlink  %s  %d ",type(chgtbl),#chgtbl))

	for r, tbl in pairs(chgtbl) do
		_wanlink_app_debug(r)
		for s, os in pairs(tbl) do
			_wanlink_app_debug(s)
			for o, v in utl.kspairs(os) do
			  _wanlink_app_debug(string.format("%s = %s ",o,v))
			end

			os['.name'] = s  -- add name
			-- section add
			if os['.type'] and os['.type'] ~= "" then
				-- add wanlink item
				_wanlink_app_debug("add")
				_wanlink_app(os,"add")
			-- section delete
			elseif os['.type'] and os['.type'] == "" then
				-- del wanlink item
				_wanlink_app_debug("delete")
				_wanlink_app(os,"del")
			-- modifications
			else
				_wanlink_app_debug("modify")
				_wanlink_app(os,"modify")
			end
			
		end
	end
	_wanlink_app_debug(string.format(" app wanlink  %s  finished ",type(chgtbl)))

	-- commit the uci 
	  uci:commit(configfs)
	-- Refresh data because commit changes section names
	  uci:load(configfs)

	  _wanklink_reload()


	  return true
end


