# Creating a new mod
<link-summary>Learn how to create a new Spore LuaAPI mod</link-summary>
<web-summary>Learn how to create a new Spore LuaAPI mod</web-summary>
<card-summary>Learn how to create a new Spore LuaAPI mod</card-summary>

Creating a new mod is simple:

1. Open [SporeModderFX](https://emd4600.github.io/SporeModder-FX/)
2. Create a new project in SMFX.
3. Select <ui-path>Project | Project Settings</ui-path> and set <ui-path>Packing path</ui-path> to <control>Spore</control> 
4. Create a folder in that project called <path>mod</path>
5. Create two files in that folder: <path>main.lua</path> and <path>info.lua</path>
6. Fill out the following information in <path>info.lua</path>:
```
name = "Your mod name"
description = "Your mod description"
version = 10000 --this version must be a number, it cannot be a string
author = "Author"

--the minimum lua api version required to run this mod
--intended to prevent mods from running if the lua api is too old
--10000 is the oldest version of the lua api
base_api_version = 10000

--set the load priority of your mod, the higher priority = mod loads sooner
--you should make sure your priority is lower than your mod dependencies
priority = 0
```
<tip>
Checkout <a href="Additional-mod-info-lua-options.md"/> for other options you can set.
</tip>
Upon packing the mod, your mod (which currently does nothing) should be loaded by the game.
To verify this you can check your <tooltip term="spore_log">spore_log.txt</tooltip> for the following message:
<code-block>
[Lua] [ModLoader]	Mod: &lt;PackageName&gt; (&lt;ModName&gt;) Loading mod/main.lua	
</code-block>
Where <code>&lt;PackageName&gt;</code> is the name of your <control>SMFX Project</control>, and <code>&lt;ModName&gt;</code> is the name you put in <path>mod/info.lua</path>

At this point you can add your code to <path>mod/main.lua</path> and start writing your mod.

<seealso style="cards">
    <category ref="features">
        <a href="Additional-mod-info-lua-options.md"/>
    </category>
</seealso>