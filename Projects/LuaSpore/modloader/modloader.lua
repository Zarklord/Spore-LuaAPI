require("luaspore/modloader/moddef")

local moddefs = {}

for i, dbpf_name in ipairs(GetSporeDBPFNames()) do
	local modinfo_path = dbpf_name.."/mod/info"
	if SporeLuaExists(modinfo_path) then
		table.insert(moddefs, ModDefinition(dbpf_name))
	end
end

table.sort(moddefs, ModDefinition.SortByPriority)

ExecuteOnAllThreads(function()
	local _ModLoader = Class(function(self)
		self.mod_envs = {}
	end)

	function _ModLoader:SetModEnviroment(mod, env)
		self.mod_envs[mod] = env
	end

	function _ModLoader:GetModEnviroment(mod)
		return self.mod_envs[mod]
	end

	ModLoader = _ModLoader()
end)

for i, moddef in ipairs(moddefs) do
	moddef:LoadMod()
end