# CET on macOS: platform survey

What the macOS game (Cyberpunk 2077 2.3.1, Steam, Apple Silicon, `CFBundleShortVersionString` 2.3.1) offers in place of the Windows pieces
CET hooks: window, input, rendering, and the engine objects CET reads. Static analysis of the game binary (`otool`, `nm`, the ObjC
metadata) and of the other macOS ports. Mac RVA = address − 0x100000000.

## Window, view and layer

- The game defines `GameWindow : NSWindow`, `GameView : NSView` (`CALayerDelegate`, `CAMetalDisplayLinkDelegate`),
  `GameWindowController`, `GameWindowDelegate`, `GameApplicationDelegate`. There is no `NSApplication` subclass.
- `-[GameWindowDelegate createMetalLayerWithWidth:height:]` (0x10768E0) builds a `CAMetalLayer`: BGRA8Unorm (0x50), opaque,
  framebuffer-only, 3 drawables. `-[GameWindowDelegate createView]` (0x1077164) creates the `GameView`, calls `setLayer:` with that layer
  (the only `setLayer:` call in the binary) and makes the view the window's content view. The view is **layer-hosting**:
  `makeBackingLayer`, `wantsUpdateLayer` and `setWantsLayer:` do not exist in the binary.
- Frames are paced by a `CAMetalDisplayLink` on that layer (`metalDisplayLink:needsUpdate:` 0x1075FFC).
- **CET's overlay** is therefore a sublayer of the game's `CAMetalLayer` (a layer-hosting view must not get subviews), drawn on CET's own
  command queue (`src/metal/MetalRenderer.mm`). Nothing in the game's renderer or in Metal is hooked or swizzled, and the game's drawables are
  never touched. The layer is hidden while the overlay is closed and no mod draws.

## Input

- The game imports `NSEvent`, `NSCursor`, `NSTrackingArea`, `CGAssociateMouseAndMouseCursorPosition`, `CGWarpMouseCursorPosition`;
  `GameController` is used for gamepads only (no `GCKeyboard` / `GCMouse`).
- `GameView` implements `keyDown:`, `keyUp:`, `flagsChanged:`, the mouse, drag and scroll methods; `GameWindow` implements none of them.
  Events reach the view through the normal `NSApplication` dispatch.
- The game installs one local event monitor itself: key-down only, it looks for Ctrl+Cmd+F (full screen) and always passes the event on.
- **CET's input** is one more `NSEvent` local monitor (`src/window/window_mac.mm`). Local monitors run before dispatch, so while the overlay
  is open the monitor returns `nil` for keyboard and mouse events and the game never sees them. Command shortcuts are never kept back.
  The events are queued and fed to ImGui and `VKBindings` on the game thread, with Windows virtual-key codes (generic `VK_SHIFT`,
  `VK_CONTROL`, `VK_MENU` for both sides, as raw input reports them; Command is `VK_LWIN` / `VK_RWIN`).
- **Cursor.** `GameView` manages the cursor itself (`HideCursor`, `ShowCursor`, `ClipCursor`, `UnLockCursor:`,
  `moveSystemCursorToCenter`, ivars `_cursorIsHidden`, `_hidenCursor`). Windows CET calls the engine's `InputSystemWin32Base::ForceCursor`
  on `CGameEngine::unkD0`; that path is not mapped on macOS (see below), so CET shows and releases the cursor through AppKit while the
  overlay is open. Whether the game re-hides or re-centres it is checked in game (NOT_YET.md).
- The automated in-game test's key injection (Codeware's `CodewareMacSelfTest.PostKey`) calls `-[GameView keyDown:]` directly and bypasses the
  monitor. CET's self-test uses `CetMacTestPostKey` (registered only under `CP2077_AUTOTEST=1`), which posts through
  `[NSApp postEvent:atStart:]`, the path of a real key.

## NightCity Console (also injected in modded sessions)

- `libcyberconsole_overlay.dylib` swizzles `-[NSApplication sendEvent:]`, `presentDrawable:` and `nextDrawable`, and toggles on backtick
  (`kVK_ANSI_Grave`) or F1. It installs no local monitor.
- CET does not swizzle anything, so the two do not chain into each other. CET's overlay key must differ from NightCity Console's:
  the preset is **F11**, a common choice on Windows. While CET's overlay is open its monitor keeps keys from `sendEvent:` too, so
  NightCity Console does not see them.

## Engine objects

- `CGameEngine` (SDK hash 2549221846, a `.data` global on Windows) has no macOS row. Codeware captures the engine in its
  `CBaseEngine::InitEngine` hook, and on macOS the game framework sits at engine **+0x338**, not the SDK's +0x308
  (`cp2077-codeware/macos/MacEngine.cpp`). CET therefore never reads `CGameEngine::Get()` on macOS: the game instance comes from Codeware's
  exported `codeware_mac_game_instance` (the route ArchiveXL already uses), in `SingletonReference`, `RTTIHelper` (every `Game.*` call that
  takes a game instance) and `RTTIExtender`.
- Function objects, stack frames, `TweakDB.unk160` and the opcode table: see the layout table in `ADDRESS_SURVEY.md`. The one difference
  CET has to handle: a scripted member function is a 0xB8-byte object on macOS (inline allocation), so `Override` swaps 0xB8 bytes for it
  (`FunctionOverride.cpp`, `GetFunctionSize`).

## Hooks shared with other macOS plugins

RED4ext for macOS accepts one hook per target. CET's hook shim (`macos/MacShims.cpp`) asks Codeware and ArchiveXL to chain first:

| CET target | Mac RVA | owner | how CET gets it |
|---|---|---|---|
| `CScript_TweakDBLoad` (TweakDB::LoadOptimized) | 0x2B75570 | ArchiveXL | `archivexl_mac_hook_chain_attach` |
| `PlayerSystem_OnPlayerSpawned` | 0x3FD4D70 | Codeware | `codeware_mac_hook_chain_attach` |

## Build facts

- The game is ad-hoc signed with `allow-jit`, `allow-unsigned-executable-memory`, `disable-library-validation`: LuaJIT's JIT works. CET's
  override thunks need no runtime code anyway (`macos/OverrideThunks.S`).
- The Oodle decompressor is linked into the game and not exported, so CET's TweakDB name table (`tweakdb.str`) and resource list stay off
  (as on a Windows install without the `tweakdb/` folder).
