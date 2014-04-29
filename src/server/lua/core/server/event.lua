local rawSetCallback = setCallback
_G.setCallback = nil

local callbacks = {}
local function setCallback(name, callback)
    if not callbacks[name] then
        callbacks[name] = {}
        rawSetCallback(name, function(...) --TODO: removecallback when no callbacks left
            local args = {...}
            local metaArgI = #args + 1
            args[metaArgI] = {returnValue = nil}
            for id, callback in pairs(callbacks[name]) do
                args[metaArgI].id = id
                callback(unpack(args))
            end
            
            return args[metaArgI].returnValue
        end)
    end
    
    local id = #callbacks[name] + 1
    callbacks[name][id] = callback
    return id
end

local function removeCallback(name, id)
    if callbacks[name] then
        callbacks[name][id] = nil
    end
end

return {
    setCallback = setCallback,
    removeCallback = removeCallback,
    rawSetCallback = rawSetCallback,    
}