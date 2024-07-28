local getinfo = debug.getinfo
local max = math.max
local concat = table.concat

local function filtersource(src)
	if not src then return "[?]" end
	if src:sub(1, 1) == "@" then
			src = src:sub(2)
	end
	return src
end

local function formatinfo(info)
	if not info then return "**error**" end
	local source = filtersource(info.source)
	if info.currentline then
			source = source..":"..info.currentline
	end
	return ("\t%s in (%s) %s (%s) <%d-%d>"):format(source, info.namewhat, info.name or "?", info.what, info.linedefined, info.lastlinedefined)
end

function debugstack(start, top, bottom)
	if not bottom then bottom = 10 end
	if not top then top = 12 end
	start = (start or 1) + 1

	local count = max(2, start)
	while getinfo(count) do
		count = count + 1
		if count > 1024 then
			-- In a stack overflow, getinfo will keep returning values up
			-- to our massive stack limit which produces too many results
			-- and makes this function take too long to be useful. Set a
			-- high limit to prevent over doing it, but still return
			-- results in under a second.
			break
		end
	end

	count = count - start

	if top + bottom >= count then
		top = count
		bottom = nil
	end

	local s = {"stack traceback:"}
	for i = 1, top, 1 do
		s[#s + 1] = formatinfo(getinfo(start + i - 1))
	end
	if bottom then
		s[#s + 1] = "\t..."
		for i = bottom , 1, -1 do
				s[#s + 1] = formatinfo(getinfo(count - i + 1))
		end
	end

	return concat(s, "\n")
end

function debugstack_oneline(linenum)
    local num = linenum or 3
    return formatinfo(getinfo(num))
end

stacktrace = {}

local function SaveToString(v)
	local _, retval = xpcall(function() return tostring(v) end, function() return "*** failed to evaluate ***" end)
	local maxlen = 1024
	if retval:len() > maxlen then
		retval = retval:sub(1,maxlen).." [**truncated**]"
	end
	return retval
end

local function getdebuglocals(res, level)
	local function format_local_value(prefix, name, value)
		if type(value) == "function" then
			local info = getinfo(value, "LnS")
			res[#res+1] = string.format("%s%s = function - %s", prefix, name, info.source..":"..tostring(info.linedefined))
		else
			res[#res+1] = string.format("%s%s = %s", prefix, name, SaveToString(value))
		end
	end

	local t = {}
	local index = 1
	while true do
		local name, value = debug.getlocal(level + 1, index)
		if not name then
			break
		end
		-- skip compiler generated variables
		if name:sub(1, 1) ~= "(" then
			if name == "self" and type(value) == "table" then
				res[#res+1] = string.format("   self =")
				for k, v in pairs(value) do
					format_local_value("      ", k, v)
				end
			else
				format_local_value("   ", name, value)
			end
		end
		index = index + 1
	end
	return table.concat(t, "\n")
end

local function getdebugstack(res, start, top)
	-- disable strict. We may hit G
	setmetatable(_G, {})

	if not top then top = 12 end
	start = (start or 1) + 1

	local count = max(2, start)
	while getinfo(count) do
		count = count + 1
	end

	count = count - start
	top = math.min(top, count)

	for i = 1, top, 1 do
		res[#res+1] = formatinfo(getinfo(start + i - 1))
		getdebuglocals(res, start + i - 1)
	end
	return res
end

local function DoStackTrace(err)
	local res = {}
	if err then
		for idx, line in ipairs(err:split("\n")) do
			res[#res+1] = "#"..line
		end
		res[#res+1] = "#LUA ERROR stack traceback:"
	end
	res = getdebugstack(res, 5)
	return concat(res, "\n")
end

function stacktrace.StackTrace(err)
	local panic_str = tostring(err).."\nLUA ERROR "..debugstack()
	local _, retval = xpcall(function() return DoStackTrace(err) end, function() return panic_str end)
	return retval
end

_TRACEBACK = stacktrace.StackTrace