# Build

## What's this?
Building instructions for the plugin.

### Requirements
- [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
- [xmake](https://github.com/xmake-io/xmake/releases)

1. Navigate to the repository using a command prompt.
2. cmd: `xmake -y`
3. The files should be in `build\windows\x64\release`

### Visual Studio

If you want visual studio projects execute `xmake project -k vsxmake` and you will find the sln in the newly created `vsxmake` folder.

### Install

1. Copy `build\windows\x64\release\cyber_engine_tweaks.asi` to `<cyberpunk install path>\bin\x64\plugins\cyber_engine_tweaks.asi`

#### ASI Loader

1. Download  Ultimate-ASI-Loader_x64.zip from [ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/latest)
2. Put the dinput8.dll in `<cyberpunk install path>\bin\x64\`
3. Rename `dinput8.dll` as `version.dll`, you should have `<cyberpunk install path>\bin\x64\version.dll`
4. Create a `global.ini` file in `<cyberpunk install path>\bin\x64\`
5. Paste the following lines in `global.ini`, save and you are all set!

```
[GlobalSets]
LoadPlugins=1
LoadFromScriptsOnly=1
DontLoadFromDllMain=0
FindModule=0
UseD3D8to9=0
DisableCrashDumps=0
Direct3D8DisableMaximizedWindowedModeShim=0
```

