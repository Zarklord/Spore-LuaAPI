function table.getkeys(t)
    local keys = {}
    for key,val in pairs(t) do
        table.insert(keys, key)
    end
    return keys
end

function table.reverse(tab)
    local size = #tab
    local newTable = {}

    for i,v in ipairs(tab) do
        newTable[size-i+1] = v
    end

    return newTable
end

function table.invert(t)
    local invt = {}
    for k, v in pairs(t) do
        invt[v] = k
    end
    return invt
end

function table.removearrayvalue(t, lookup_value)
    for i, v in ipairs(t) do
        if v == lookup_value then
            return table.remove(t, i)
        end
    end
end

function table.removetablevalue(t, lookup_value)
    for k, v in pairs(t) do
        if v == lookup_value then
            t[k] = nil
            return v
        end
    end
end

function table.reverselookup(t, lookup_value)
    for k, v in pairs(t) do
        if v == lookup_value then
            return k
        end
    end
    return nil
end