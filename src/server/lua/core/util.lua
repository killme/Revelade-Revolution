local table = require('core.table')

local util = {}

function util.nop()
end

function util.dump(value, depth)
    local valueType = type(value)
    local s = ""

    depth = depth or 1

    if valueType == "string" then
        s = ("%q"):format(value)
    elseif valueType == "number" then
        s = tostring(value)
    elseif valueType == "boolean" then
        s = value and "true" or "false"
    elseif valueType == "table" then
        local indent = ("    "):rep(depth)
        local parentIndent = ("    "):rep(depth-1)
        local isArray = table.isArray(value)

        s = s .. "{\n"

        if depth > 10 then
            s = " ... "
        else

            for key, row in pairs(value) do
                local line = ""
                if type(key) == "string" and key:find("^[%a_][%a%d_]*$") then
                    line = key .. " = "
                else
                    line = "[ ".. util.dump(key) .. " ] = "
                end

                if row == value then
                    line = line .. "RECURSION"
                else
                    line = line .. util.dump(row, depth + 1)
                end


                s = s .. indent .. line .. ",\n"
            end
        end

        s = s .. parentIndent .. "}"

    else
        s = "("..valueType..") "..tostring(value)
    end

    return s
end

return util
