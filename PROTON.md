# Proton Usage Guide

---

This is a basic guide to using the mod within Proton (aka linux).

Please make sure your game works in proton _first_, before installing this mod.

---

## Enabling proton Experimental (Optional)

While this isn't a required step, it's useful for performance.

Right click the game in steam and select 'Properties'. Go to the 'Compatibility' tab and tick 'Force the use of a specific Steam Play compatibility tool'. 

In the dropdown, select 'Proton Experimental'.

## Setting up the mod with ProtonTricks

First, follow the steps to install the mod. These are in the [wiki](https://github.com/yamashi/PerformanceOverhaulCyberpunk/wiki). 

Run `protontricks 1091500 --gui`, you may get an error saying "You are using a 64 bit WINEPREFIX", you can ignore this. Press 'ok' when this error appears.

Choose the option 'Select the default wineprefix', then 'Install a Windows DLL or component'. Tick`vcrun2019` and press ok. Go through the steps to install that runtime when the window opens. It's pretty simple (just click next really).

The options menu for protontricks will show up again, this time select 'Run winecfg'. In the window that opens, select the 'Libraries' tab. In the dropdown on 'New override for library', select `version`. Click `apply`, then `ok`.



You can now exit out of protontricks and start your game. Check to see if the log exists after you run the game. If it's there, you've successfully installed the mod. 

## Thanks to

- [@bundyo](https://github.com/bundyo)

- @okamidash (Author)


