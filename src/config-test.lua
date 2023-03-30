--VERSION and DATASIZE added by build.sh

function set_runpath(tab)
    if (tab.options[3].value == true) and (tab.options[3].disabled == false) then
        if os.execute("groups games &> /dev/null") == 0 then
            if os.execute("chown -R games:games " .. MojoSetup.destination) == 0 then
                os.execute("find \"" .. MojoSetup.destination .. "\" -type d -exec chmod g+s {} \\;")
                os.execute("chmod u+s \"" .. MojoSetup.destination .. "/savage2.bin\" \"" .. MojoSetup.destination .. "/savage2_update.bin\"")
            end
        end
    end
    os.execute(MojoSetup.destination .. "/savage2_update.bin --update-runpath")
    return nil
end

Setup.Package
{
    vendor = "s2games.com",
    id = "Savage2Test",
    description = "Savage 2 - A Tortured Soul (Test Build)",
    version = VERSION,
    use_manifest_for_uninstall = false, -- just delete everything under the installed directory
    postinstall = set_runpath,
    
    recommended_destinations = 
    {
        MojoSetup.info.homedir,
        MojoSetup.info.homedir .. "/games",
        "/opt",     
        "/usr/local",
        "/opt/games",
        "/usr/local/games"
    },
    splash = "splash.png",
    
    Setup.Eula
    {
        description = "Terms of Service",
        source = "license.txt"
    },
    
    Setup.Option
    {
        required = false,
        value = true,
        bytes = 0,
        description = "XDG Menu Entries",
        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Savage 2 (Test Build)",
            genericname = "Multiplayer Game",
            tooltip = "Savage 2 - A Tortured Soul test client",
            builtin_icon = false,
            icon = "s2icon.png",
            commandline = "%0/savage2.bin",
            category = "Game"
        },
    },
    
    Setup.Option
    {
        required = true,
        bytes = DATASIZE,
        description = "Savage 2 Test",
        Setup.File
        { 
            filter = function(fn)
                if fn == "savage2" or string.find(fn, ".sh") or string.find(fn, ".bin") or string.find(fn, ".so") then return fn,"0755" end
                return fn,"0644"
            end
        }
    },
    
    Setup.Option
    {
        required = false,
        disabled = (MojoSetup.info.uid ~= 0),
        value = true,
        bytes = 0,
        description = "Allow all users to update Savage 2",
    }
}
