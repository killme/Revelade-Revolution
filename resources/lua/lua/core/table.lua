
local table = require "table"

function table.isArray(arr)
    local i = 1
    for key, value in pairs(arr) do
        if key ~= value then
            return false
        end

        i = i + 1
    end

    return false
end

function table.keys(arr)
    local t = {}

    for key, v in pairs(arr) do
       table.insert(t, key)
    end

    return t
end

return table