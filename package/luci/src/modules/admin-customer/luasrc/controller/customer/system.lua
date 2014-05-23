--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: system.lua 6068 2010-04-15 00:15:35Z cshore $
]]--

module("luci.controller.customer.system", package.seeall)

function index()
	luci.i18n.loadc("base")
	local i18n = luci.i18n.translate

	entry({"customer", "system"}, alias("customer", "system", "passwd"), i18n("System"), 5).index = true
--	entry({"customer", "system", "index"}, cbi("customer/system", i18n("General"), 1)
	entry({"customer", "system", "passwd"}, cbi("customer/passwd"), i18n("Admin Password"), 10)
	entry({"customer", "system", "backup"}, call("action_backup"), i18n("Backup / Restore"), 80)
 	--entry({"customer", "system", "upgrade"}, call("action_upgrade"), i18n("Flash Firmware"), 90)
	entry({"customer", "system", "reboot"}, call("action_reboot"), i18n("Reboot"), 99)
--	entry({"customer", "system", "logout"}, call("action_logout"), i18n("Logout"), 100)
end


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
function action_backup()
	local sys = require "luci.sys"
	local fs  = require "luci.fs"
	local reset_avail = os.execute([[grep '"rootfs_data"' /proc/mtd >/dev/null 2>&1]]) == 0
--	local restore_cmd = "tar -xzC/ >/dev/null 2>&1"
	local restore_file = "/tmp/restore_file"
	local backup_cmd  = "tar -czT %s 2>/dev/null"
	
	local restore_fpi 
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not nixio.fs.access(restore_file) and not restore_fpi and chunk and #chunk > 0 then
				restore_fpi = io.open(restore_file,"w")
	--		if not restore_fpi then
	--			restore_fpi = io.popen(restore_cmd, "w")
			end
			if restore_fpi and chunk then
				restore_fpi:write(chunk)
			end
			if restore_fpi and eof then
				restore_fpi:close()
			end
		end
	)
		  
	local upload = luci.http.formvalue("archive")
	local backup = luci.http.formvalue("backup")
	local reset  = reset_avail and luci.http.formvalue("reset")
	local backupcmd = luci.http.formvalue("backupcmd")

	if backupcmd then
		if backupcmd == "upload" then
		    if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

--			nixio.exec("/bin/sh" ,"-c","mtd -r erase rootfs_data")
			nixio.exec("/bin/sh" ,"-c","reset_default -r -f " .. restore_file) --reboot after reset to default
		    else
			luci.template.render("customer/applyreboot")
			os.exit(0)
	   	    end
			
		elseif backupcmd == "reset" then
		    if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

--			nixio.exec("/bin/sh" ,"-c","mtd -r erase rootfs_data")
			nixio.exec("/bin/sh" ,"-c","reset_default -p -r ") --reboot after reset to default
		    else
			luci.template.render("customer/applyreboot")
			os.exit(0)
	 	    end
		end 
	elseif upload and #upload > 0 then
	--[[
		if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

--			nixio.exec("/bin/sh" ,"-c","mtd erase rootfs_data;reboot")
			nixio.exec("/bin/sh" ,"-c","reset_default -r -f " .. restore_file) --reboot after reset to default
		else
			luci.template.render("customer/applyreboot")
			os.exit(0)
		end
--		luci.template.render("customer/applyreboot")
--		luci.sys.reboot()
	--]]
		luci.template.render("customer/applyreboot",{backupcmd="upload"})
	elseif backup then
		local backup_file="/tmp/backup_file"
		backup_cmd="cat %s 2>/dev/null "
		sys.call("reset_default -g %s >/dev/null 2>&1" % backup_file )	

		if fs.access(backup_file) then
	--		local reader = ltn12_popen(backup_cmd:format(_keep_pattern()))
			local reader = ltn12_popen(backup_cmd:format(backup_file))
			luci.http.header('Content-Disposition', 'attachment; filename="eoc_%s.cfg"' % {
				luci.sys.hostname()})
			luci.http.prepare_content("application/x-targz")
			luci.ltn12.pump.all(reader, luci.http.write)
			fs.unlink(backup_file)
		end
	elseif reset then
		--[[
	--	luci.template.render("customer/applyreboot")
	--	luci.util.exec("mtd -r erase rootfs_data")
		if nixio.fork() == 0 then
			local i = nixio.open("/dev/null", "r")
			local o = nixio.open("/dev/null", "w")

			nixio.dup(i, nixio.stdin)
			nixio.dup(o, nixio.stdout)

			i:close()
			o:close()

--			nixio.exec("/bin/sh" ,"-c","mtd erase rootfs_data;reboot")
			nixio.exec("/bin/sh" ,"-c","reset_default -p -r ") --reboot after reset to default
		else
			luci.template.render("customer/applyreboot")
			os.exit(0)
		end
		--]]
		luci.template.render("customer/applyreboot",{backupcmd="reset"})
	else
		luci.template.render("customer/backup", {reset_avail = reset_avail})
	end
