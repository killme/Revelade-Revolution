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
local Emitter = require('luvit.core').Emitter

local Process = Emitter:extend()
local process = Process:new()
process.execPath = native.execpath()
process.cwd = getcwd
process.argv = argv

_G.getcwd = nil
_G.argv = nil

package.preload["luvit.process"] = function ()
    return process
end

require = require('luvit.module').require
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

function signalStringToNumber(name)
  if name == 'SIGHUP' then
    return constants.SIGHUP
  elseif name == 'SIGINT' then
    return constants.SIGINT
  elseif name == 'SIGQUIT' then
    return constants.SIGQUIT
  elseif name == 'SIGILL' then
    return constants.SIGILL
  elseif name == 'SIGTRAP' then
    return constants.SIGTRAP
  elseif name == 'SIGABRT' then
    return constants.SIGABRT
  elseif name == 'SIGIOT' then
    return constants.SIGIOT
  elseif name == 'SIGBUS' then
    return constants.SIGBUS
  elseif name == 'SIGFPE' then
    return constants.SIGFPE
  elseif name == 'SIGKILL' then
    return constants.SIGKILL
  elseif name == 'SIGUSR1' then
    return constants.SIGUSR1
  elseif name == 'SIGSEGV' then
    return constants.SIGSEGV
  elseif name == 'SIGUSR2' then
    return constants.SIGUSR2
  elseif name == 'SIGPIPE' then
    return constants.SIGPIPE
  elseif name == 'SIGALRM' then
    return constants.SIGALRM
  elseif name == 'SIGTERM' then
    return constants.SIGTERM
  elseif name == 'SIGCHLD' then
    return constants.SIGCHLD
  elseif name == 'SIGSTKFLT' then
    return constants.SIGSTKFLT
  elseif name == 'SIGCONT' then
    return constants.SIGCONT
  elseif name == 'SIGSTOP' then
    return constants.SIGSTOP
  elseif name == 'SIGTSTP' then
    return constants.SIGSTSP
  elseif name == 'SIGTTIN' then
    return constants.SIGTTIN
  elseif name == 'SIGTTOU' then
    return constants.SIGTTOU
  elseif name == 'SIGURG' then
    return constants.SIGURG
  elseif name == 'SIGXCPU' then
    return constants.SIGXCPU
  elseif name == 'SIGXFSZ' then
    return constants.SIGXFSX
  elseif name == 'SIGVTALRM' then
    return constants.SIGVTALRM
  elseif name == 'SIGPROF' then
    return constants.SIGPROF
  elseif name == 'SIGWINCH' then
    return constants.SIGWINCH
  elseif name == 'SIGIO' then
    return constants.SIGIO
  elseif name == 'SIGPOLL' then
    return constants.SIGPOLL
  elseif name == 'SIGLOST' then
    return constants.SIGLOST
  elseif name == 'SIGPWR' then
    return constants.SIGPWR
  elseif name == 'SIGSYS' then
    return constants.SIGSYS
  elseif name == 'SIGUNUSED' then
    return constants.SIGUNUSED
  end
  return nil
end

--
process.signalWraps = {}
process.on = function(self, _type, listener)
  if _type:find('SIG') then
    local number = signalStringToNumber(_type)
    if number then
      local signal = process.signalWraps[_type]
      if not signal then
        signal = uv.Signal:new()
        process.signalWraps[_type] = signal
        signal:on('signal', function()
          self:emit(_type, number)
        end)
        signal:start(number)
      end
    end
  end
  Emitter.on(self, _type, listener)
end

process.removeListener = function(self, _type, callback)
  if _type:find('SIG') then
    local signal = process.signalWraps[_type]
    if signal then
      signal:stop()
      process.signalWraps[_type] = nil
    end
  end
  Emitter.removeListener(self, _type, callback)
end

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
--_G.loadfile = nil
--_G.dofile = nil
_G.printSync = _G.printSync or print
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

setmetatable(_G, {
  __index = function(self, key)
    process.stderr:write(traceback("Attepmting to get a global variable \"%s\""):format(tostring(key)).."\n")
  end,
  __newindex = function(self, key, value)
    process.stderr:write(traceback("Attepmting to set a global variable \"%s\""):format(tostring(key)).."\n")
  end,
})
