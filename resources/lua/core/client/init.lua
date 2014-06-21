local capi = require "core.engine.capi"

local path = require "luvit.path"
local table = require "core.table"
local process = require "luvit.process"
local fs = require "luvit.fs"
local json = require "luvit.json"

local data

local res = pcall(function()
    data = fs.readFileSync("bundles.json")
end)

if not res then
    res = pcall(function()
        data = fs.readFileSync("bundles.json.dist")
    end)
end

if not res then
    print "Could not open bundles.json or bundles.json.dist file. Are you running from the right directory?"
    return
end

local info = json.parse(data)

local paths = info.paths
local bundles = {}

local function loadBundle(bundleName)
    local bundle = {
        bundle = bundleName
    }
    table.insert(bundles, bundle)

    local loaded = false
    local tried = {}
    local data

    for i, _path in pairs(paths) do
        loaded = pcall(function()
            local curPath = path.join(_path, bundle.bundle, "info.json")
            table.insert(tried, curPath)
            data = fs.readFileSync(curPath)
        end)

        if loaded then

            local info = json.parse(data)
            bundle.info = info

            for i, bundle in pairs(info.require or {}) do
                loadBundle(bundle)
            end

            if type(info.files) == "table" and #info.files ~= 0 then
                for i, file in pairs(info.files) do
                    local curPath = path.join(_path, bundle.bundle, file)

                    if file == "bundle.zip" then
                        capi.cubescript(("addzip %q %q"):format(curPath, "@{"..bundle.bundle.."}"))
                        --capi.addzip(curPath, bundle, "")
                    else
                        --capi.registerBundleFile(bundle, file, curPath)
                    end
                end
            end

            break
        end
    end

    if not loaded then
        require "luvit.utils".warning(("Could not load %s:\nTried:\n\t %s\n"):format(bundleName, table.concat(tried, "\n\t")))
    end
end

capi.setEvent("resources_load", function()
    --Bundles
    for i, bundle in pairs(info.require) do
        loadBundle(bundle)
    end

    for i, bundle in pairs(bundles) do
        if bundle.info then
            for i, resource in pairs(bundle.info.resources or {}) do
                if not resource.type then
                    resource = {type = resource[1], path = resource[2], tags = resource[3]}
                end

                local path = "@{"..bundle.bundle.."}/"..resource.path

                if resource.type == "font" then
                    if resource.path:find("%.json$") then
                        --Parse json font format
                        --capi.cubescript(("newfont %q %q %i %i %i"):format())
                        --p(capi)
                    else
                        capi.exec(path..".cfg")
                    end
                elseif resource.type == "texture" then
                    --Special textures
                    if resource.path == "notexture" or resource.path == "notexture.png" or resource.tags == "notexture" then
                        capi.setNotextureFile(path);
                    else

                    end
                elseif resource.type == "fontdir" then
                    --loop dir and add all font configs
                elseif resource.type == "map" then
                    capi.registerMap(resource.path:gsub("map/", ""),  path)
                end
            end
        end
    end

    --Keymap
    local data = fs.readFileSync("resources/keymap.json")
    local info = json.parse(data, {allow_comments = true})
    for key, names in pairs(info) do
        if type(names) ~= "table" then
            names = { names }
        end

        for i, name in pairs(names) do
            capi.keymap(key, name)
        end
    end
end)

capi.setEvent("shaders_load", function()
    capi.exec("resources/shaders/glsl.cfg")
end)

capi.setEvent("cubescript_init", function()
    --Load default settings
    local data = fs.readFileSync("resources/config.json")
    local info = json.parse(data, {allow_comments = true})

    for key, value in pairs(info) do
        if key == "keys" then
            for keyName, command in pairs(value) do
                capi.bind(keyName, command)
            end
        else
            capi.cubescript (("%s %q"):format(key, value))
        end
    end
end)