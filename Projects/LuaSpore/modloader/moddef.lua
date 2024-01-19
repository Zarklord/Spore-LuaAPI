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
	RunInEnvironment(SporeLoadLua(self.modinfo_path), self.modinfo)

	if not self.modinfo.name or
	not self.modinfo.description or
	not self.modinfo.version or
	not self.modinfo.name or
	not self.modinfo.author then
		printf.ModLoader("Package %s had an invalid mod/info.lua", self.dbpf_name)
		self.invalid_modinfo = true
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
		},
		{
			__index = _G,
		}
	)

	RunInEnvironment(SporeLoadLua(self.modmain_path), self.mod_environment)
end

function ModDefinition.SortByPriority(a, b)
	local apriority = a.modinfo.priority
	local bpriority = b.modinfo.priority
	if apriority == bpriority then
		return tostring(a.modinfo.name) > tostring(b.modinfo.name)
	else
		return apriority  > bpriority
	end
end