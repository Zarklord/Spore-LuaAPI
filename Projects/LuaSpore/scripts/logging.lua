local print_loggers = {}

function AddPrintLogger(fn)
    table.insert(print_loggers, fn)
end

local function packstring(...)
    local str = ""
    local n = select('#', ...)
    local args = {...}
    for i=1,n do
        str = str..tostring(args[i]).."\t"
    end
    return str
end

local function do_print(...)
    local str = packstring(...)

    for i,v in ipairs(print_loggers) do
        v(str)
    end
end

print = setmetatable({}, {
    __call = function(t, ...)
        return do_print(...)
    end,
    __index = function(t, channel)
        channel = string.format("[%s]", channel)
        return function(...) return do_print(channel, ...) end
    end
})

printf = setmetatable({}, {
    __call = function(t, ...)
        return do_print(string.format(...))
    end,
    __index = function(t, channel)
        channel = string.format("[%s]", channel)
        return function(...) return do_print(channel, string.format(...)) end
    end
})

local function disable() end

enable_print_channel = function(channel)
    print[channel] = nil
    printf[channel] = nil
end

disable_print_channel = function(channel)
    print[channel] = disable
    printf[channel] = disable
end

-- add our print loggers
AddPrintLogger(LuaPrint)