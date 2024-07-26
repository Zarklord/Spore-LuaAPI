ModDefinition = Class(function(self, dbpf_name)
	self.dbpf_name = dbpf_name
	self.modinfo_path = dbpf_name.."/mod/info"
	self.modmain_path = dbpf_name.."/mod/main"

	assert(SporeLuaExists(self.modinfo_path), "missing mod/info for mod definition")
	assert(SporeLuaExists(self.modmain_path), "missing mod/main for mod definition")

	self:LoadModInfo()
end)

function ModDefinition:LoadModInfo()
	self.modinfo = {}
	local modinfo_fn = SporeLoadLua(self.modinfo_path)
	if modinfo_fn then
		RunInEnvironment(modinfo_fn, self.modinfo)
	else
		printf.ModLoader("Package %s mod/info.lua has a syntax error", self.dbpf_name)
		self.invalid_modinfo = true
	end

	if not self.modinfo.name or
	not self.modinfo.description or
	not self.modinfo.version or
	not self.modinfo.name or
	not self.modinfo.author then
		printf.ModLoader("Package %s has an invalid mod/info.lua", self.dbpf_name)
		self.invalid_modinfo = true
	end

	if self.modinfo.base_api_version then
		local cur_version = self.LoadedCPPMods["SporeLuaAPI"]
		if self.modinfo.base_api_version > cur_version then
			printf.ModLoader("Package %s could not be loaded because the SporeLuaAPI version %d is too low, expected: %d", self.dbpf_name, cur_version, self.modinfo.base_api_version)
			self.invalid_modinfo = true
		end
	end

	if self.modinfo.cpp_mod_requirements then
		for modname, min_version in pairs(self.modinfo.cpp_mod_requirements) do
			local cur_version = self.LoadedCPPMods[modname]
			if not cur_version then
				printf.ModLoader("Package %s could not be loaded because the cpp mod '%s' was missing", self.dbpf_name, modname)
				self.invalid_modinfo = true
			elseif min_version > cur_version then
				printf.ModLoader("Package %s could not be loaded because the cpp mod '%s' version %d is too low, expected: %d", self.dbpf_name, modname, cur_version, min_version)
				self.invalid_modinfo = true
			end
		end
	end

	self.priority = self.modinfo.priority or 0
end

function ModDefinition:LoadMod()
	if self.invalid_modinfo then return end
	printf.ModLoader("Mod: %s (%s) Loading mod/main.lua", self.dbpf_name, self.modinfo.name)

	self.mod_environment = setmetatable(
		{
			_G = _G,
			modinfo = self.modinfo,
			dbpf_name = self.dbpf_name,
			require = function(path)
				if path:count("/") == 1 then
					require(self.dbpf_name.."/"..path)
				else
					require(path)
				end
			end,
			RequireOnAllThreads = function(path)
				if path:count("/") == 1 then
					RequireOnAllThreads(self.dbpf_name.."/"..path)
				else
					RequireOnAllThreads(path)
				end
			end,
		},
		{
			__index = _G,
		}
	)

	local mod_fn = SporeLoadLua(self.modmain_path)
	if mod_fn then
		RunInEnvironment(mod_fn, self.mod_environment)
	else
		printf.ModLoader("Mod: %s (%s) Failed to load mod/main.lua due to a syntax error", self.dbpf_name, self.modinfo.name)
	end
end

ModDefinition.LoadedCPPMods = GetCPPMods()

function ModDefinition.SortByPriority(a, b)
	local apriority = a.modinfo.priority
	local bpriority = b.modinfo.priority
	if apriority == bpriority then
		return tostring(a.modinfo.name) > tostring(b.modinfo.name)
	else
		return apriority  > bpriority
	end
end