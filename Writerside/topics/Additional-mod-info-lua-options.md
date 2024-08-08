# Additional mod/info.lua options
<link-summary>Additional options you can put in your mod's info file</link-summary>
<web-summary>Additional options you can put in your mod's info file</web-summary>
<card-summary>Additional options you can put in your mod's info file</card-summary>

This covers the additional options you can put in your mod's info file:
<deflist>
    <def title="cpp_mod_requirements">
        <p>A table containing a list of key value pairs defining the <tooltip term="CPP">CPP</tooltip> mods and their minimum required versions for this mod to work.</p>
        <code-block>
            cpp_mod_requirements = {
                ["SomeCPPModsName"] = 10000,
            }
        </code-block>
        <p>The above code would require a <tooltip term="CPP">CPP</tooltip> mod by the name of <code>SomeCPPModsName</code> that is atleast version <code>10000</code> to load this mod</p>
    </def>
    <def title="lua_mod_requirements">
        <p>A table containing a list of key value pairs defining the lua mods and their minimum required versions for this mod to work.</p>
        <code-block>
            lua_mod_requirements = {
                ["SomeLuaModsName"] = 10000,
            }
        </code-block>
        <p>The above code would require a lua mod by the name of <code>SomeLuaModsName</code> that is atleast version <code>10000</code> to load this mod</p>
    </def>
</deflist>