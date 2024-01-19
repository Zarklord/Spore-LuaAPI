require("luaspore/scripts/constants")
require("luaspore/util/util")

require("luaspore/scripts/logging")
require("luaspore/scripts/scheduler")
require("luaspore/scripts/update")

print.Lua("main loaded with gametype: "..(GAMETYPE == GameType.Disk and "Disk" or "March2017"))

function ExecuteCheatCommand(fn)
    local status, r = pcall(fn)
    if not status then
        print(r)
    end
end

require("luaspore/modloader/modloader")