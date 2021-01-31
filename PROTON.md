# Proton Usage Guide

---

This is a basic guide to using the mod within Proton (aka Linux).

Please make sure your game works in proton _first_, before installing this mod.

---

## Enabling proton Experimental (Optional)

While this isn't a required step, it's useful for performance.

Right click the game in Steam and select 'Properties'. Go to the 'Compatibility' tab and tick 'Force the use of a specific Steam Play compatibility tool'. 

In the dropdown, select 'Proton Experimental'.

## Setting up the mod with ProtonTricks

First, follow the steps to install the mod. These are in the [wiki](https://github.com/yamashi/PerformanceOverhaulCyberpunk/wiki). 

Run `protontricks 1091500 --gui`, you may get an error saying "You are using a 64 bit WINEPREFIX", you can ignore this. Press 'Ok' when this error appears.

Choose the option 'Select the default wineprefix', then 'Install a Windows DLL or component'. Tick `vcrun2019` and press Ok. Go through the steps to install that runtime when the window opens. It's pretty simple (just click next really).

The options menu for Protontricks will show up again, this time select 'Run winecfg'. In the window that opens, select the 'Libraries' tab. In the dropdown on 'New override for library', select `version`. Click `Add`, then `Apply` and `Ok`.



You can now exit out of Protontricks and start your game. Check to see if the log exists after you run the game. If it's there, you've successfully installed the mod. 

## Setting up the mod with proton as non-Steam Game

Search for the game APPID with:
`$ protontricks -s name_of_the_game_on_your_steam_lib`

like:

`$ protontricks -s Cyberpunk 2077`

copy the APPID from the terminal then run:

`$ protontricks "APPID" --gui`

From now on, you can follow the Setting up the mod with ProtonTricks instructions.

## Setting up the mod with Winetricks

For users running on the GOG version of the game, via launcher like [Lutris](https://lutris.net/), [Minigalaxy](https://github.com/sharkwouter/minigalaxy) or [GameHub](https://github.com/tkashkin/GameHub), those running from the direct GOG Galaxy Windows application through Wine, or those directly using the Game libraries, Protontricks won´t be helpful in your case, as Protontricks will notice that you don´t have the game on Steam.

Instead, use Winetricks in which Protontricks is derived from. 

Either run Winetricks in itself if you use your default prefix, or from your custom prefix via: `WINEPREFIX=<Location of your WinePrefix directory> winetricks --gui` via the terminal.

From now on, you can follow the Setting up the mod with ProtonTricks instructions. If you want to verify in Winetricks has correctly taken in account your custom Wine directory, click on `Browse files` to see if it brings you to the designated folder.

## Troubleshooting

If for some reason protontricks crashes on start with the APPID, try installing it from the latest master, as described in the repo: https://github.com/Matoking/protontricks

## Thanks to

- [@bundyo](https://github.com/bundyo)

- [@okamidash (Author)](https://github.com/okamidash)

