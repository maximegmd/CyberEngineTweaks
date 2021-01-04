# Cyber Engine Tweaks

[![Patreon](https://img.shields.io/badge/Patreon-donate-purple.svg)](https://www.patreon.com/tiltedphoques) [![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/) [![Discord](https://img.shields.io/discord/717692382849663036.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/Epkq79kd96)

## What's this?

This plugin fixes some Cyberpunk 2077 issues and adds some features.

Works with 1.06.

### Current fixes

| Patches      | Description     |
| :------------- | :------------------------------ |
| AMD SMT  | Improves performance on AMD CPUs by enabling all cores. |
| Debug Menu | Enables the debug menus in game so you can ~~cheat~~, investigate...  |
| Steam Input (up to 1.04) | Enables the use of more gamepads (such as the Steam gamepad) |
| Skip start menu | Skips the menu asking you to press space bar to continue (Breaching...) |
| Remove pedestrians and traffic | Removes most of the pedestrians and traffic |
| Disable Async Compute | Disables async compute, this can give a boost on older GPUs (Nvidia 10xx series for example)|
| Disable Antialiasing TAA | Disables antialiasing, not recommended but you do what you want! |
| Disable Windows 7 VSync | Disables VSync on Windows 7 to bypass the 60 FPS limit |
| Console | Adds an overlay to draw a console so you can write any kind of script command (full Lua support) |

### Upcoming

* Memory allocation performance
* Skip conditions that never fail
* Probably some pattern conversion to SIMD

## Usage and configuration

[Read the wiki](https://wiki.cybermods.net/cyber-engine-tweaks/)

[Usage with Proton](PROTON.md)

## Contributing

If you wish to contribute to the main repo, try to follow the coding style in the code, otherwise not much to say, don't use code that is not yours unless the license is compatible with MIT.

As for the wiki, please ask on discord for write permissions.

## Credits

* [UnhingedDoork](https://www.reddit.com/r/Amd/comments/kbp0np/cyberpunk_2077_seems_to_ignore_smt_and_mostly/gfjf1vo/?utm_source=reddit&utm_medium=web2x&context=3)
* [CookiePLMonster](https://www.reddit.com/r/pcgaming/comments/kbsywg/cyberpunk_2077_used_an_intel_c_compiler_which/gfknein/?utm_source=reddit&utm_medium=web2x&context=3)
* [SirLynix](https://github.com/DigitalPulseSoftware/BurgWar) for the CI file.
* [emoose](https://github.com/yamashi/PerformanceOverhaulCyberpunk/issues/75) for pedestrian removal and start menu skip research.
* [WopsS](https://github.com/WopsS/RED4ext) for being a good friend and for doing an excellent research on scripting.
