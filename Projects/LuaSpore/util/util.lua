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

local function SortByTypeAndValue(a, b)
    local typea, typeb = type(a), type(b)

	-- Prevent lua error when both types are tables
	if typea == "table" and typeb == "table" then
		return false
	end
    return typea < typeb or (typea == typeb and a < b)
end

function dumptable(obj, recurse_levels, indent, visit_table, is_terse)
    local is_top_level = visit_table == nil
    if visit_table == nil then
        visit_table = {}
    end

    indent = indent or 1
    local i_recurse_levels = recurse_levels or 5
    if obj then
        local dent = string.rep("\t", indent)
        if type(obj) == "string" then
            print(obj)
            return
        end
        if type(obj) == "table" then
            if visit_table[obj] ~= nil then
                print(dent.."(Already visited",obj,"-- skipping.)")
                return
            else
                visit_table[obj] = true
            end
        end
        local keys = {}
        for k,v in pairs(obj) do
            table.insert(keys, k)
        end
        table.sort(keys, SortByTypeAndValue)
        if not is_terse and is_top_level and #keys == 0 then
            print(dent.."(empty)")
        end
        for i,k in ipairs(keys) do
            local v = obj[k]
            if type(v) == "table" and i_recurse_levels>0 then
                print(dent.."K: ",k," V: ", v)
                dumptable(v, i_recurse_levels-1, indent+1, visit_table)
            else
                print(dent.."K: ",k," V: ",v)
            end
        end
    elseif not is_terse then
        print("nil")
    end
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
            local result = callback(...)
            if result ~= nil then
                return result
            end
        end
    end
    return AddCallback, RemoveCallback, ExecuteCallbacks
end

function GenerateOrderedCallbackExecuter()
    local callbacks = {}
    local function AddCallback(callback)
        table.insert(callbacks, callback)
    end
    local function RemoveCallback(callback)
        table.removearrayvalue(callbacks, callback)
    end
    local function ExecuteCallbacks(...)
        for i, callback in ipairs(callbacks) do
            local result = callback(...)
            if result ~= nil then
                return result
            end
        end
    end
    return AddCallback, RemoveCallback, ExecuteCallbacks
end