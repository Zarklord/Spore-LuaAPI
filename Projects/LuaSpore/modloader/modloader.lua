require("luaspore/modloader/moddef")

local moddefs = {}

for i, dbpf_name in ipairs(GetSporeDBPFNames()) do
	local modinfo_path = dbpf_name.."/mod/info"
	if SporeLuaExists(modinfo_path) then
		table.insert(moddefs, ModDefinition(dbpf_name))
	end
end

table.sort(moddefs, ModDefinition.SortByPriority)

for i, moddef in ipairs(moddefs) do
	moddef:LoadMod()
end