
local table = require "table"
local fs = require "luvit.fs"
local json = require "luvit.json"

local function loadConfiguration(path, callback)
    fs.readFile(path, function(error, data)
        if error then
            return callback(error)
        end

        local success, config
        success, error = pcall(function()
            config = json.parse(data, {allow_comments = true})
        end)
        
        if not success then
            callback("Could not read json string: \n"..tostring(error))
        else
            callback(nil, config)
        end
    end)
end

--[[!
    Loads a json configuration file.
    Tries to load the first existing file from the fallbacks list.

    @param fallbacks table a list of filenames to try to load.
    @param callback function (err, data) standard callback function containing a lua representation of the json config as the data.
]]
local function loadConfigurationWithFallback(fallbacks, callback, tried)
    tried = tried or {}
    
    if #fallbacks == 0 then
        return false, "No fallbacks left. Tried:\n"..table.concat(tried, "\n")
    end
    
    local firstPath = table.remove(fallbacks, 1)
    table.insert(tried, firstPath)
    
    loadConfiguration(firstPath, function(error, data)
        if error then
            local success, msg = loadConfigurationWithFallback(fallbacks, callback, tried)
            
            if not success then
                callback(msg, nil)
            end
        else
            callback(nil, data)
        end
    end)
    
    return true
end

return {
    loadConfigurationWithFallback = loadConfigurationWithFallback,
    loadConfiguration = loadConfiguration,
}
