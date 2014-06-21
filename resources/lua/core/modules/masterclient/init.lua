local module = require "core.server.module"
local timer = require "luvit.timer"
local http = require "luvit.http"
local querystring = require "luvit.querystring"
local getVar = require "core.cubescript".getVar
local json = require "luvit.json"

local MasterClientModule = module.BaseModule:extend()

MasterClientModule.name = "core.modules.masterclient"

MasterClientModule.registerPath = "/register"
MasterClientModule.updatePath = "/update"
MasterClientModule.unRegisterPath = "/delete"
MasterClientModule.host = "playrr.theintercooler.com"
MasterClientModule.port = 80

local INTERVAL = 5 * 1000 -- Every 5 sec

function MasterClientModule:initialize(server)
    module.BaseModule.initialize(self, server)

    local startupTask = server:getStartupTask()
    startupTask:addTask(self:loadConfiguration(function(err, config)
        if err then
            self:warning("Could not load configuration: "..tostring(err))
            return startupTask:finishTask()
        end
        
        if type(config.host) ~= "string" then
            self:warning("host is not defined.\n")
        else
            self.host = config.host 
        end
        
        if type(config.port) ~= "number" then
            self:warning("port is not defined.\n")
        else
            self.port = config.port
        end

        if type(config["path.update"]) ~= "string" then
            self:warning("path.update is not defined.\n")
        else
            self.updatePath = config["path.update"]
        end
        
        if type(config["path.register"]) ~= "string" then
            self:warning("path.register is not defined.\n")
        else
            self.registerPath = config["path.register"]
        end
        
        if type(config["listname"]) ~= "string" then
            if type(config["listname"]) ~= "nil" then
                self:warning("listname is not defined.\n")
            end
        else
            self.listName = config["listname"]:gsub("^%s*(.-)%s*$", "%1") --trim
            if #self.listName == 0 then
                self.listName = nil
            end
        end
        
        timer.setTimeout(0, function()
            self:updateMaster()
        end)
        
        startupTask:finishTask()
    end))
end

function MasterClientModule:updateMaster()
    local finished = false
    local function finish()
        if not finished then
            finished = true
            
            if not self._timer then
                self._timer = timer.setInterval(INTERVAL, function()
                    self:updateMaster()
                end)
            end
        end
    end
    
    local path = self._password and self.updatePath or self.registerPath
    
    self:info (("Registering to %s : %i %s\n"):format(self.host, self.port, path))
    
    local req
    req = http.request({
        host = self.host,
        port = self.port,
        path = path,
        method = 'POST',
    }, function (res)
        if self._password then --was update request
            if res.statusCode == 204 then
                self:info("Finished updating.\n")
                res:destroy()
                finish()
                return
            elseif res.statusCode == 404 then
                req:emit("error", "Registration timed out.\n")
                res:destroy()
                return
            end
        end
        
        local parser = json.streamingParser(function(value)
            if value.error then
                req:emit("error", value.error)
            else        
                if value and value.password and value.uuid then
                    self._uuid = value.uuid
                    self._password = value.password
                    res:destroy()
                    self:info (("Finished registering, UUID: %s, Password: %s\n"):format(self._uuid, self._password))
                    finish()
                else
                    p(value)
                    req:emit("error", "response does not contain either password or uuid.")
                end
            end
        end)
        
        res:on('data', function (chunk)
            local suc, err = pcall(function()
                parser:parse(chunk)
            end)
            p({chunk = chunk})
            if not suc then
                req:emit("error", err)
                res:destroy()
                parser = nil
            end
        end)
        
        res:on("end", function ()
            if parser then
                local suc, err = pcall(function()
                    parser:complete()
                end)
                
                if not suc then
                    p(res)
                    req:emit("error", err)
                end
            end
            res:destroy()
        end)
    end)
    
    req:on("error", function(err)
        self._uuid = nil
        self._password = nil
        self:warning (("Could not update to the master: %s\n"):format(tostring(err)))
        finish()
    end)
    
    local content = 
        "gamemode="     ..querystring.urlencode(tonumber(self.server:getRawApi().getGameModeId()))      .. "&" ..
        "mapname="      ..querystring.urlencode(tostring(self.server:getMapName()))                     .. "&" ..
        "timeleft="      ..querystring.urlencode(tonumber(self.server:getRawApi().getTimeLeft()))      .. "&" ..
        "playercount="  ..querystring.urlencode(tonumber(self.server:getRawApi().getPlayerCount()))
    
    if self._password and self._uuid then
        --Update
        content = content .. "&uuid=" ..querystring.urlencode(self._uuid).. "&password="..querystring.urlencode(self._password)
    else
        --First registration
        content = content .. "&" ..
            "maxplayers="       ..querystring.urlencode(tonumber(getVar "maxclients"))                  .. "&" ..
            "name="             ..querystring.urlencode(tostring(self.listName or getVar "servername")) .. "&" ..
            "port="             ..querystring.urlencode(tonumber(getVar "serverport"))                  .. "&" ..
            "version="          ..querystring.urlencode(tostring(self.server:getVersionString()))       .. "&" ..
            "protocol="         ..querystring.urlencode(tostring(self.server:getRawApi().getProtocolVersion()))
            
        local host = querystring.urlencode(getVar "serverhost" or getVar "serverip" or "")
        
        if host ~= ""then
            --Don't send if not given
            content = content .. "&host="..host
        end
    end
    
    p {content = content }
    
    req:setHeader('User-Agent', ('ReveladeRevolution-server/%s (protocol-%i)'):format(self.server:getVersionString(), self.server:getRawApi().getProtocolVersion()))
    req:setHeader('Content-Length', #content)
    req:setHeader('Content-Type', 'application/x-www-form-urlencoded')
    req:write(content)
    
    req:done()
end

function MasterClientModule:unload(shutdownTask, ...)
    module.BaseModule.unload(self, shutdownTask, ...)

    if self._uuid and self._password then
        self:info ("Removing server from master list\n")
        
        local httpTask = shutdownTask and shutdownTask:addTask() --Http request
            
        local req
        req = http.request({
            host = self.host,
            port = self.port,
            path = self.unRegisterPath,
            method = 'POST',
        }, function (res)
        
            local parser = json.streamingParser(function(value)
                if value.error then
                    req:emit("error", value.error)
                end
            end)
            
            res:on('data', function (chunk)
                local suc, err = pcall(function()
                    parser:parse(chunk)
                end)
                
                p({chunk = chunk})
                
                if not suc then
                    req:emit("error", err)
                    res:destroy()
                    parser = nil
                end
            end)
        
            res:on("end", function ()
                p "end"
                if parser then
                    parser:complete()
                end
                
                res:destroy()
                    
                if httpTask then
                    shutdownTask:finishTask()
                    shutdownTask = nil
                end
            end)
        end)

        req:on("error", function(err)
            self:warning (("Could not remove from the master server: %s\n"):format(tostring(err)))

            if httpTask then
                shutdownTask:finishTask()
                shutdownTask = nil
            end
        end)
        
        local content = 
        'uuid='..querystring.urlencode(self._uuid or "").. "&"..
        'password='..querystring.urlencode(self._password or "")
        
        req:setHeader('User-Agent', 'tig.rr.server.master-client')
        req:setHeader('Content-Length', #content)
        req:setHeader('Content-Type', 'application/x-www-form-urlencoded')
        req:write(content)
        
        req:done()
    end
    
    if self._timer then
        self._timer:stop()
        self._timer = nil
    end
end

return {
    Module = MasterClientModule,    
}