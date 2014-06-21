local process = require "luvit.process"
local timer = require "luvit.timer"
local ffi = require "ffi"
local os = require "os"

local config = require "core.server.config"
local server = require "core.server.api"
local event = require "core.server.event"
local task = require "core.task"
local cubescript = require "core.cubescript"
local moduleContainer = require "core.server.module".createModuleContainer()
local Object = require "luvit.core".Object

local startupTask

local ApiWrapper = Object:extend()

function ApiWrapper:getRawApi()
    return server
end

function ApiWrapper:getStartupTask()
    return startupTask
end

function ApiWrapper:getMapName()
    return ffi.string(self:getRawApi().getMapName())
end

function ApiWrapper:getVersionString()
    return ffi.string(self:getRawApi().getVersionString())
end

function ApiWrapper:print(...)
    for k, msg in pairs({...}) do
        process.stdout:write(("[%s] %s\n"):format(os.date("%Y-%m-%dT%H:%M:%S"), msg))
    end
end

function ApiWrapper:printError(...)
    for k, msg in pairs({...}) do
        process.stderr:write(("[%s] %s\n"):format(os.date("%Y-%m-%dT%H:%M:%S"), msg))
    end
end

local serverApi = ApiWrapper:new()

startupTask = task.createTask(function()
    server.startServer();
end, function(self, message)
    serverApi:printError ("Could not start server: "..tostring(message))
    
    local traceback = require "debug".traceback()
    serverApi:printError(traceback)
    
    process.exit();
end);

startupTask:addTask(config.loadConfiguration("resources/config/server.json", function(err, data)
    if err then
        return startupTask:cancel("Could not read server configuration resources/config/server.json:"..tostring(err))
    end
    
    local function optionalString(str)
        if type(str) == "string" and str ~= "" then
            return str
        end
        
        return nil
    end
    
    local function userList(list)
        if type(list) == "table" then
            for k, v in pairs(list) do
                if type(list) ~= "table" then
                    p "User is not table"
                    return nil
                end

                if not v.value or not v.password then
                    p "Value or password missing"
                    return nil
                end

                if type(v.value) == "string" then
                    list[k].value = ({
                        PRIV_NONE = 0,
                        PRIV_PLAYER = 1,
                        PRIV_MODERATOR_INACTIVE = 2,
                        PRIV_DEVELOPER_INACTIVE = 3,
                        PRIV_CREATOR_INACTIVE = 4,
                        PRIV_ADMIN_INACTIVE = 5,
                        PRIV_MASTER = 6,
                        PRIV_MODERATOR = 7,
                        PRIV_DEVELOPER = 8,
                        PRIV_CREATOR = 9,
                        PRIV_ADMIN = 10
                    })[v.value]
                    if not v.value then
                        p "Invalid priv"
                        return nil
                    end
                end
            end
            
            return list
        end

       return nil
    end
    
    for k, setting in pairs({
        {field = "server.maxclients", name = "maxclients", checkFunction = tonumber},
        {field = "server.port", name = "serverport", checkFunction = tonumber},
        
        {field = "server.name", name = "servername", checkFunction = tostring},
        
        {field = "server.ip", name = "serverip", checkFunction = optionalString, optional = true},
        
        {field = "server.hostname", name = "serverhost", checkFunction = optionalString, optional = true},
        {field = "server.users", name = "users", checkFunction = userList}
    }) do
    
        local value = setting.checkFunction(data[setting.field])
        
        if not value then
            if not setting.optional then
                return startupTask:cancel(("Invalid value given for \"%s\" (%s)"):format(setting.field, tostring(value)));
            end
        else
            cubescript.setVar(setting.name, value);
            
            local cubescriptValue = cubescript.getVar(setting.name)
            if value ~= cubescriptValue then
                return startupTask:cancel(("Failed to set cubescript value for: %s aka %s (%s != %s)."):format(setting.field, setting.name, value, cubescriptValue))
            end
        end
    end
    
    if type(data["server.modules"]) ~= "table" then
        serverApi:printError("Warning: not loading any modules.")
    else
        for k, module in pairs(data["server.modules"]) do
            local success, msg = pcall(function()
                local mod = require (module)
                
                if type(mod.Module) ~= "table" then
                    error("Invalid module (does not implement Module object)")
                end
                
                moduleContainer:register(module, mod.Module:new(serverApi))
            end)
            
            if success then
                serverApi:print (("~  Loading module: %s"):format(module))
            else
                serverApi:print (("~! Could not load module: %s\n%s"):format(module, msg))
            end
        end
    end
    
    startupTask:finishTask()
end))

event.setCallback("client.setmaster", function(cn, password, meta)
    local users = cubescript.getVar("users")
    if type(users) ~= "table" then
        print "WARNING: users table is not set"
    end
    meta.returnValue = -1
    if password == "" then --/setmaster 1
        for k, user in pairs(users) do
            if user.password == "" then
                meta.returnValue = user.value
                return
            end
        end
    else
        for k, user in pairs(users) do
            if server.checkpassword(cn, user.password, password) then
                meta.returnValue = user.value
                return
            end
        end
    end
    
    return 0
end)

event.setCallback("server.listen", function(port, extinfoPort, lanPort)
    serverApi:print(("Listening on port %i (extinfo: %i, laninof: %i)"):format(port, extinfoPort, lanPort))
end)

event.setCallback("server.start", function()
    serverApi:print("Lua initialized")
end)

local shutdownTask
event.setCallback("server.stop", function()
    if shutdownTask then
        serverApi:print "Forced shutdown .."
        return server.shutdownFinished()
    end

    serverApi:print "Shutting down ..."
    
    shutdownTask = task.createTask(function()
        server.shutdownFinished()
    end)
    
    shutdownTask:addTask()
    moduleContainer:unloadAll(shutdownTask)
    shutdownTask:finishTask()
end)

event.setCallback("server.afterStop", function()
    serverApi:print "Exit"
    process.exit()
end)

event.setCallback("server.getVar", function(var, meta)
    meta.returnValue = cubescript.getVar(({
        ["server.name"] = "servername"
    })[var])
end)