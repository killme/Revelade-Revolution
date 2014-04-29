--[[

Copyright 2012 The Luvit Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

--]]

-- Bootstrap require system
local native = require('uv_native')
local process = {
  execPath = native.execpath(),
  cwd = getcwd,
  argv = argv
}

package.preload["luvit.process"] = function ()
    return process
end

_G.getcwd = nil
_G.argv = nil
require = require('luvit.module').require

local Emitter = require('luvit.core').Emitter
local timer = require('luvit.timer')
local env = require('env')
local constants = require('constants')
local uv = require('luvit.uv')
local utils = require('luvit.utils')

setmetatable(process, {
  __index = function (table, key)
    if key == "title" then
      return native.getProcessTitle()
    else
      return Emitter[key]
    end
  end,
  __newindex = function (table, key, value)
    if key == "title" then
      return native.setProcessTitle(value)
    else
      return rawset(table, key, value)
    end
  end,
  __pairs = function (table)
    local key = "title"
    return function (...)
      if key == "title" then
        key = next(table)
        return "title", table.title
      end
      if not key then return nil end
      local lastkey = key
      key = next(table, key)
      return lastkey, table[lastkey]
    end
  end
})

-- Replace lua's stdio with luvit's
-- leave stderr using lua's blocking implementation
process.stdin = uv.createReadableStdioStream(0)
process.stdout = uv.createWriteableStdioStream(1)
process.stderr = uv.createWriteableStdioStream(2)

-- clear some globals
-- This will break lua code written for other lua runtimes
_G.io = nil
_G.os = nil
_G.math = nil
_G.string = nil
_G.coroutine = nil
_G.jit = nil
_G.bit = nil
_G.debug = nil
_G.table = nil
_G.loadfile = nil
_G.dofile = nil
_G.print = utils.print
_G.p = utils.prettyPrint
_G.debug = utils.debug

-- Move the version variables into a table
process.version = VERSION
process.versions = {
  luvit = VERSION,
  uv = native.VERSION,
  luajit = LUAJIT_VERSION,
  yajl = YAJL_VERSION,
  zlib = ZLIB_VERSION,
  http_parser = HTTP_VERSION,
  openssl = OPENSSL_VERSION,
}
_G.VERSION = nil
_G.YAJL_VERSION = nil
_G.LUAJIT_VERSION = nil
_G.UV_VERSION = nil
_G.HTTP_VERSION = nil
_G.ZLIB_VERSION = nil
_G.OPENSSL_VERSION = nil

-- Add a way to exit programs cleanly
local exiting = false
function process.exit(exit_code)
  if exiting == false then
    exiting = true
    process:emit('exit', exit_code or 0)
  end
  exitProcess(exit_code or 0)
end

function process:addHandlerType(name)
  local code = constants[name]
  if code then
    native.activateSignalHandler(code)
  end
end

function process:missingHandlerType(name, ...)
  if name == "error" then
    error(...)
  elseif name == "SIGINT" or name == "SIGTERM" then
    process.exit()
  end
end

function process.nextTick(callback)
  timer.setTimeout(0, callback)
end

process.kill = native.kill

-- Add global access to the environment variables using a dynamic table
process.env = setmetatable({}, {
  __pairs = function (table)
    local keys = env.keys()
    local index = 0
    return function (...)
      index = index + 1
      local name = keys[index]
      if name then
        return name, table[name]
      end
    end
  end,
  __index = function (table, name)
    return env.get(name)
  end,
  __newindex = function (table, name, value)
    if value then
      env.set(name, value, 1)
    else
      env.unset(name)
    end
  end
})

--Retrieve PID
process.pid = native.getpid()

-- Copy date and time over from lua os module into luvit os module
local OLD_OS = require('os')
local OS_BINDING = require('os_binding')
package.loaded.os = OS_BINDING
package.preload.os_binding = nil
package.loaded.os_binding = nil
OS_BINDING.date = OLD_OS.date
OS_BINDING.time = OLD_OS.time
OS_BINDING.clock = OLD_OS.clock
OS_BINDING.execute = OLD_OS.execute


-- Ignore sigpipe and exit cleanly on SIGINT and SIGTERM
-- These shouldn't hold open the event loop
if OS_BINDING.type() ~= "win32" then
  native.activateSignalHandler(constants.SIGPIPE)
  native.activateSignalHandler(constants.SIGINT)
  native.activateSignalHandler(constants.SIGTERM)
end

local traceback = require('debug').traceback

-- This is called by all the event sources from C
-- The user can override it to hook into event sources
function eventSource(name, fn, ...)
  if not fn then
     p("ERROR: missing function to eventSource", name, fn, ...)
     return
  end
    
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
