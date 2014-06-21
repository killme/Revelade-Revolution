local os = require "os"
local table = require "table"

local config = require "core.server.config"
local event = require "core.server.event"

local process = require "luvit.process"

local Object = require "luvit.core".Object
local Emitter = require "luvit.core".Emitter

local BaseModule = Emitter:extend()

function BaseModule:initialize(server)
    self.server = server
    self._listeners = {}
end

function BaseModule:addServerListener(name, callback)
    local listener = event.setCallback(name, callback)
    table.insert(self._listeners, self._listeners)
    return listener
end

function BaseModule:removeSeverListener(id)
    for k, v in pairs(self._listeners) do
        if v == id then
            self._listeners[k] = nil
            event.removeCallback(id)
            break;
        end
    end
end

function BaseModule:load()
    
end

function BaseModule:unload()
    for k, v in pairs(self._listeners) do
        self:removeSeverListener(v)
    end
end

function BaseModule:loadConfiguration(callback)
    config.loadConfiguration(("resources/config/%s.json"):format(self.name), callback)
end

function BaseModule:warning(msg)
    process.stderr:write(("[%s] Warning (%s): %s"):format(os.date("%Y-%m-%dT%H:%M:%S"), self.name, msg))
end

function BaseModule:info(msg)
    process.stdout:write(("[%s] Info (%s): %s"):format(os.date("%Y-%m-%dT%H:%M:%S"), self.name, msg))
end

local moduleContainer

local ModuleContainer = Object:extend()

function ModuleContainer:register(name, module)
    self._modules = self._modules or {}
    self._modules[name] = module
    module:load()
end

function ModuleContainer:unload(name, ...)
    if self._modules[name] then
        self._modules[name]:unload(...)
        self._modules[name] = nil
    end
end

function ModuleContainer:getModule(name)
    return self._modules[name]
end

function ModuleContainer:unloadAll(...)
    for name, v in pairs(self._modules) do
        print (("[%s] ~ Unloading module: %s"):format(os.date("%Y-%m-%dT%H:%M:%S"), name))
        self:unload(name, ...)
    end
end

return {
    moduleContainer = moduleContainer,
    BaseModule = BaseModule,
    createModuleContainer = function()
        if not moduleContainer then
            moduleContainer = ModuleContainer:new()
        end
        
        return moduleContainer
    end
}       