
module("luci.controller.head", package.seeall)

local user = luci.dispatcher.context.path[1]  or "admin"
--local uci = require "luci.model.uci".cursor()

function index()
	local i18n = luci.i18n.translate
	local redir = luci.http.formvalue("redir", true) or
	luci.dispatcher.build_url(unpack(luci.dispatcher.context.request))

	entry({"admin", "head"},nil, i18n("Configuration"))
	entry({"admin", "head", "logout"}, call("action_logout"), i18n("Logout"), 20).query = {redir=redir}
	entry({"customer", "head"},nil, i18n("Configuration"))
	entry({"customer", "head", "logout"}, call("action_logout"), i18n("Logout"), 20).query = {redir=redir}
	entry({"failsafe", "head"},nil, i18n("Configuration"))
	entry({"failsafe", "head", "logout"}, call("action_logout"), i18n("Logout"), 20).query = {redir=redir}

end


function action_logout()
    local e=require"luci.dispatcher"
    local t=require"luci.sauth"
    if  e.context.authsession then
        t.kill(e.context.authsession)
        e.context.urltoken.stok=nil
    end

    luci.http.header("Set-Cookie","sysauth=; path="..e.build_url())
    luci.http.redirect(luci.dispatcher.build_url())
end
--[[
function change_lang_en()
	local value = luci.http.formvalue("language") or "en"

	uci:set("luci", "main", "lang", value)
	uci:commit("luci")
	luci.http.redirect( luci.dispatcher.build_url(user))
end	
function change_lang_cn()
	local value = luci.http.formvalue("language") or "zh_cn"

	uci:set("luci", "main", "lang", value)
	uci:commit("luci")
	luci.http.redirect( luci.dispatcher.build_url(user))
end	
]]--




