# Cyber Engine Tweaks

[![Patreon](https://img.shields.io/badge/Patreon-donate-purple.svg)](https://www.patreon.com/tiltedphoques) [![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/) [![Discord](https://img.shields.io/discord/717692382849663036.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/Epkq79kd96)

* Bitcoin: bc1q0neujk5e8v8sc3934ajn8z8zav7hl6557fjj54
* Bitcoin Cash: qps5ze9p8fxmu4rszyxwy3g0ctlmrhvc3uqq0fzsnl
* Dogecoin: DMoReR33D87D6rYeUkyQb2BsEHJTqfBFva
* Ethereum: 0x7Cd23BE1C507Da85ABF0B05c7A3C03e6d3d0233B

## What's this?

**Cyber Engine Tweaks** is a framework giving modders a way to script mods using [Lua](https://www.lua.org/) with access to all the internal scripting features. It also comes with a [Dear ImGui](https://github.com/ocornut/imgui/tree/v1.82) to provide GUI for different mods you are using, along with console and TweakDB editor for more advanced usage. It also adds some patches for quality of life, all of which can be enabled/disabled through the settings menu or config files (requires game restart to apply).

Current version works with 1.61 game version.

### Current patches

| Patch      | Description     |
| :------------- | :------------------------------ |
| AMD SMT | For AMD CPUs that did not get a performance boost after CDPR's patch |
| Remove pedestrians and traffic | Removes most of the pedestrians and traffic |
| Disable Async Compute | Disables async compute, this can give a boost on older GPUs (Nvidia 10xx series for example)|
| Disable Temporal Antialiasing | Disables antialiasing, not recommended but you do what you want! |
| Skip start menu | Skips the menu asking you to press space bar to continue (Breaching...) |
| Suppress Intro Movies | Disables logos played at the beginning |
| Disable Vignette | Disables vignetting along screen borders |
| Disable Boundary Teleport | Allows players to access out-of-bounds locations |
| Disable Windows 7 VSync | Disables VSync on Windows 7 to bypass the 60 FPS limit |

### Current mod development options
| Development      | Description     |
| :------------- | :------------------------------ |
| Draw ImGui Diagnostics Window | Toggles drawing of internal ImGui diagnostics window to show what is going on behind the scenes (good for mod debugging) |
| Remove Dead Bindings | Removes bindings for mods that were not loaded |
| Enable ImGui Assertions | Enables all ImGui assertions (use this option to check mods for errors before shipping!) |
| Debug Menu | Enables the debug menus in game |
| Dump Game Options | Dumps all game options into main log file |

## Usage and configuration

[Read the wiki](https://wiki.redmodding.org/cyber-engine-tweaks/)

[Official mod examples](https://github.com/WolvenKit/cet-examples)

[Usage with Proton](https://wiki.redmodding.org/cyber-engine-tweaks/getting-started/installing/untitled)

## Contributing

If you wish to contribute to the main repo, try to follow the coding style in the code, otherwise not much to say, don't use code that is not yours unless the license is compatible with MIT.

As for the wiki, please ask on discord for write permissions.
