
local SERVER = SERVER
local CLIENT = CLIENT
local LUAPP = LUAPP

local traceback = require('debug').traceback

-- This is called by all the event sources from C
-- The user can override it to hook into event sources
function eventSource(name, fn, ...)
  local args = {...}
  return assert(xpcall(function ()
    return fn(unpack(args))
  end, traceback))
end

errorMeta = {__tostring=function(table) return table.message end}

local realAssert = assert
function assert(good, error)
  return realAssert(good, tostring(error))
end

require "luvit"

local exceptions = {
	_yajl = true,
}

setmetatable(_G, {
    __index = function(self, index)
    	if not exceptions[index] then
	        require "luvit.utils".warning(("Attempting to index a global (%s)"):format(index))
	    end
    end,

    __newindex = function(self, index, value)
    	if not exceptions[index] then
        	local utils = require "luvit.utils"
        	require "luvit.utils".warning(("Attempting to set a global (%s = %s)"):format(index, utils.dump(value)))
		end
    end,
})

eventSource("load", function()
    if SERVER then
        require "core.server"
    elseif CLIENT then
        require "core.client"
    elseif LUAPP then
        require "core.luapp"
    end
end)
