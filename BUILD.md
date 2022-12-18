# Build

## What's this?
Building instructions for the plugin.

### Requirements
- [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
    - (You can use VS2022, just make sure you're on at least MSVC 14.34)
- [xmake](https://github.com/xmake-io/xmake/releases)
- [git](https://git-scm.com/downloads)

### Build
1. Navigate to the repository using a command prompt.
2. Check out the correct branch/tag if you're not working against master
3. Run `git submodule update --init` to pull in vendored dependencies
4. Run `xmake -y` (add `-v` for verbose output)


### Visual Studio

If you want visual studio projects execute `xmake project -k vsxmake` and you will find the sln in the newly created `vsxmake` folder.


### Additional configuration / troubleshooting

- You can specify the VS version with `xmake f --vs=2019|2022` in case it's not detected right
- Use Visual Studio Installer to install any missing components in the toolset if needed

### Install

1. Configure the install path, run: `xmake f --installpath=<cyberpunk install path>\bin\x64\plugins` (not the base dir!)
2. `xmake install` (or `xmake i` for short.)


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

