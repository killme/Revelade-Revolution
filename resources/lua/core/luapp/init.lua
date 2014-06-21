--local process = require "core.luvit.process"
local native = require "uv_native"
local process = require "luvit.process"

if not process.argv[1] then
    print (("Usage: %s <script file> [<Arguments>...]"):format(process.argv[0]))
else
    
    process.luappArgv = process.argv
    
    local newArgs = {}
    for k, v in pairs(process.argv) do
        if k > 1 then
            newArgs[k-1] = v
        end
    end
    process.argv = newArgs
    
    require(process.luappArgv[1])
end

-- Start the event loop
native.run()

process.exit(process.exitCode or 0)