
--[[
    _G.setVar and _G.getVar only know about already existing idents,
    We store our custom values in lua
]]

local _vars = {}
local _setVar = _G.setVar
local _getVar = _G.getVar

local function getVar(var)
   if type(_vars[var]) ~= "nil" then 
       return _vars[var]
   else
       return _getVar(var)
   end
end

local function setVar(var, value)
    if not _setVar(var, value) then
        _vars[var] = value
    end
end

return {
    getVar = getVar,
    setVar = setVar
}