local module = require "core.server.module"

local MotdModule = module.BaseModule:extend()

MotdModule.name = "core.modules.motd"

function MotdModule:initialize(server)
    module.BaseModule.initialize(self, server)
    
    self:addServerListener("client.connect", function(player)
        self:sendMessageTo(player)
    end)
    
    local startupTask = server:getStartupTask()
    startupTask:addTask(self:loadConfiguration(function(err, config)
        if err then
            self:warning("Could not load configuration: "..tostring(err))
            return startupTask:finishTask()
        end
        
        if type(config.motd) ~= "string" then
            self:warning("motd is not defined.")
        else
            self.message = config.motd 
        end
        
        startupTask:finishTask()
    end))
end

function MotdModule:sendMessageTo(player)
    if self.message then
        self.server:getRawApi().sendServerMessageTo(player, self.message)
    end
end

return {
    Module = MotdModule,    
}