--VERSION and DATASIZE added by build.sh

function check_root(tab)
	if (MojoSetup.info.uid == 0) or (MojoSetup.info.euid == 0) then
		MojoSetup.fatal("Heroes of Newerth Beta must be installed as a regular (non-root) user.")
	end
end

Setup.Package
{
	vendor = "s2games.com",
	id = "HoN",
	description = "Heroes of Newerth",
	version = VERSION,
	
	precheck = check_root,
	
	recommended_destinations = 
	{
		MojoSetup.info.homedir,
		MojoSetup.info.homedir .. "/games" --,
		--"/opt",		
		--"/usr/local",
		--"/opt/games",
		--"/usr/local/games"
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
		description = "Menu Entries",
		Setup.DesktopMenuItem
		{
			disabled = false,
			name = "Heroes of Newerth",
			genericname = "Multiplayer Game",
			tooltip = "Heroes of Newerth",
			builtin_icon = false,
			icon = "icon.png",
			commandline = "%0/hon.sh",
			category = "Game"
		},
	},
	
	Setup.Option
	{
		required = true,
		bytes = DATASIZE,
		description = "Heroes of Newerth",
		Setup.File { }
	}
}
