
local Database = require "geoip".Database
local BaseModule = require "server.module".BaseModule
local Module = BaseModule:extend()

function Module:load(config)
    self.db = Database:new(config.path)

    self.handler = function(event)
        local ip = self.serverHandle:getIp(event.cn)
        local from = self.db:lookup(ip) or {}
        self:sendMessage(event.cn, from)
    end

    self.serverHandle:print ("[geoip] Opened database: "..config.path)

    self.serverHandle:on("client.connect", self.handler)

    self:registerCommand("geoip.getIp", "PRIV_MODERATOR", function(cn)
        cn = tonumber(cn)
        if type(cn) ~= "number" then error("Invalid client number") end
        p "exec getip"
        return tostring(self.serverHandle:getIp(cn))
    end)

    BaseModule.load(self, config)
end

function Module:unload()
    if self.handler then
        self.serverHandle:removeListner("client.connect", self.handler)
        self.handler = nil
    end
    BaseModule.unload(self)
end

function Module:sendMessage(cn, location)
    self.serverHandle:broadcast(
        tostring(self.serverHandle:getDisplayName(cn)) .. 
        " connected from ".. 
        (location.city and location.city ..", " or "") .. 
        (location.country_name or "Unkown")
    )
end

return Module


