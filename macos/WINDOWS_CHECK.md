# Windows check

The macOS port must not change the Windows build. Every change to a file Windows compiles is inside `#ifdef __APPLE__` (or the
`#else` of one keeps the original text), and the Mac-only files are either under `macos/` or not picked up by `xmake.lua`.

## How it is checked

For every file modified since `v1.37.1`, the Windows view (the file with the Apple branches removed) is compared with the tag:

```bash
git show v1.37.1:<file> | diff -B - <(unifdef -U__APPLE__ <file>)
```

`-B` ignores blank lines only. Result for HEAD (`cd65b37`, 2026-10-13):

| file | lines that differ in the Windows view |
|---|---|
| `src/Image.cpp` | 0 |
| `src/Image.h` | 0 |
| `src/Options.cpp` | 0 |
| `src/Paths.cpp` | 0 |
| `src/Utils.cpp` | 0 |
| `src/Utils.h` | 0 |
| `src/VKBindings.cpp` | 0 |
| `src/VKBindings.h` | 0 |
| `src/d3d12/D3D12.h` | 0 |
| `src/dllmain.cpp` | 0 |
| `src/imgui_impl/imgui_user_config.cpp` | 0 |
| `src/imgui_impl/imgui_user_config.h` | 0 |
| `src/lsqlite3/lsqlite3.cpp` | 0 |
| `src/overlay/widgets/TweakDBEditor.cpp` | 0 |
| `src/reverse/RTTIExtender.cpp` | 0 |
| `src/reverse/RTTIHelper.cpp` | 0 |
| `src/reverse/Relocation.h` | 0 |
| `src/reverse/SingletonReference.cpp` | 0 |
| `src/reverse/TweakDB/ResourcesList.cpp` | 0 |
| `src/scripting/FunctionOverride.cpp` | 0 |
| `src/scripting/GameDump.cpp` | 0 |
| `src/scripting/GameHooks.cpp` | 0 |
| `src/scripting/LuaSandbox.cpp` | 0 |
| `src/scripting/LuaVM.h` | 0 |
| `src/scripting/LuaVM_Hooks.cpp` | 0 |
| `src/scripting/ScriptStore.cpp` | 0 |
| `src/scripting/Scripting.cpp` | 0 |
| `src/scripting/Texture.cpp` | 0 |
| `src/scripting/Texture.h` | 0 |
| `src/stdafx.h` | 0 |
| `src/window/window.h` | 0 |

## New files outside `macos/`

| file | why Windows does not build it |
|---|---|
| `src/metal/MetalRenderer.mm` | `xmake.lua` adds `src/**.cpp` only |
| `src/window/window_mac.mm` | same |
| `src/metal/MetalRenderer.h` | listed by `add_headerfiles("src/**.h")` but included only from the `__APPLE__` branch of `src/d3d12/D3D12.h` |

Everything else new is under `macos/` (CMake build, entry point, shims, address rows, thunks, docs), which `xmake.lua` does not use.
