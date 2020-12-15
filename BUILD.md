# Build

## What's this?
Building instructions for the plugin.

### Requirements
- [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
- [xmake](https://github.com/xmake-io/xmake/releases)
- [CMake](https://cmake.org/download/)

### version.dll
1. Open "..\version.sln" in Visual Studio
2. Configure it for a x64 release
3. Build (CTRL + B)

### performance_overhaul.dll
1. Navigate to the `..\cyberpunk_patch` folder in cmd
2. cmd: `xmake project -k vsxmake`

It may be necessary to repeat this command if the mhook installation fails.

3. Open `..\cyberpunk_patch\vsxmake2019\cyberpunk_patch.sln` in Visual Studio
4. Configure it for a x64 release
5. Build (CTRL + B)

It may be necessary to manually install `mhook` at this point if Visual Studio can not find it.

6. Navigate to the `..\cyberpunk_patch\vsxmake2019` folder in cmd *Optional*
7. cmd: `xmake config -P . -p windows -m release -a x64 -o "build"` *Optional*
8. Repeat Build (CTRL + B) *Optional*

### Install
The compiled .dll files can be found at:
- `..\x64\Release\version.dll`
- `..\cyberpunk_patch\build\windows\x64\release\performance_overhaul.dll`

They should be copied to the game's install folder:
- `<cyberpunk install path>\bin\x64\version.dll`
- `<cyberpunk install path>\bin\x64\Cyberpunk2077.exe.plugins\performance_overhaul.dll`
- `<cyberpunk install path>\bin\x64\performance_overhaul\config.json`
