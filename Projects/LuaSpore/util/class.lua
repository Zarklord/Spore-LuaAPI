local function _is_a(self, klass)
	local m = getmetatable(self)
	while m do
		if m == klass then return true end
		m = m._base
	end
	return false
end

function Class(base, _ctor)
    local c = {}
	if not _ctor and type(base) == 'function' then
        _ctor = base
        base = nil
    elseif type(base) == 'table' then
        for i,v in pairs(base) do
            c[i] = v
        end
        c._base = base
    end

	c.__index = c

    local mt = {__call = function(class_tbl, ...)
		local obj = setmetatable({}, class_tbl)
		if class_tbl._ctor then
			class_tbl._ctor(obj, ...)
		end
		return obj
	end}

    c._ctor = _ctor
	c.is_a = _is_a
	c.is_class = true
	c.is_instance = function(obj)
		return type(obj) == "table" and _is_a(obj, c)
	end

    setmetatable(c, mt)
    return c
end