require("luaspore/util/class")
require("luaspore/util/table")
require("luaspore/util/string")

function deepcopy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

function shallowcopy(orig, dest)
    local copy
    if type(orig) == 'table' then
        copy = dest or {}
        for k, v in pairs(orig) do
            copy[k] = v
        end
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

-- setfenv and getfenv for lua 5.2 and up
-- taken from https://leafo.net/guides/setfenv-in-lua52-and-above.html
function setfenv(fn, env)
	local i = 1
	while true do
		local name = debug.getupvalue(fn, i)
		if name == "_ENV" then
			debug.upvaluejoin(fn, i, (function()
				return env
			end), 1)
			break
		elseif not name then
			break
		end

		i = i + 1
	end

	return fn
end

function getfenv(fn)
	local i = 1
	while true do
		local name, val = debug.getupvalue(fn, i)
		if name == "_ENV" then
			return val
		elseif not name then
			break
		end
		i = i + 1
	end
end

function RunInEnvironment(fn, fnenv)
	setfenv(fn, fnenv)
	return xpcall(fn, function(message) print(debug.traceback(message), 2) end)
end

function FunctionOrValue(func_or_val, ...)
    if type(func_or_val) == "function" then
        return func_or_val(...)
    end
    return func_or_val
end

function GenerateCallbackExecuter()
    local callbacks = {}
    local function AddCallback(callback)
        callbacks[callback] = true
    end
    local function RemoveCallback(callback)
        callbacks[callback] = false
    end
    local function ExecuteCallbacks(...)
        for callback in pairs(callbacks) do
            callback(...)
        end
    end
    return AddCallback, RemoveCallback, ExecuteCallbacks
end