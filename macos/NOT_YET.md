# CET on macOS: not done yet

What the macOS build of Cyber Engine Tweaks leaves out or has not verified in game. Each item says what happens instead.

1. **Byte patches** (`src/patches/`: disable vignette, disable boundary teleport, the hidden options hook) are not built on macOS. Their
   settings have no effect. They write into game code on Windows; on macOS they would need their own verified targets.
2. **Cursor while the overlay is open.** Windows calls the engine's `InputSystemWin32Base::ForceCursor`. On macOS CET shows and releases
   the cursor through AppKit (`src/metal/MetalRenderer.mm`, `CursorFallback`). `GameView` manages its own cursor (hide, clip, centre); in a
   hand test the cursor showed while the overlay was open and CET's and a large mod's windows worked with the mouse. The
   automated test covers keys and the overlay, not the cursor.
3. **`exEntitySpawner`** (`src/reverse/RTTIExtender.cpp`) needs five `gameIGameSystem` functions and `CPhotoMode_SetRecordID`, which have no
   macOS rows yet; its calls resolve to the logged fail-safe stub and return nothing. Mods that try Codeware's `DynamicEntitySystem` first
   (Hotscenes does) are not affected. The type is registered but no system object is added to the game instance, and the two
   FunctionalTests systems (kept upstream for old mods, with a write at `WorldFunctionalTests+0x40`) are not created either: a test build
   that added them at the main menu never finished the following world load.
4. **Game log window.** Done: the five script `Log` hooks have macOS rows and install; `gamelog.log` fills in game.
5. **TweakDB names** (`tweakdb.str`) and the resource list need the Oodle decompressor, which the macOS game does not export. TweakDBIDs show
   as hashes where CET would show names (as on a Windows install without the `tweakdb/` folder).
6. **Mod Organizer 2** detection is off (there is no MO2 on macOS).
7. **SQLite extensions**: the system SQLite has no extension loading; `db:load_extension` returns an error.
8. **Text input** comes from `NSEvent.characters` (no input-method editor): plain typing works; composed input (Japanese, Chinese IME) does not.
9. **Third script stack shape.** Two game call sites run a script function through a stack with its result at +0x48 (vptr 0x6FFAEE8,
   `ADDRESS_SURVEY.md`). CET's override path treats it like the array-args stack; overrides that are reached from those two sites are
   untested.
10. **`registerHotkey` / `registerInput` kept past `init.lua`** (upstream, all platforms; found by a capture audit, 2026-10-13). The
    wrappers in `ScriptContext::ScriptContext` capture the local `registerBinding` (and through it two more locals) by reference.
    They are set to nil before the constructor returns, so normal mods are fine; a mod that keeps its own reference to them and calls
    it later would use dead locals. Not changed.
