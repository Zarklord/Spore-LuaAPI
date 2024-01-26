require("luaspore/scripts/constants")
require("luaspore/util/util")

require("luaspore/scripts/logging")
require("luaspore/scripts/scheduler")
require("luaspore/scripts/update")
require("luaspore/scripts/gamelogic")

print.Lua("main.lua loaded with gametype: "..(GAMETYPE == GameType.Disk and "Disk" or "March2017"))

require("luaspore/modloader/modloader")