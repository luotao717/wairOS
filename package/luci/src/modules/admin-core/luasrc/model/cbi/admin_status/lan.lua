--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: network.lua 6440 2010-11-15 23:00:53Z jow $
]]--

local fs = require "nixio.fs"

m = Map("network", nil)
m:section(SimpleSection).template = "admin_status/lan"
m.pageaction = false
return m
