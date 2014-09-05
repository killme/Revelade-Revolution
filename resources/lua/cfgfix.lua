--[[
    Script to dump texture information

    Purpose: Dump all information of all textures in the default config to the default output
]]


local math = require "math"

local fs = require "luvit.fs"
local json = require "luvit.json"

local cubescript = require "cubescript"
local stdlib = require "cubescript.stdlib"

local utils = require "luvit.utils"
utils.DUMP_MAX_DEPTH = 5

local env = cubescript.createEnvironment()

for k, v in pairs(stdlib.api) do
    env:register(k, v)
end

p(getmetatable(env))

local function executeFile(file)
    return env:run(fs.readFileSync(file))
end

local function noop() return "" end

env:register("maptitle", noop)

env:register("exec", function(env, scope, trace, file)
    return executeFile(file)
end)

env:register("mapmodelreset", noop)
env:register("materialreset", noop)
env:register("mmodel", noop)
env:register("skybox", noop)

local function textureType(type)
    return ({
        [0] = "c",
    })[type] or type
end

local textures = {}
local textureIndex = 0

local definedShaders = {}
local currentShader = "stdworld"

env:register("texturereset", function() textureIndex = 0 textures = {} return "" end)

env:register("setshader", function(env, scope, trace, shader)
    if not definedShaders[shader] then definedShaders[shader] = { name = shader } end
    currentShader = shader
    return shader
end)

env:register("setshaderparam", function(env, scope, trace, name, ...)
    definedShaders[currentShader][name] = {...}
    return name
end)

env:register("texture", function(evn, scope, trace, type, name, rot, xoffset, yoffset, scale)
    local shaderSettings = {}

    for k, v in pairs(definedShaders[currentShader] or {}) do
        shaderSettings[k] = v
    end

    type = textureType(type)

    local texture = {
        index   = textureIndex,
        type    = type,
        name    = name,
        rot     = env:toNumber(rot, 0),
        xoffset = env:toNumber(xoffset, 0),
        yoffset = env:toNumber(yoffset, 0),
        scale   = env:toNumber(scale, 1),
        shaderSettings = shaderSettings,
        derrivatives = {}
    }

    if type == "c" or type =="water" or type == "lava" then
        textureIndex = textureIndex + 1
        textures[textureIndex] = texture
    else
        textures[textureIndex].derrivatives[type] = texture
    end

    return tostring(textureIndex)
end)

env:register("texlayer", function(env, scope, trace, layer, name, mode, scale)
    local t = textures[textureIndex]

    t.variants = t.variants or {}
    t.variants.layer    = layer < 0 and math.max(textureIndex+layer, 0) or layer
    t.layermaskname     = name and "packages/"..name or nil
    t.layermaskmode     = mode
    t.layermaskscale    = env:toNumber(scale, 0) <= 0 and 1 or scale

    return ""
end)

env:register("nextslotindex", function() return textureIndex - 1 end)

executeFile("data/default_map_settings.cfg")

for k, v in pairs(textures) do
    utils.prettyPrint(v)
end