end

function action_reboot()
	local reboot = luci.http.formvalue("reboot")
	luci.template.render("customer/reboot", {reboot=reboot})
	if reboot then
		luci.sys.reboot()
	end
end

function action_upgrade()
	require("luci.model.uci")

	local tmpfile = "/tmp/firmware.img"
	
	local function image_supported()
		-- XXX: yay...
		return ( 0 == os.execute(
			". /etc/functions.sh; " ..
			"include /lib/upgrade; " ..
			"platform_check_image %q >/dev/null"
				% tmpfile
		) )
	end
	
	local function image_checksum()
		return (luci.sys.exec("md5sum %q" % tmpfile):match("^([^%s]+)"))
	end
	
	local function storage_size()
		local size = 0
		if nixio.fs.access("/proc/mtd") then
			for l in io.lines("/proc/mtd") do
				local d, s, e, n = l:match('^([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+"([^%s]+)"')
				if n == "image" then
					size = tonumber(s, 16)
					break
				end
			end
		elseif nixio.fs.access("/proc/partitions") then
			for l in io.lines("/proc/partitions") do
				local x, y, b, n = l:match('^%s*(%d+)%s+(%d+)%s+([^%s]+)%s+([^%s]+)')
				if b and n and not n:match('[0-9]') then
					size = tonumber(b) * 1024
					break
				end
			end
		end
		return size
	end


	-- Install upload handler
	local file
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not nixio.fs.access(tmpfile) and not file and chunk and #chunk > 0 then
				file = io.open(tmpfile, "w")
			end
			if file and chunk then
				file:write(chunk)
			end
			if file and eof then
				file:close()
			end
		end
	)


	-- Determine state
	local keep_avail   = true
	local step         = tonumber(luci.http.formvalue("step") or 1)
	local has_image    = nixio.fs.access(tmpfile)
	local has_support  = image_supported()
	local has_platform = nixio.fs.access("/lib/upgrade/platform.sh")
	local has_upload   = luci.http.formvalue("image")
	
	-- This does the actual flashing which is invoked inside an iframe
	-- so don't produce meaningful errors here because the the 
	-- previous pages should arrange the stuff as required.
	if step == 4 then
		if has_platform and has_image and has_support then
			-- Mimetype text/plain
			luci.http.prepare_content("text/plain")
			luci.http.write("Starting sysupgrade...\n")

			-- Now invoke sysupgrade
			local keepcfg = keep_avail and luci.http.formvalue("keepcfg") == "1"
			local flash = ltn12_popen("/sbin/sysupgrade %s %q" %{
				keepcfg and "" or "-n", tmpfile
			})

			luci.ltn12.pump.all(flash, luci.http.write)

			-- Make sure the device is rebooted
			luci.sys.reboot()
		end


	--
	-- This is step 1-3, which does the user interaction and
	-- image upload.
	--

	-- Step 1: file upload, error on unsupported image format
	elseif not has_image or not has_support or step == 1 then
		-- If there is an image but user has requested step 1
		-- or type is not supported, then remove it.
		if has_image then
			nixio.fs.unlink(tmpfile)
		end
			
		luci.template.render("customer/upgrade", {
			step=1,
			bad_image=(has_image and not has_support or false),
			keepavail=keep_avail,
			supported=has_platform
		} )

	-- Step 2: present uploaded file, show checksum, confirmation
	elseif step == 2 then
		luci.template.render("customer/upgrade", {
			step=2,
			checksum=image_checksum(),
			filesize=nixio.fs.stat(tmpfile).size,
			flashsize=storage_size(),
			keepconfig=(keep_avail and luci.http.formvalue("keepcfg") == "1")
		} )
	
	-- Step 3: load iframe which calls the actual flash procedure
	elseif step == 3 then
		luci.template.render("customer/upgrade", {
			step=3,
			keepconfig=(keep_avail and luci.http.formvalue("keepcfg") == "1")
		} )
	end	
end

function _keep_pattern()
	local kpattern = ""
	local files = luci.model.uci.cursor():get_all("luci", "flash_keep")
	if files then
		kpattern = ""
		for k, v in pairs(files) do
			if k:sub(1,1) ~= "." and nixio.fs.glob(v)() then
				kpattern = kpattern .. " " ..  v
			end
		end
	end
	return kpattern
end

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end
