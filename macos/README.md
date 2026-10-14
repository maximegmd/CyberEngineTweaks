# Cyber Engine Tweaks on native macOS

This folder builds Cyber Engine Tweaks 1.37.1 for the native macOS version of Cyberpunk 2077 (game 2.3.1, Apple silicon). CET runs
as a RED4ext plugin, so Lua mods (`init.lua`) run on the Mac game the same way they do on Windows.

Port by codeman101 (skyman3215). Cyber Engine Tweaks is by the Cyber Engine Tweaks team, MIT license (`../LICENSE`).

## Status (2026-10-13)

Tested on game 2.3.1 with the macOS ports of Codeware, ArchiveXL and TweakXL, by an automated in-game test (`cet_world`, five
passing runs) and one Lua mod of real size (Hotscenes, 49 files, about 650 TweakDB uses and 119 script hooks), which also passed a hand
test in game (its window and scenes, through the overlay):

- Lua VM, mods folder, `onHook`, `onTweak`, `onInit`, `onUpdate`, `onDraw`, `onOverlayOpen`, `onOverlayClose`, `onShutdown`.
- TweakDB: `GetRecord`, `GetFlat`, `SetFlat`, `CloneRecord` (and the rest of the TweakDB API on the same code).
- `Observe`, `ObserveAfter`, `Override` on script functions; the game's own calls go through them.
- `Game.*`, `GetSingleton`, `CName`, `TweakDBID`, `json`, `io` in the mod's folder, `db` (SQLite).
- The overlay: ImGui drawn on its own transparent Metal layer over the game view. F11 toggles it (set in `bindings.json`). While it is
  open, key presses, clicks and mouse movement are kept from the game; key releases always reach it.
- The game log window (script `Log` calls).

Not done yet: see `NOT_YET.md` (byte patches, `exEntitySpawner`, TweakDB names, IME text input, and smaller items).

## Requirements

- The macOS RED4ext loader (jackmaxwil's RED4ext-macos with the Frida gadget) and its hook allowlist: every target in
  `hook-offsets.txt` must be on it.
- Codeware for macOS: CET takes the game instance from it (`codeware_mac_game_instance`) and chains the player-spawned hook through it.
- ArchiveXL for macOS: CET chains the TweakDB load hook through it.

## Build

Dependency sources go in `../../vendor-mac/` (next to this checkout): `luajit2` (OpenResty, v2.1-20260914), `imgui`
(v1.91.1-docking), `sol2` (v3.5.0), `json` (nlohmann, v3.12.0), `hopscotch-map` (v2.3.1), `stb` (2c980bb), `TiltedCore` (v0.2.7).
Homebrew: `cmake`, `fmt`, `spdlog`, `freetype`, `libpng`.

```bash
cmake -S macos -B macos/build -DCMAKE_BUILD_TYPE=Release -DRED4EXT_SDK_DIR=<RED4ext.SDK-macos> -DCODEWARE_DIR=<cp2077-codeware>
cmake --build macos/build -j 10
```

The build makes LuaJIT (static, arm64, GC64) first, then `cyber_engine_tweaks.dylib`, signs it ad hoc and checks that it exports only
`Main`, `Query` and `Supports` and needs no symbol it cannot find.

## Install

```
red4ext/plugins/cyber_engine_tweaks/
  cyber_engine_tweaks.dylib
  fonts/            (from ../fonts)
  scripts/          (json, IconGlyphs; from ../scripts)
  mods/             (Lua mods)
  bindings.json     (optional; {"cet": {"overlay_key": 34339947158700032}} is F11)
```

CET writes `config.json`, `persistent.json`, `layout.ini` and its logs (`cyber_engine_tweaks.log`, `scripting.log`, `gamelog.log`,
one log per mod) in that folder.

**The F11 key on a Mac.** macOS gives F11 to Show Desktop by default, so the game never sees it. Either turn that shortcut off (System
Settings > Keyboard > Keyboard Shortcuts > Mission Control > Show Desktop) or pick another overlay key in `bindings.json` (or in CET's
Bindings tab once the overlay is open). On Apple keyboards the top row sends F11 only with fn held (fn+F11), unless "Use F1, F2, etc.
keys as standard function keys" is on.

## How it differs from Windows

| area | Windows | macOS |
|---|---|---|
| entry | `DllMain` | RED4ext `Main`/`Query`/`Supports` (`MacEntry.cpp`) |
| hooks | MinHook | the loader's hooking API through a MinHook-shaped front (`MacShims.cpp`); hooks on targets ArchiveXL or Codeware own are chained through them at the first game-state tick |
| overlay | D3D12 hook | own `CAMetalLayer` sublayer of the game view, ImGui Metal backend, hidden when nothing draws (`src/metal/`) |
| input | WndProc, raw input | `NSEvent` local monitor, macOS key codes mapped to VK codes (`src/window/window_mac.mm`) |
| cursor | `InputSystemWin32Base::ForceCursor` | the game view's own `ShowCursor`/`HideCursor` |
| `Override` | xbyak thunk; script functions allocated larger by an `AllocateFunction` hook | a fixed pool of arm64 stubs (`OverrideThunks.S`); `AllocateFunction` is inlined on macOS, so a scripted member function keeps its 0xB8-byte object and gets its own vtable copy whose handler-index getter returns the stub's index |
| script start-up | `TranslateBytecode` hook | hook on the script binder's load function, which `LoadOpcodes` is inlined into |
| `TweakDB.unk160` | set to 0 after the load | left as the game set it (it is TweakDB's generation byte on 2.3.1; zeroing it kept record handles from refreshing and the world never finished loading) |
| byte patches | vignette, boundary teleport, options | off |

The Windows build is unchanged: `WINDOWS_CHECK.md`.

## Files

- `CMakeLists.txt`, `build-luajit.sh`, `rebuild.sh`, `check-undefined.sh`: build.
- `MacEntry.cpp`, `MacShims.*`, `MacVault.cpp`, `MacPools.inc`, `TiltedMallocAllocator.cpp`, `OverrideThunks.S`, `Win32Compat.h`,
  `inc/MinHook.h`: the platform layer.
- `CetSdkRows.inc`, `MacAddresses.hpp`, `hook-offsets.txt`: macOS addresses (game 2.3.1) and the hook allowlist entries.
- `ADDRESS_SURVEY.md`, `PLATFORM_SURVEY.md`: how every address and layout was found and checked.
- `NOT_YET.md`, `WINDOWS_CHECK.md`.
