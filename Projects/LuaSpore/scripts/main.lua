RequireOnAllThreads("luaspore/scripts/constants")
RequireOnAllThreads("luaspore/util/util")

RequireOnAllThreads("luaspore/scripts/logging")
require("luaspore/scripts/scheduler")
require("luaspore/scripts/update")
require("luaspore/scripts/gamelogic")

print("main.lua loaded with gametype: "..(GAMETYPE == GameType.Disk and "Disk" or "March2017"))

require("luaspore/modloader/modloader")