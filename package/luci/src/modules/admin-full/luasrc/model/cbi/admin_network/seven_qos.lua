--[[


LuCI qos_classtype
(c) 2008 Yanira <forum-2008@email.de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id: qos_classtype.lua 5448 2009-10-31 15:54:11Z jow $

]]--


CREATE_PREFIX = "cbi.cts."
REMOVE_PREFIX = "cbi.rts."
local ds = require "luci.dispatcher"
local s_name = arg[1] --section name by 
local wanlnk= require "luci.model.wanlink".init()
local uci = luci.model.uci.cursor_state()
m = SimpleForm("qos", translate("QoS Type Settting"))
m.redirect = ds.build_url("admin", "network", "qos","rule")
m.pagebuttonaction = true
local typelist = {}

typelist = uci:get("qos",s_name,"type")
if typelist then
--os.execute("echo " .. type(tlist) .. " " .. #tlist .. " >> /tmp/test")	
	for i,v in pairs(typelist) do
	--	os.execute("echo " .. i .. " " .. v .. " >> /tmp/test")	
		local  idx,valType,valMax,valMin,valProtocolList
		_,_,idx,valType,valMax,valMin,valProtocolList=v:find("(.+),(.+),(.+),(.+),(.+)")
		local tbl={}
		tbl["idx"] = idx
		tbl["Type"] = valType
		tbl["Max"] = valMax
		tbl["Min"] = valMin
		tbl["ProtocolList"] = valProtocolList
		typelist[i]=tbl
	end
end
s1 = m:section(Table, typelist)
s1.addremove = true
s1.anonymous = true
s1:option(DummyValue, "Type", translate("Type"))
s1:option(DummyValue, "Max", translate("Parameter 1"))	
Minv=s1:option(DummyValue, "Min", translate("Parameter 2"))
function Minv.cfgvalue(self, s)
	local f = self.map:get(s, "Min")
	if f=="-1" then
		return ""
	else
		return f
	end
end
s1:option(DummyValue, "ProtocolList", translate("ProtocolList"))	
local nam 
function s1.parse(self,section)
--	os.execute("echo parse " .. self.sectiontype .. "  >> /tmp/test" ) 
	if self.addremove then
		-- Remove
	
		local crval = REMOVE_PREFIX .. self.config
		 nam = self.map:formvaluetable(crval)
		for k,v in pairs(nam) do
			if k:sub(-2) == ".x" then
				k = k:sub(1, #k - 2)
				local typelist={}
				typelist = uci:get("qos",s_name,"type")
			
				table.remove(typelist,k)
				local t = table.getn(typelist)
				if t>0 then
					uci:set("qos",s_name,"type",typelist)
				else
					uci:delete("qos",s_name,"type")
				end
				uci:save("qos")
				uci:commit("qos")
				uci:save("qos")
				luci.http.redirect(luci.dispatcher.build_url("admin/network/qos/rule"))
			end
	
		end
	
	end
end


local nam = nam and 1 or 0
local crval = CREATE_PREFIX .. s1.config .. "." .. s1.sectiontype
local name  = s1.map:formvalue(crval)
local tblcreate = name and 1 or 0
local msubmit = m:formvalue("cbi.submit") and 1 or 0

if tblcreate == 1   then

  local valType = m:field(ListValue, "Type", translate("Type"))
valType:value("SMAC", translate("SMAC"))
valType:value("DMAC", translate("DMAC"))
valType:value("8021P", translate("802.1P"))
valType:value("SIP", translate("SIP"))
valType:value("DIP", translate("DIP"))
valType:value("SPORT", translate("SPORT"))
valType:value("DPORT", translate("DPORT"))
valType:value("TOS", translate("TOS"))
valType:value("DSCP", translate("DSCP"))
valType:value("WANInterface", translate("WANInterface"))
valType:value("LANInterface", translate("LANInterface"))
  function valType.validate(self, value,section)
	local typelt = {}
	local valid = true
	local svalue = value
	typelt = uci:get("qos",s_name,"type")
	if typelt then
			--os.execute("echo " .. type(tlist) .. " " .. #tlist .. " >> /tmp/test")	
		for i,v in pairs(typelt) do
		--	os.execute("echo " .. i .. " " .. v .. " >> /tmp/test")	
			local  idx,valType,valMax,valMin,valProtocolList
			_,_,idx,valType,valMax,valMin,valProtocolList=v:find("(.+),(.+),(.+),(.+),(.+)")
			local tbl={}
			tbl["idx"] = idx
			tbl["Type"] = valType
			tbl["Max"] = valMax
			tbl["Min"] = valMin
			tbl["ProtocolList"] = valProtocolList
			typelt[i]=tbl
			if "8021P" ~= valType and value =="8021P" then
				valid = false
      				m.message = translate("802.1p classification type and other types of combinations are not allowed, please adjust")
      			end
      			if valType =="8021P" and value ~="8021P" then
      				valid = false
      				m.message = translate("802.1p classification type and other types of combinations are not allowed, please adjust")
      			end
		end

	end
		
	if valid then
		return value
	else
		return nil
	end

end

  local valMax1 = m:field(Value, "Max1", translate("Soure MAC"),nil)
  valMax1:depends("Type","SMAC")
  valMax1.datatype = "macaddr"
  
   local valMin1 = m:field(Value, "Max11", translate("Dest MAC"),nil)
     valMin1:depends("Type","DMAC")
       valMin1.datatype = "macaddr"
       
  local valMax2 = m:field(Value, "Max2", translate("802.1P Value"),nil)
 valMax2:depends("Type","8021P")
   valMax2.datatype = "range(0,7)"
 
   --[[
  local valMin2 = m:field(Value, "Min2", translate("Min"),nil)
   valMin2:depends("Type","8021P")
     valMin2.datatype = "range(0,7)"
    ]]-- 
   local valMax3 = m:field(Value, "Max3", translate("Soure IP"),nil)
 valMax3:depends("Type","SIP")
 valMax3.datatype = "ipaddr"
 local valMin3 = m:field(ListValue, "Min3", translate("Soure IP NetMask"),nil)
  valMin3:depends("Type","SIP")
   valMin3.datatype = "ipaddr"
  valMin3:value("255.255.255.255",translate("255.255.255.255"))
valMin3:value("255.255.255.0",translate("255.255.255.0"))
valMin3:value("255.255.0.0",translate("255.255.0.0"))
valMin3:value("255.0.0.0",translate("255.0.0.0"))
   
   local valMax4 = m:field(Value, "Max4", translate("Dest IP"),nil)
 valMax4:depends("Type","DIP")
  valMax4.datatype = "ipaddr"
   local valMin4 = m:field(ListValue, "Min4", translate("Dest IP NetMask"),nil)
    valMin4:depends("Type","DIP")
    valMin4.datatype = "ipaddr"
valMin4:value("255.255.255.255",translate("255.255.255.255"))
valMin4:value("255.255.255.0",translate("255.255.255.0"))
valMin4:value("255.255.0.0",translate("255.255.0.0"))
valMin4:value("255.0.0.0",translate("255.0.0.0"))
    
   local valMax5 = m:field(Value, "Max5", translate("Soure Port Max"),nil)
 valMax5:depends("Type","SPORT")
  valMax5.datatype = "port"
 local valMin5 = m:field(Value, "Min5", translate("Soure Port Min"),translate("-1 Means no value of this item"))
  valMin5:depends("Type","SPORT")
   valMin5.datatype = "range(-1,65535)"
   function valMax5.validate(self, value,section)
   		local valid = true
   		local sporte = valMin5:formvalue(section)
   		if sporte>value then
      			valid = false
      			m.message = translate("Soure Port Max must be greater than Soure Port Min")
      		end
 
   		if valid then
			return value
		else
			return nil
	end
   end
   
  local valMax6 = m:field(Value, "Max6", translate("Dest Port Max"),nil)
 valMax6:depends("Type","DPORT")
    valMax6.datatype = "port" 
 local valMin6 = m:field(Value, "Min6", translate("Dest Port Min"),translate("-1 Means no value of this item"))
  valMin6:depends("Type","DPORT")
      valMin6.datatype = "range(-1,65535)"
      
   function valMax6.validate(self, value,section)
   		local valid = true
   		local sporte = valMin6:formvalue(section)
   		if sporte>value then
      			valid = false
      			m.message = translate("Dest Port Max must be greater than Dest Port Min")
      		end
      		 	--	m.message = translate("Soure Port Max must be greater than Soure Port Min")
   		if valid then
			return value
		else
			return nil
		end
   end    

   local valMax7 = m:field(ListValue, "Max7", translate("Max"),nil)
 valMax7:depends("Type","TOS")
 valMax7:value("0", translate("0"))
valMax7:value("2", translate("2"))
valMax7:value("4", translate("4"))
valMax7:value("8", translate("8"))
valMax7:value("16", translate("16"))
--  local valMin = m:field(Value, "Min7", translate("Min"),nil)
  -- valMin:depends("Type","TOS")

   local valMax8 = m:field(Value, "Max8", translate("DSCP Value"),nil)
      valMax8:depends("Type","DSCP")
 valMax8.datatype = "range(0,63)"
 --[[
   local valMin8 = m:field(Value, "Min8", translate("Min"),nil)
    valMin8:depends("Type","DSCP")
      valMin8.datatype = "range(0,63)"
    ]]--
   local valMax9 = m:field(ListValue, "Max9", translate("WAN Interface Select"),nil)
 valMax9:depends("Type","WANInterface")
 for o,v in pairs(wanlnk.waninfo_get()) do
	valMax9:value(v.Interface,translate(v.ConnName))
end
--   local valMin = m:field(Value, "Min9", translate("Min"),nil)
 --  valMin:depends("Type","WANInterface")
    
 local valMax10 = m:field(ListValue, "Max10", translate("LAN Interface Select"),nil)
 valMax10:value("eth1_0", translate("LAN1"))
valMax10:value("eth1_1", translate("LAN2"))
valMax10:value("eth1_2", translate("LAN3"))
valMax10:value("eth1_3", translate("LAN4"))
valMax10:value("ath0", translate("SSID1"))
valMax10:value("ath1", translate("SSID2"))
valMax10:value("ath2", translate("SSID3"))
valMax10:value("ath3", translate("SSID4"))
 valMax10:depends("Type","LANInterface")
 --[[
 local valMin10 = m:field(ListValue, "Min10", translate("Min"),nil)
 valMin10:value("LAN1", translate("LAN1"))
valMin10:value("LAN2", translate("LAN2"))
valMin10:value("LAN3", translate("LAN3"))
valMin10:value("LAN4", translate("LAN4"))
valMin10:value("SSID1", translate("SSID1"))
valMin10:value("SSID2", translate("SSID2"))
valMin10:value("SSID3", translate("SSID3"))
valMin10:value("SSID4", translate("SSID4"))
 valMin10:depends("Type","LANInterface")
 ]]--
  local valproto = m:field(ListValue, "ProtocolList", translate("ProtocolList"))
valproto:value("TCP", translate("TCP"))
valproto:value("UDP", translate("UDP"))
--valproto:value("TCP/UDP", translate("TCP/UDP"))
valproto:value("RTP", translate("RTP"))
valproto:value("ICMP", translate("ICMP"))
valproto:value("ALL", translate("ALL"))

function valproto.validate(self, value,section)
	local typelt = {}
	local valid = true
	local svalue = value
	typelt = uci:get("qos",s_name,"type")
	if typelt then
			--os.execute("echo " .. type(tlist) .. " " .. #tlist .. " >> /tmp/test")	
		for i,v in pairs(typelt) do
		--	os.execute("echo " .. i .. " " .. v .. " >> /tmp/test")	
			local  idx,valType,valMax,valMin,valProtocolList
			_,_,idx,valType,valMax,valMin,valProtocolList=v:find("(.+),(.+),(.+),(.+),(.+)")
			local tbl={}
			tbl["idx"] = idx
			tbl["Type"] = valType
			tbl["Max"] = valMax
			tbl["Min"] = valMin
			tbl["ProtocolList"] = valProtocolList
			typelt[i]=tbl
			if svalue ~= valProtocolList then
				valid = false
      				m.message = translate("The same group, the protocol must be the same type")
      			end
		end
	

	end
	
	if valid then
		return value
	else
		return nil
	end

end


function m.handle(self, state, data)	
    	
		if state == FORM_VALID then	
			local t={}
			local data1 
			data.Max = data.Max1 or data.Max2 or data.Max3 or data.Max4 or data.Max5 or data.Max6 or data.Max7 or data.Max8 or data.Max9 or data.Max10 or data.Max11
			data.Min = data.Min1 or data.Min2 or data.Min3 or data.Min4 or data.Min5 or data.Min6 or data.Min7 or data.Min8 or data.Min9 or data.Min10
						
			if data.Min then
			else
				data.Min = -1
			end
			
			 data1 =data.Type and data.Max and  data.Min and data.ProtocolList
			local tmpx
			
			
			local typelist={}
			local addnew 
			typelist = uci:get("qos",s_name,"type")
		
			
			if typelist then
				addnew =  #typelist+1
			else
				addnew = 1
			end
			
			if data1 then
				tmpx=string.format("%s,%s,%s,%s,%s",addnew,data.Type,data.Max,data.Min,data.ProtocolList)
				t={tmpx}
	
			if typelist then 
				table.insert(typelist, tmpx)

			else
				typelist = {tmpx}
				local t = table.getn(typelist)
				os.execute("echo parse " .. t .. "  >> /tmp/test" ) 
			end
		
			uci:set("qos",s_name,"type",typelist)
			uci:save("qos")
			uci:commit("qos")
			
			luci.http.redirect(luci.dispatcher.build_url("admin/network/qos/rule"))
			end
		end	
	
	
		
end
end

return  m
