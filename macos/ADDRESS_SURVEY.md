# CET on macOS: address and layout survey

Two static surveys of the macOS game (2.3.1, Steam build 20383525, the same build as the Windows reference), done by matching
the macOS functions against the Windows binary of the same build, 2026-10-13. Part 1 maps CET's address hashes; part 2 checks the
layouts CET reads and writes directly.

## Decisions taken from it (Gate S)

| CET use | macOS | how the port uses it |
|---|---|---|
| 4 game-state OnTicks | MAPPED (vtables, slot +0x28) | rows in `CetSdkRows.inc`, hooked |
| CScript_RunPureScript | MAPPED 0x2172BF4 (already in the macOS SDK table; the loader's address DB value 0x950AB8 is a different function) | hooked |
| CBaseFunction_InternalExecute | MAPPED 0x2173120 (SDK table) | called |
| CScript_AllocateFunction | INLINED (103 sites `mov w0,#0xb8` + PoolRTTIFunction::Allocate) | not hooked; `Override` swaps 0xB8 bytes for scripted functions instead (`GetFunctionSize`) |
| CScript_TranslateBytecode (LoadOpcodes) | INLINED in the binder load 0x21E897C, `bool(binder, source, x2, x3)` | that function is hooked (prologue verified, `MacAddresses.hpp`); CET's Lua start-up runs after it |
| PlayerSystem_OnPlayerSpawned | MAPPED 0x3FD4D70 (Codeware hooks it) | chained through Codeware |
| CScript_TweakDBLoad | MAPPED 0x2B75570 (ArchiveXL hooks it) | chained through ArchiveXL; `TweakDB.unk160` confirmed at +0x160 |
| CScript_TDBIDConstructorDerive | MAPPED 0x3453B14, ABI `uint64_t(const TDBID* base, const char* name)` | macOS hook with that signature (`LuaVM_Hooks.cpp`) |
| CScript_ToStringDEBUG, the 5 Log handlers | MAPPED | rows, hooked |
| InputSystemWin32Base_ForceCursor | no twin (the cursor is ObjC state on `GameView`) | `-[GameView ShowCursor]` / `HideCursor` while the overlay is open |
| 5 gameIGameSystem functions, CPhotoMode_SetRecordID | not found | `exEntitySpawner` off (NOT_YET.md) |
| CGameEngine (SDK data global) | not mapped; framework at engine +0x338 on macOS | game instance from Codeware (`codeware_mac_game_instance`) |
| Function object sizes | CClassFunction / CClassStaticFunction 0xC0, CGlobalFunction 0xB8, scripted 0xB8 | the swap size fix above |

---

# CET address survey — Windows 2.3.1 (build 20383525) to native arm64

Static analysis only. Same game build on both sides.

- Windows RVAs: `tools/addr-match/win-addr.tsv` (image base `0x140000000`).
- Mac RVAs: file offset in `Cyberpunk2077` (image base `0x100000000`). RVA = address − `0x100000000`.
- CET hashes: `cet/CyberEngineTweaks/src/reverse/Addresses.h` (v1.37.1).
- `CBaseFunction_InternalExecute` is not in `Addresses.h`. Hash `404169501` (`0x1817231D`) is `RED4ext::Detail::AddressHashes::CBaseFunction_InternalExecute` in `cet/CyberEngineTweaks/vendor/RED4ext.SDK/include/RED4ext/Detail/AddressHashes.hpp:14`. CET calls it from `src/scripting/FunctionOverride.cpp:25-29` and `src/reverse/RTTIHelper.cpp:785`.

Status:

- **MAPPED** — the two bodies were compared and they are the same function. A matcher vote alone is not enough.
- **INLINED** — no Mac function has the Windows signature. The body lives inside the named Mac function(s).
- **UNSURE** — Mac twin not identified, or the bodies were compared and they are not the same function.

The Mac address DB (`red4ext/cyberpunk2077_addresses.json`) is wrong for two of these hashes. The macOS SDK override already has the right bodies: `RED4ext.SDK-macos/include/RED4ext/Relocation-inl.hpp:153` (`404169501 → 0x2173120`) and `:155` (`3791200470 → 0x2172BF4`).

Vtable rule used below: an Itanium virtual destructor takes two slots, so every slot after it is +8 versus Windows. That shift holds for the four game states. It does not hold for every class: the LoadOpcodes binder vcalls at `+0x130` and `+0xf8` are the same offset on both sides, while the per-element vcall moved from `+0x10` to `+0x18`.

## Table

| name | hash | Windows RVA | Mac RVA | status | entry proof | ABI notes | evidence |
|---|---|---|---|---|---|---|---|
| CRunningState_OnTick | 3592689218 | 0x9f33c4 | 0x3d8d820 | MAPPED | vtable `0x726f160` slot +0x28; `stp` prologue | `bool(this, app)`: x0, x1, w0. No sret. | GetName "Running", GetType `mov w0,#2`. See note 1. |
| CInitializationState_OnTick | 2447710505 | 0x9f3428 | 0x3d8d710 | MAPPED | vtable `0x726f118` slot +0x28; `stp` prologue | same | GetName "Initialization", GetType `mov w0,#1`. Note 2. |
| CBaseInitializationState_OnTick | 4233370276 | 0x9f336c | 0x3d8c0cc | MAPPED | vtable `0x726f0c0` slot +0x28; `stp` prologue | same | GetName "BaseInitialization", GetType `mov w0,#0`. Note 3. |
| CShutdownState_OnTick | 4069332669 | 0x42bddc | 0x3d8d94c | MAPPED | vtable `0x726f1a8` slot +0x28; `stp` prologue | same | GetName "Shutdown", GetType `mov w0,#3`. Note 4. |
| CScript_AllocateFunction | 160045886 | 0x2046e90 | none. Exemplar site `0x2201538`; alloc stub `0x4a3bc5c` | INLINED | no shared function; 103 inlined sites | Windows ignores both args and hardcodes 0xB8. 16-byte alloc result is x0=ptr, x1=size. | `mov w0,#0xb8; bl 0x4a3bc5c; bl 0x4a3de58` (_bzero). Note 5. |
| CScript_TranslateBytecode | 3442875632 | 0x5c4a78 | inlined at `0x21e8afc` inside `0x21e897c`. Inner `0x21ebbb0` | INLINED | `0x21e897c` is a real entry (`sub sp,#0x80`). The loop is not. | Container is 4 register args and returns bool. CET's `void(binder, data)` has no entry. | stride-8 loop, `cmp w0,#5`, vcall +0x18, then `bl 0x21ebbb0`. Note 6. |
| PlayerSystem_OnPlayerSpawned | 2050111212 | 0xa1452c | 0x3fd4d70 (thunk `0x3fd788c`) | MAPPED | secondary vtable slot `0x729b748`; one `b` from the thunk. Hook the body. | 4 register args x0–x3. x0 on return is leftover. | Thunk matches Windows `0xa14514`. Flag bytes +0xc/+0xd. Note 7. |
| CScript_RunPureScript | 3791200470 | 0x1bc104 | 0x2172bf4 | MAPPED | `stp` prologue, size to `0x2172d54`. SDK already lists it. | x0=func, x1=stack, x2=frame, bool in w0. Mac epilogue is `mov w0,#1; ret`. | DB `1:0x950AB8` is a different function. Note 8. |
| CScript_TweakDBLoad | 3602585178 | 0x50eb1c | 0x2b75570 | MAPPED | `stp` prologue, size to `0x2b75744`. DB value is this function. | x0=this, x1=param. Return is leftover. Field +0x160 is the byte CET clears. | `ldrb`/`strb [x20,#0x160]`; double `bl 0x2b7cc9c`. Note 9. |
| CBaseFunction_InternalExecute | 404169501 | 0x32bb28 | 0x2173120 | MAPPED | `stp` prologue, size to `0x21732c0`. SDK line 153. Called, not hooked. | 5 pointer args x0–x4, bool in w0. No sret. | Prologue saves x0..x4. DB `1:0x94FE44` starts `mov x19,x8` (sret). Note 10. |
| CScript_TDBIDConstructorDerive | 326438016 | 0x341bd8 | 0x3453b14 | MAPPED | `stp` prologue; 5598 `bl` callers. | ABI does not match CET. Mac is `(TDBID* base, const char* name)` and returns the 8-byte TDBID in x0. | strlen, `ldr x2,[x0]`, tail `b 0x34535c0`. Caller `str x0,[dest]`. Note 11. |
| CScript_ToStringDEBUG | 3515162577 | 0xe64cd0 | 0x2b79d10 | MAPPED | passed as x4 into native-member ctor `bl 0x2173860` | native handler: x0 scriptable, x1 frame, x2 result, x3 unused | frame +0x30, +0x40, +0x62; opcode table `0x908b798`; string `ToStringDEBUG`. Note 12. |
| InputSystemWin32Base_ForceCursor | 2130646213 | 0x21ad8cc | no twin. Show `0x1075238`, Hide `0x1075320`, Clip `0x1074cb0`, Update `0x1074fe4`, UnLockCursor: `0x1074bcc` | UNSURE | those five are ObjC IMPs (relative method list), not a C++ twin of `0x21ad8cc` | CET wants `void(UnkD0*, CName, bool)`. That function does not exist on Mac. | Windows edits a CName set at this+0x158. Mac ShowCursor clears ivar +0x4a and calls CGAssociate(true). Note 13. |
| CScript_Log | 3455393801 | 0xf60fe4 | 0x22111f8 | MAPPED | x3 of `bl 0x2173ad4` in the registrar; size 0xe0 | native handler, x1=frame. CET replaces the body. | one opcode, `stp xzr,xzr [frame,#0x30]`, `strb` at +0x62, table `0x908b798`. Note 14. |
| CScript_LogError | 2135235617 | 0x114a974 | 0x22112d8 | MAPPED | same ctor pattern; size 0xe0 | same as Log | registrar string `LogError`. Same frame traffic as Log. Note 14. |
| CScript_LogWarning | 3222609133 | 0x11ebd08 | 0x22113b8 | MAPPED | same; size 0xe0 | same as Log | registrar string `LogWarning`. Note 14. |
| CScript_LogChannel | 1663049434 | 0xd279d8 | 0x2211498 | MAPPED | same; size 0x118 | same. Two string opcodes (channel, then text). | two `blr`, two `strb #0x62`, two `stp #0x30`. Note 14. |
| CScript_LogChannelWarning | 2841780134 | 0x205573c | 0x22116c8 | MAPPED | same; size 0x118 | same as LogChannel | registrar string `LogChannelWarning`. Note 14. |
| gameIGameSystem_Initialize | 385618721 | 0xac7ed4 | not found | UNSURE | — | CET calls `void(TEMP_Spawner*, GameInstance*)` | Windows head loads `[rax+0x229ef0]`. The Mac `add #0x229,lsl#12` cluster at `0xcf4bcc` is a different object. Note 15. |
| gameIGameSystem_UnInitialize | 3313306514 | 0x204d490 | not found | UNSURE | — | CET calls `void(TEMP_Spawner*)` | win-addr name `spawn::Set::Deinitialize`. Not compared. Note 15. |
| gameIGameSystem_Spawn | 2509382878 | 0x661538 | not found | UNSURE | — | CET: `void(spawner, REDSmartPtr* sret, SpawnSettings&, CName)`. sret is arg2. | win-addr name `spawn::Set::SpawnObject`. Not compared. Note 15. |
| gameIGameSystem_Despawn | 3168866665 | 0x65cefc | not found | UNSURE | — | CET calls `void(TEMP_Spawner*, IScriptable*)` | win-addr name `spawn::Set::DespawnObject`. Not compared. Note 15. |
| gameIGameSystem_SpawnCallback | 2840271332 | 0x8b57f4 | not found | UNSURE | — | CET calls `void(IScriptable*, ent::Entity*)` | win-addr name `world::RuntimeEntityRegistry::RegisterEntity`. Not compared. Note 15. |
| CPhotoMode_SetRecordID | 4052428712 | 0x661900 | not found | UNSURE | — | CET calls `void(TweakDBID by value, Handle<IScriptable>&)` | Windows `0x661900`–`0x661963` saves rcx and rdx, calls `0x660258`, `0x660168`, `0x100c10`. Mac twin not identified. Note 15. |

## Notes

### 1. CRunningState_OnTick — hook, signature matches

CET installs one hook on all four: `GameHooks.cpp:33` (`MH_CreateHook` of `HookStateTick`). Signature is `bool(IGameState*, CGameApplication*)` (`GameHooks.h:16`, body `GameHooks.cpp:93`). `HookStateTick` indexes `m_stateTickOverrides` by `apThisState->GetType()` (`GameHooks.cpp:101`). The four overrides are registered in enum order in `GameHooks.h:44-47`. `EGameStateType` is BaseInitialization=0, Initialization=1, Running=2, Shutdown=3 (`vendor/RED4ext.SDK/include/RED4ext/GameStates.hpp:11-16`).

Windows `IGameState` slots: +0x00 dtor, +0x08 GetName, +0x10 GetType, +0x18 OnEnter, +0x20 OnTick, +0x28 OnExit. Mac slots after the two-slot destructor: GetName +0x10, GetType +0x18, OnEnter +0x20, OnTick +0x28, OnExit +0x30.

Running vtable:

- Windows `0x2ea5b10`: GetName `0x18ec570`, GetType `0x18e4120`, OnEnter `0x9f3844`, OnTick `0x9f33c4`, OnExit `0x496010`.
- Mac `0x726f160`: GetName `0x3d8d7e4` (adrp `0x6d31000` + `0x6f6` = cstring `0x6d316f6` "Running"), GetType `0x3d8d7f0` (`mov w0,#2`), OnEnter `0x3d8d7f8`, OnTick `0x3d8d820`, OnExit `0x3d8da30`.

Mac OnTick `0x3d8d820` keeps the app in x19 (x1), loads the engine pointer at global `0x90dc3a0`, calls `0x3d99928`, tests byte `[obj+0x48]`, and on the null path does a vcall at +0x70 with w1=`-0x2a`. Otherwise it calls helper `0x3d8af90(app, 3)` and returns `eor w0,w20,#1`. That is the same shape as Windows: if the process call returns true, OnTick returns false; otherwise it runs the phase helper and returns true. The Windows side asserts when `[global+0x48]==0`; Mac still runs the helper and returns true. Helper `0x3d8af90` matches Windows `0x9f385c` on fields +0x268, +0x278, +0x274, +0x288. `0x3d99928` was not body-compared to Windows `0x29a208`.

Hook the Mac OnTick entry. x0/x1/w0 match `HookStateTick`. The forwarded bool is real.

### 2. CInitializationState_OnTick — hook, signature matches

- Windows vtable `0x2ea5b40`: GetName `0x18ec560`, GetType `0x18e3fb0`, OnEnter `0x9f375c`, OnTick `0x9f3428`, OnExit `0x496010`.
- Mac vtable `0x726f118`: GetName `0x3d8d5c4` (page `0x6d79000` + `0x8f7` = `0x6d798f7` "Initialization"), GetType `0x3d8d5d0` (`mov w0,#1`), OnEnter `0x3d8d5d8`, OnTick `0x3d8d710`, OnExit `0x3d8da30`.

Same hook and ABI as note 1. GetType immediate 1 is `EGameStateType::Initialization`, which is the index CET uses.

### 3. CBaseInitializationState_OnTick — hook, signature matches

- Windows vtable `0x2ea2570`: GetName `0x18ec550`, GetType `0x18d9c80`, OnEnter `0x9f349c`, OnTick `0x9f336c`, OnExit `0x496010`.
- Mac vtable `0x726f0c0`: GetName `0x3d8b74c` (page `0x6d79000` + `0x695` = `0x6d79695` "BaseInitialization"), GetType `0x3d8b758` (`mov w0,#0`), OnEnter `0x3d8b760`, OnTick `0x3d8c0cc`, OnExit `0x3d8da30`.

Same hook and ABI. Two control-flow differences, identity still holds: on the phase-3 path and on a null engine global, Mac returns 0 where Windows asserts. A second Mac BaseInitialization table was seen earlier at `0x6e4b258` with the same OnTick slot; that table was not re-read in the last pass, so treat `0x726f0c0` as the confirmed one.

### 4. CShutdownState_OnTick — hook, signature matches

- Windows vtable `0x2ea5ae0`: GetName `0x18ec580`, GetType `0x18e46c0`, OnEnter `0x496010`, OnTick `0x42bddc`, OnExit `0x496010`.
- Mac vtable `0x726f1a8`: GetName `0x3d8d908` (page `0x6d79000` + `0x95c` = `0x6d7995c` "Shutdown"), GetType `0x3d8d914` (`mov w0,#3`), OnEnter `0x3d8d91c`, OnTick `0x3d8d94c`, OnExit `0x3d8da30`.

OnExit is shared. Mac `0x3d8da30` and Windows `0x496010` both return true, and Shutdown's OnEnter is that same trivial function on Windows (`0x496010`). Mac Shutdown OnTick also releases the engine singleton on a path Windows does not. Hook `0x3d8d94c`. Signature matches.

These four were taken from the vtables. `CGameApplication::AddState` was not used; that RED4ext hook fails on macOS at runtime.

### 5. CScript_AllocateFunction — INLINED, hook signature does not match the body

CET hooks it (`FunctionOverride.cpp:591`) as `void*(void* pool, size_t)` (`FunctionOverride.h:40`). `HookCreateFunction` ignores the size and calls the trampoline with `kOverrideAllocationSize` (`FunctionOverride.cpp:153-160`).

Windows `0x2046e90` (size `0x3e`) ignores rcx and rdx. It does `mov edx,#0xb8`, `lea rcx,[rsp+0x20]`, `call 0xb550ec` (wrapper around `DefaultAllocator::Allocate` `0x1457c8`, 16-byte `{ptr,size}` via a hidden out), then `memset` (`0x1908820`) of that pointer for the second qword, and returns the pointer. 48 `call` sites. A sample registrar at `0x18e08a0` (string `Cast;Int16;Float`) issues a bare `call 0x2046e90` with no args, then the native-global ctor `0xc896e4`.

Mac has no function with that body. 103 sites do `mov w0,#0xb8; bl 0x4a3bc5c; bl 0x4a3de58`. Stub `0x4a3bc5c` is `red::memory::PoolStorageProxy<red::PoolRTTIFunction>::Allocate(unsigned long long)`. Stub `0x4a3de58` is `_bzero`. The allocator returns the 16-byte result in x0 (pointer) and x1 (size), so `bzero(x0, x1)` zeros the object. 101 of those sites sit in registrars clustered at `0x2201538`..`0x220c1d8`. Exemplar `0x2201538` (string `Cast;Uint8;Int8`): `mov w0,#0xb8; bl` alloc; `mov x19,x0; bl` bzero; `bl 0x2173ad4` (native ctor, twin of Windows `0xc896e4`); `b 0x21884a4`. Two further sites use `mov w0,#0xc0` instead of `0xb8`.

`HookCreateFunction`'s `(pool, size)` signature matches neither body. The Windows function never reads the pool or the size. Hooking one registrar does not enlarge the others. To enlarge every script-function object, patch each `mov w0,#0xb8` that feeds stub `0x4a3bc5c`, or hook `PoolRTTIFunction::Allocate` itself (that changes every allocation from that pool, not only script functions). Sites such as `0x39af0a4` that also move `0xb8` are stride loops, not this allocator.

### 6. CScript_TranslateBytecode — INLINED inside `0x21e897c`

CET hooks `CScriptDataBinder::LoadOpcodes` as `void(uintptr_t binder, uintptr_t data)` (`LuaVM.h:75`, `LuaVM_Hooks.cpp:325-331`, install at `LuaVM_Hooks.cpp:461`). The hook calls `Initialize()`, the original, then `PostInitializeScripting()`. It needs one call per load, not one call per element.

Windows outer `0x5c4a78` (through `0x5c4ad6`): `rbx=[rdx]`, `eax=[rdx+0xc]`, end = begin + count*8. Loop: element = `[rbx]`, vcall `[element]+0x10`, if `al==5` call inner `0x5c4c14(binder, element)`, `rbx+=8`. The only caller of the outer is `0x7b8a55` inside `0x7b891c` (through `0x7b8a9c`). That caller runs five bool passes, vcall `[binder]+0x130` with rdx = param+0xf0, calls LoadOpcodes(binder, &array), vcall `[binder]+0xf8`, and returns bool. Inner `0x5c4c14` has that one caller. Gather helper `0x7b8aa8` (also called from `0x3ccbe2`) adds counts at this+0xac/+0xbc/+0xcc/+0xdc/+0xec plus `[array+0xc]`, compares to `[array+8]`, and resizes with element size 8. DynArray layout is pointer +0, capacity +8, size +0xc on both sides.

Mac twin of the caller is `0x21e897c` (through `0x21e8bec`), found because gather helper `0x7b8aa8` = Mac `0x21eef74` (`bl 0x286e8` with w2=w3=8, then walk `[this+0xe0]`). Callers of `0x21eef74`: `0x21e8a78` (inside `0x21e897c`), `0x21e96e4`, `0x21fbff4`. Only `0x21e897c` then does the five test-w0 calls (`0x21e8c88`, `0x21e9040`, `0x21e9234` with no test, `0x21e93a8`, `0x21e94dc`) and:

- `ldr x22,[x20]; bl 0x21f3458`; vcall `[x22]+0x130` (same offset as Windows).
- Inlined loop `0x21e8afc`–`0x21e8b44`: count at `[sp+0x1c]`, begin at `[sp+0x10]`, stride 8, vcall `[element]+0x18` (Windows was +0x10), `cmp w0,#5`, `b.ne` next, `bl 0x21ebbb0(x0=binder, x1=element)`.
- vcall `[obj]+0xf8` only if w19!=0; `mov w23,#1`; return that bool in x0.

Inner `0x21ebbb0` (size `0x17c`) matches Windows `0x5c4c14`: `ldr w [x1,#0xbc]`, `ldr x [x1,#0x18]`, `add x0, that, #0x78`, `ldr [x1,#0xc0]`, `ldr w [that,#0x18]` (Windows was +0x1c), `ldrh [x1,#0xc8]`, `ldr [x1,#0xb0]`.

`0x21e897c` prologue: `sub sp,#0x80; mov x19,x3; mov x22,x2; mov x21,x1; mov x20,x0`. So x0=binder, x1=source, x2 and x3 are extra (w19 gates the final vcall; the Windows caller had 3 args). There is no `void(binder, data)` entry. Hook `0x21e897c` once per load, forward x0–x3, and forward the bool. Do not hook `0x21ebbb0`; it runs once per type-5 element.

### 7. PlayerSystem_OnPlayerSpawned — hook, 4 args match, return is leftover

CET hooks `uint64_t(a1, a2, a3, a4)` (`LuaVM_Hooks.cpp:334-341`, install at `:492`). It forwards the original's return, then calls `PostInitializeMods` once.

The hash is the body, not the thunk, on both sides.

- Windows body `0xa1452c`–`0xa146c7`. Thunk `0xa14514`. `.rdata` slot `0x312a748`.
- Mac body `0x3fd4d70` (function through `0x3fd5250`). Thunk `0x3fd788c` (through `0x3fd78a0`): `ldp x8,x1,[x0,#0x10]; add x2,x0,#0x20; add x3,x0,#0xb0; mov x0,x8; b 0x3fd4d70`. That is the same unpack as Windows `0xa14514`: a1=`[ps+0x10]`, a2=`[ps+0x18]`, a3=`ps+0x20`, a4=`ps+0xb0`.
- The thunk pointer is stored at `__DATA_CONST` `0x729b748` (raw `0x30000003fd788c`, chained low 36 bits). PlayerSystem vptr is `0x729b260`; the secondary vptr at +0x320 is `0x729b580`. `0x729b748` is slot +0x1c8 of the secondary. No direct `bl` to the body. Hook `0x3fd4d70`.

Body match against Windows `0xa1452c`:

- Helper `0x3fd88a4` is `mov w8,#1; strb w8,[x0,#0xc]; strb w1,[x0,#0xd]; ret`. Windows writes `mov byte [r9+0xc],1; mov byte [r9+0xd],al` with r9=`[a2+8]`. Mac calls it as `bl 0x3fd88a4(x0=[x20+8], x1=vcall result)`.
- Vcall is `[[a2+0x20]]+0x20`. Windows is +0x18 (+8).
- Then an sret `bl 0x1fd1408`, a handle swap into `[a2+0x10]`, an sret `bl 0x9aa9a4` on `[a3+0x20]`, a deref of a4, and if both are live an add of +0xcc (Windows +0xd0) and a vcall at +0x1d8 (Windows +0x1d0) with w2=1.
- x24 = a1+`0x604c0` (`add x8,x19,#0x60,lsl #12; add x24,x8,#0x4c0`). Compare `[a1+0x61]` with `[a3+0x52]`, lock/swap at +`0x604d0`/+`0x604c0`, load `[x24+0x38]` which is `[a1+0x604f8]`, then a direct `bl 0x39a6f60(a1, [a3+0x52], [a2+0x10])`. Windows does that as vcall `[a1]+0x1f8` (devirtualized; same args). Then test `[a3+0x50]` and `[a3+0x80]`.
- Failure path uses the string `player` at `0x6c90269` via `bl 0x140c6fc`.
- The early Mac lock on +0x244/+0x240/+0x246 through globals `0x6e43b78`/`0x6e43b70` is the inlined form of Windows `call 0x20464e4(a1+0x240)` plus the tail `jmp 0x377b1c`.

PlayerSystem ctor Mac `0x3fcdb84` matches Windows `0xade154`: vptr `0x729b260`, secondary +0x320 stored at `[this+0x70]`, zeros +0x238 and +0x240, string `PlayerSystem - Stubs` at `0x6d84519`.

`0x3fd68a8` and `0x3fd6608` are siblings that touch +`0x604f8`. They are not the hook target.

Control flow that differs, identity unchanged:

- Mac skips the call at `[a1+0x604f8]` only when the value is neither 0 nor 3. Windows asserts if it is nonzero.
- If `[a3+0x80]` is nonzero, Mac calls the function pointer. Windows asserts.
- Mac epilogue is `ret`. Windows tail-jumps `0x377b1c`.

The four register args match the hook. The value CET forwards in x0 is whatever the last refcount call left there. Windows forwards the tail-call's return the same way. The hook can keep the 4-arg signature.

### 8. CScript_RunPureScript — MAPPED, DB wrong, bool is always 1

CET hooks `bool(CClassFunction*, CScriptStack*, CStackFrame*)` (`FunctionOverride.h:39`, body `FunctionOverride.cpp:163`, install at `:574`). Both return paths forward the original's bool (`FunctionOverride.cpp:246` and `:249`).

Windows `0x1bc104` returns the bool from helper `0x145d78`. Mac `0x2172bf4` (through `0x2172d54`) takes the same three pointers and ends with `mov w0,#1; ret`. The SDK override already points this hash at `0x2172BF4` (`Relocation-inl.hpp:155`, comment `CBaseFunction_ExecuteScripted`). The JSON DB value `1:0x950AB8` is a different function: it uses x0–x5 and compares the bytes `0x4e,0x6f` (`"No"`). Do not use it.

The hook ABI matches. Because the Mac original always returns true, `return RealRunPureScriptFunction(...)` cannot report a script failure on Mac. Callers that branch on the bool will always see success unless the hook returns false itself (the "hooked but missing" path at `FunctionOverride.cpp:173` still returns false).

### 9. CScript_TweakDBLoad — MAPPED, DB value is correct

CET hooks `uint64_t(this, param)` (`LuaVM_Hooks.cpp:344-353`, install at `:477`). After the call it sets `TweakDB::Get()->unk160 = 0`. The SDK asserts that field at offset `0x160` (`vendor/RED4ext.SDK/include/RED4ext/TweakDB.hpp:165` and `:242`).

Mac `0x2b75570` (through `0x2b75744`): x20=x0 (this), x21=x1 (param). `bl 0x2b7cc9c` at `0x2b7561c` and again at `0x2b75634` (twin of Windows `0x50dfcc`). `bl 0x2b78c50` at `0x2b75648`. `ldrb w8,[x20,#0x160]` / `strb w8,[x20,#0x160]` at `0x2b75650` / `0x2b75658`. Failure path `bl 0x2b77730` at `0x2b7573c`. The DB entry `1:0x2B75570` is this function.

Earlier body compare, not re-printed in the last pass: a `cache` string, a vcall at +0xc0 where Windows uses +0xb8, the byte at +0x160 incremented on the failure path, and an assert line `0x1af` against Windows `0x1ad` (`tweakDB.cpp`, "Failed to load optimized TweakDB file!"). The +0x160 pair above was reconfirmed and is the field CET writes.

x0/x1 match the hook. Neither side produces a meaningful return; CET still forwards it. The `unk160 = 0` write is the right field on Mac.

### 10. CBaseFunction_InternalExecute — MAPPED, called, DB wrong

Not a CET `Addresses.h` hash. Called, not hooked:

```c
using TCallScriptFunction = bool (*)(IFunction*, IScriptable*, CStackFrame*, void* out, void* resultType);
```

`FunctionOverride.cpp:25` and `:29`, also `RTTIHelper.cpp:25` and `:785`. Five pointer arguments, bool result.

Mac `0x2173120` (through `0x21732c0`) prologue saves x22=x0, x25=x1, x20=x2, x21=x3, x24=x4. Bool comes back in w0. That is the CET order. The SDK override already has `{404169501, 0x2173120}` at `Relocation-inl.hpp:153`.

Reject DB / NCC `1:0x94FE44`. That function starts `mov x19,x8` (sret in x8) and is not this body. `sdk-hashes-ncc-check.tsv` lists the same wrong NCC for hash `404169501`.

### 11. CScript_TDBIDConstructorDerive — MAPPED, ABI does not match the hook

CET hooks `TDBID*(TDBID* base, TDBID* this, const char* name)` (`LuaVM.h:72`, `LuaVM_Hooks.cpp:300-304`, install at `:431`):

```c
const auto result = s_vm->m_realTDBIDCtorDerive(apBase, apThis, acpName);
s_vm->RegisterTDBIDString(apThis->value, apBase->value, std::string(acpName));
return result;
```

Windows `0x341bd8`–`0x341c4a` has 4658 callers. A sample site `0x340653` is `lea r8, name; lea rdx, out; lea rcx, base` — three arguments, out pointer in rdx. The CRC itself is helper `0x1cc3d50` (not a pdata start; `not eax` at the top; returns only the 32-bit hash). The Windows wrapper does the length-byte packing. CRC table `0x314b800` starts `00 00 00 00 96 30 07 77 2c 61 0e ee`.

Mac `0x3453b14` (through `0x3453b58`) has 5598 `bl` callers. `vote_string_arg(0x341bd8, reg 8)` anchored 2834 strings (`.cpoCharacterBuild`, `.vendorID`, and the rest) and every one of them voted for `0x3453b14`. The body is `mov x8,x1` (the name), an inlined strlen, `ldr x2,[x0]` (seed = `*base`), `mov x0,x8`, `b 0x34535c0`. Call site `0x26d3fdc`: `add x1, string; add x0, sp,#8` (the base); `bl 0x3453b14`; `str x0,[x20,#0x54]`.

`0x34535c0` is the byte-CRC over table `0x6c0bc80` (same first bytes as Windows `0x314b800`; other copies of the table sit at `0x6b38eb8`, `0x6b44ea8`, `0x6c0cf28`). It returns the packed TDBID in x0:

```
mvn w8, w10
add x9, x2, x1, lsl #32
and x9, x9, #0xff00000000
orr x0, x8, x9
```

Arguments of the CRC are `(name, len, seed_tdbid)`. The length byte is bits 32–39 of `seed + (len<<32)`, which is the old length byte plus `strlen`. That is the same packing Windows does in the wrapper.

Nearby functions that are not this hash: `0x3453484` (1805 callers) is the absolute constructor with seed x2=0. `0x3453bc0` is an in-out form `(TDBID*, name, len)` with 0 callers.

What a Mac port has to change. Callers pass two args and `str` the returned x0. The real function ignores a third register. If the Windows hook is installed as-is:

- x0 = base, so `apBase` is right.
- x1 = the name pointer, so `apThis` is a `char*`, and `apThis->value` reads the first 8 bytes of the string.
- x2 is whatever the caller left there, so `acpName` is not the name. `std::string(acpName)` reads the wrong pointer.
- Calling the trampoline as `(apBase, apThis, acpName)` happens to still compute the right TDBID, because the Mac body uses x0 as the base and x1 as the name.
- Returning that x0 is what Mac callers store. Returning a `TDBID*` the Windows way would store a pointer into the TDBID slot.

Rewrite the hook as `uint64_t(TDBID* base, const char* name)`, register `(result, base->value, name)`, and return `result` in x0. Do not read an out-pointer.

### 12. CScript_ToStringDEBUG — hook replacement, signature matches

CET replaces the native handler (`LuaVM_Hooks.cpp:307-322`, install at `:445`). It does not call the original. The handler must be the 4-arg native function, and the `CStackFrame` offsets CET uses must be the same: `data` +0x30, `dataType` +0x38, `context` +0x40, `paramFlags` word +0x60, `currentParam` byte +0x62, code pointer at +0.

Windows `0xe64cd0`–`0xe64d4d`. Opcode table `0x33a6e50`.

Mac `0x2b79d10` (through `0x2b79d9c`). The registrar is inside `0x2b79424`. The string site `0x2b79598` is `ToStringDEBUG` at `0x6d2befb`. The once-init at `0x2b798a8` passes x4=`0x2b79d10` into `bl 0x2173860` (native member ctor). The body does `stp xzr,xzr [frame,#0x30]`, increments `[frame,#0x62]`, reads context `[frame,#0x40]`, loads the opcode from `[frame]` and increments, dispatches through table `0x908b798`, increments the code pointer again, `bl 0x3453c08`, and if x2 (the result) is set, `bl 0x2aacc` to copy the string. x0 (scriptable) is unused, x3 is unused.

`0x908b798` is the opcode-handler table the SDK override already names (`Relocation-inl.hpp:157`, hash `961751128`). `0x2aacc` is `CString_copy` in that same table. The offsets CET's hook uses are the Mac offsets. Replacing this entry is the right hook.

### 13. InputSystemWin32Base_ForceCursor — no twin

CET calls it, it does not hook it (`src/d3d12/D3D12.cpp:10-16`):

```c
void (*)(CBaseEngine::UnkD0* this, CName reason, bool show)
forceCursor(CGameEngine::Get()->unkD0, CName "ImGui", acEnabled);
```

Windows `0x21ad8cc`–`0x21ad915` does not call `ShowCursor` or `ClipCursor`. It takes the set at `this+0x158` (`lea r9,[rcx+0x158]`). If r8b (show) it calls `0x15213c(set, &reason)` and, if the name is absent, `0x109a44` to insert. Otherwise `0x6c4244` erases. Returns void. Three `.rdata` slots were recorded earlier (`0x2c2eac8`, `0x2c2eb90`, `0x3175bb0`).

Mac has no function with that signature and no CName reason set on the input system. Cursor policy is ObjC on the game view. Method lists are relative (`__TEXT,__objc_methlist` at `0x4a47d70`), so scanning `__DATA` for absolute IMP pointers finds nothing. The view list at `0x4a47fd8` (72 entries) holds the methods below. They have zero C++ vtable slots and zero `bl` callers. Entry proof is the relative IMP field plus an `stp` prologue.

Resolved methods:

| method | RVA | size | what it does |
|---|---|---|---|
| initialize | 0x107416c | 0x2b0 | calls Hide. Ivar offsets from `0x73976fc`. Init stub `0x4a3ece0`, selector cstring `0x6dbbfae`. |
| mouseDown: | 0x1074620 | 0x11c | calls Hide. |
| UnLockCursor: | 0x1074bcc | 0x64 | the only caller of `_CGAssociateMouseAndMouseCursorPosition`. |
| ClipCursor | 0x1074cb0 | 0x230 | lock, warp, menu-state special case. |
| UpdateCursor | 0x1074fe4 | 0x124 | per-move decider. Returns early (leaves the cursor alone) when a policy byte or dword is set. |
| mouseMoved: | 0x1075108 | 0x130 | calls ClipCursor and UpdateCursor. |
| ShowCursor | 0x1075238 | 0x84 | force visible. |
| HideCursor | 0x1075320 | 0xd4 | sets the hidden flag and locks. |
| flagsChanged: | 0x1075bd4 | 0x1f4 | calls Show. |

`UnLockCursor:` (`0x1074bcc`): the ivar offset slot `0x7397728` is the signed dword 72 (`0x48`). If byte `[this+0x48]` already equals w2, it returns. Otherwise `mov x0,x2; bl 0x4a399a0; strb w20,[x19,x21]`, then a window getter `bl 0x4a3fe60` and a tail `b 0x4a40c20` with w2=1. Stub `0x4a399a0` is `_CGAssociateMouseAndMouseCursorPosition` (symbol index 67994, indirect slot 64, exactly one caller, and that caller is inside this method). w2=1 associates the mouse with the cursor (unlocked, cursor tracks the mouse). w2=0 disassociates (captured). The flag is stored at ivar +0x48.

`_CGWarpMouseCursorPosition` is stub `0x4a39ae4` (symbol 68021, 3 callers). `_CGDisplayPixelsHigh` is stub `0x4a39a84`. `_CGMainDisplayID` is the `bl` immediately before the warp. `_NSPointInRect` is stub `0x4a39c10`. An earlier guess that stub `0x4a39950` was CGAssociate was wrong; that stub is `_CFStringCreateWithBytes` and has 0 callers. `OBJC_CLASS_$_NSCursor` is indirect slot 2037 (GOT, not a stub).

`ShowCursor` (`0x1075238`, through `0x10752bc`) is the function that forces the cursor visible. If `cursorIsHidden` is set, it clears it and calls `UnLockCursor:` with w2=1, which is `CGAssociate(true)`. It then builds a block (function `0x10752bc`, descriptor `0x6efed50`) and calls stub `0x4a3dfb4` on the global at `0x6e49e90`. The `cursorIsHidden` ivar offset slot is `0x7397720`, value `0x4a`. Getter `0x107632c` is `ldrb w8,[x0,x8]; and w0,w8,#1`. `IsGameInOptionMode` `0x10752e4` reads that same byte.

`HideCursor` (`0x1075320`) calls ClipCursor first, loads a cursor via GOT `0x6de06b8` / stub `0x4a40280` and ivar +0x50 (slot `0x7397734`), and calls the policy accessors. If the policy byte is clear it sets `cursorIsHidden=1` and locks (stubs `0x4a3fe60`, `0x4a3fbe0`).

`ClipCursor` (`0x1074cb0`) reads `cursorIsHidden`. If set, it calls `0x1073358` (a bool, xor'd with 1 into w20) and a window-key check (`bl 0x4a3fee0`). It compares a CName against the literal built at `0x1074d68` (`x21 = 0xb40d5ba406b018a2`) with the string `inkInGameMenuState` at `0x6c657c1`, resolved by `bl 0x3452da0` (`CNamePool_AddPair`, the SDK RVA). If the current UI state equals that CName, it calls stub `0x4a3ec40` with d4=5 (the first objc stub; the selector name was not resolved; it is used again with d4=30 and behaves like a point-in-rect clamp). Otherwise it can call `UnLockCursor:` with w2=0 at `0x1074e38` (CGAssociate false), then `_CGMainDisplayID`, `_CGDisplayPixelsHigh`, and `_CGWarpMouseCursorPosition`. A global byte at `0x79010c0` gates a second `UnLockCursor:(w2=1)` at `0x1074e90` and `0x1074eac`. It also does `bl 0x4a40c20` with w2=0 at `0x1074ebc`.

`UpdateCursor` (`0x1074fe4`) calls accessor `0x1cd68ac`, then byte getter `0x1cd73b4`. If bit 0 is set it returns and leaves the cursor alone. Then dword getter `0x1cd73ec`; if nonzero it returns. Otherwise it checks the window and the `hidenCursor` ivar and, on a hit of an inset rect, tails to a hide selector. Accessor `0x1cd68ac` returns the singleton at `0x7c63220` (`adrp 0x7c63000; add #0x220; ret`). Getter `0x1cd73b4` is `ldrb w0,[x0,#0x39]`. Setter `0x1cd73bc` is `strb w1,[x0,#0x39]`. Getter `0x1cd73ec` is `ldr w0,[x0,#0x44]`. The names of fields +0x39 and +0x44 were not identified.

Ivar offset slots (signed dwords in `__DATA,__objc_ivar`):

| slot | field | offset |
|---|---|---|
| 0x739771c | cursorInView | (used by the view; not re-printed) |
| 0x7397720 | cursorIsHidden | 0x4a |
| 0x7397724 | hidenCursor | 0x80 (getter `0x10763c8`, objc atomic) |
| 0x7397728 | UnLockCursor flag | 0x48 |
| 0x739772c | pointer ClipCursor loads at `0x1074e54` | 0x40 |
| 0x7397734 | cursor object | 0x50 |

Window controller (`0x4a48538`, 13 methods), not the decision itself:

- `windowDidBecomeKey:` `0x1073888`–`0x1073900`: `setM_windowFocus:(w2=1)` stub `0x4a40da0` (sel `0x6dbe739`); `m_view` stub `0x4a3ffe0` (sel `0x6dbdc12`); `handleWindowDidBecameKey` stub `0x4a3f960` (sel `0x6dbd56c`); `HideCursor` stub `0x4a3ecc0` (sel `0x6dbbf9b`); then engine global `0x90dc3a0`, `ldr x0,[engine,#0xd0]` (this is `unkD0`), vcall `[vtable+0x40]`, then `mov w1,#1; b 0x3d9ae5c(engine, 1)`.
- `windowDidResignKey:` `0x1073900`–`0x1073964`: focus w2=0, `handleWindowResignKey` stub `0x4a3f980` (sel `0x6dbd585`), `ShowCursor` stub `0x4a3eda0` (sel `0x6dbbbef`), `b 0x3d9ae5c(engine, w1=0)`.
- `0x3d9ae5c`: if the global is null or byte `[global+0x48]` is nonzero, return. Otherwise `ldr x0,[x0,#0xa8]; b 0x3eb63cc` with w1 preserved. `0x3eb63cc` was not disassembled. The hidden flag reaches it only when engine byte +0x48 is clear. The same global `0x90dc3a0` and byte +0x48 are what Running OnTick tests.

Other selectors: ClipCursor cstring `0x6dbbf58`, UnLockCursor: `0x6dbc3b6`, Init `0x6dbbfae`. `0x1078148` (size `0x58`) also calls Show; it belongs to the other window class (method list `0x4a485e0`, 56 entries) and is not the decision.

How CET can force the cursor visible. There is nothing to pass `CName "ImGui"` to. Call the view's `ShowCursor` at `0x1075238`. That clears `cursorIsHidden` at +0x4a and calls `CGAssociateMouseAndMouseCursorPosition(true)`. `HideCursor` `0x1075320` and `ClipCursor` `0x1074cb0` will set the flag again and call `CGAssociate(false)` plus `CGWarpMouseCursorPosition`, so a one-shot Show loses to the next mouse-move unless the overlay also holds off those two. `UpdateCursor` `0x1074fe4` is the per-move decider and returns without hiding when singleton `0x7c63220` byte +0x39 is set or dword +0x44 is nonzero. The become-key path hides, the resign-key path shows, and both also call `0x3d9ae5c`.

### 14. Log, LogError, LogWarning, LogChannel, LogChannelWarning — hook replacements, signatures match

CET replaces the handlers. It does not call the original. `HookLog` (`LuaVM_Hooks.cpp:201`) consumes one string opcode. `HookLogChannel` (`:224`) consumes a channel, then a string. Installs: Log `:366`, LogError `:379` (also `HookLog`), LogWarning `:392` (also `HookLog`), LogChannel `:405`, LogChannelWarning `:418`. The address has to be the 4-arg native handler. CET's frame offsets are the ones in note 12.

Registrars call native ctor `bl 0x2173ad4` with the handler in x3 (`adrp 0x2211000` + add). The registrar string sits immediately before the once-flag. `rx.cstr` drops strings shorter than 4, so `"Log"` itself prints empty; the page math is `0x6cc5000+0xa28 = 0x6cc5a28`, and the next strings (`LogError` at +0xa2c, and the rest) print.

| hash | Windows | Mac handler | registrar string site | size | opcodes |
|---|---|---|---|---|---|
| Log 3455393801 | 0xf60fe4–0xf61083 | 0x22111f8 (add at `0x2214ac0`, #0x1f8) | `0x2214a60` | 0xe0 | 1 |
| LogError 2135235617 | 0x114a974 | 0x22112d8 | `0x2214b14` "LogError" | 0xe0 | 1 |
| LogWarning 3222609133 | 0x11ebd08 | 0x22113b8 | `0x2214bc8` "LogWarning" | 0xe0 | 1 |
| LogChannel 1663049434 | 0xd279d8 | 0x2211498 | `0x2214c7c` "LogChannel" | 0x118 | 2 |
| LogChannelWarning 2841780134 | 0x205573c | 0x22116c8 | `0x2214de4` "LogChannelWarning" | 0x118 | 2 |

`LogChannelError` is the extra registrar at `0x2214d30` → handler `0x22115b0`. CET has no hash for it.

Log body `0x22111f8` against Windows `0xf60fe4`: frame in x1; `bl 0x29218`; `bl 0x21af014(sp, 0)` ref init; `stp xzr,xzr [x19,#0x30]`; `ldrh [frame,#0x60]` shifted by `ldrb [frame,#0x62]`, `tbz` the param flag; optional `bl 0x21af338`; inc `[frame,#0x62]`; context `[frame,#0x40]`; opcode; table `0x908b798`; `blr` with x2=x3=sp (both point at the ref, same as Windows r8=r9); inc code; `bl 0x21afcbc` sink; `bl 0x29228`. Log, LogError, and LogWarning each have 1 `blr`, 1 `strb #0x62`, 1 `stp #0x30`, and the same callees. The byte differences between them are relocated `bl`/`adrp` offsets. LogChannel and LogChannelWarning have 2 `blr`, 2 `strb #0x62`, 2 `stp #0x30` (channel, then text) and the same sink. That split is exactly `HookLog` versus `HookLogChannel`. x1=frame and x2=result match the hook.

### 15. gameIGameSystem_* and CPhotoMode_SetRecordID — UNSURE, Mac twins not identified

All six are calls, not hooks, in `src/reverse/RTTIExtender.cpp`. Windows RVAs are from `win-addr.tsv`. No Mac RVA is claimed.

| hash | Windows | win-addr name | CET signature | site |
|---|---|---|---|---|
| 385618721 Initialize | 0xac7ed4 | (unnamed; comment in Addresses.h says the hash should probably be 3114931869, `spawn::Set::Initialize`, and that the hashed function is the weird one) | `void(TEMP_Spawner*, GameInstance*)` | RTTIExtender.cpp:227-231 |
| 3313306514 UnInitialize | 0x204d490 | spawn::Set::Deinitialize | `void(TEMP_Spawner*)` | :234-238 |
| 2509382878 Spawn | 0x661538 | spawn::Set::SpawnObject | `void(TEMP_Spawner*, REDSmartPtr* sret, TEMP_SpawnSettings&, CName)` — sret is the second argument | :241-248 |
| 3168866665 Despawn | 0x65cefc | spawn::Set::DespawnObject | `void(TEMP_Spawner*, IScriptable*)` | :282-286 |
| 2840271332 SpawnCallback | 0x8b57f4 | world::RuntimeEntityRegistry::RegisterEntity | `void(IScriptable*, ent::Entity*)` | :439-442 |
| 4052428712 SetRecordID | 0x661900 | (unnamed) | `void(const TweakDBID, Handle<IScriptable>&)` — TDBID by value | :191-198, called at :278 |

Windows `Initialize` `0xac7ed4` (through `0xac7f5a`) starts by writing `[rdx+0x78]`, then `[that+0xa0]`, `[that+0x48]`, `mov r8,[rax+0x229ef0]`, stores `[rcx+0x60]`, and vcalls `[rdx]+8` with a global at `0x342f798`. A scan for `add #0x229, lsl #12` (the `0x229ef0` fingerprint) returned 30 hits, all in `0xcf4bcc`, which adds `0x229f38`, `0x2265b0`, and similar immediates to `this`. That is a different large object, not the nested load `[rax+0x229ef0]`. It is not a match.

Windows `SetRecordID` `0x661900`–`0x661963` saves rcx (the TDBID by value) and rdx (the Handle*), calls `0x660258`, asserts through `0x1ecc5e8` if `[rbx]!=0`, calls `0x660168`, calls `0x100c10` (handle assign), and runs two cleanups through `0x102080`.

UnInitialize, Spawn, Despawn, and SpawnCallback were not re-disassembled in the last pass. Do not invent Mac entries for any of the six.

## DB and SDK

| hash | JSON DB | use this | why |
|---|---|---|---|
| 3791200470 RunPureScript | `1:0x950AB8` | `0x2172bf4` | DB function compares `"No"` and takes more than 3 args. SDK `Relocation-inl.hpp:155` already has `0x2172BF4`. |
| 404169501 InternalExecute | `1:0x94FE44` | `0x2173120` | DB function is sret (`mov x19,x8`). SDK line 153 already has `0x2173120`. |
| 3602585178 TweakDBLoad | `1:0x2B75570` | `0x2b75570` | DB matches the body, including the +0x160 byte. |

## What the hook signature can keep

| function | keep the CET signature? |
|---|---|
| four OnTick | Yes. `bool(this, app)`, w0 is a real bool. Slot is Mac +0x28. |
| AllocateFunction | No entry to hook. The Windows body already ignores both arguments. |
| TranslateBytecode | No. Hook `0x21e897c` (4 args, returns bool) and forward them. Do not hook `0x21ebbb0`. |
| OnPlayerSpawned | Yes for the 4 args. Hook body `0x3fd4d70`, not thunk `0x3fd788c`. Forwarded x0 is leftover. |
| RunPureScript | Yes for the 3 pointers. w0 is always 1, and CET forwards it. |
| TweakDBLoad | Yes for `(this, param)`. +0x160 is the right field. Forwarded x0 is leftover. |
| InternalExecute | Yes. Called as 5 pointers, bool in w0. Use `0x2173120`. |
| TDBIDConstructorDerive | No. Two args, TDBID returned in x0. The 3-arg hook reads the name pointer as `apThis`. |
| ToStringDEBUG and the five Log handlers | Yes. 4-arg native handlers, same `CStackFrame` offsets. CET replaces the body. |
| ForceCursor | No such function. Call view `ShowCursor` `0x1075238`. |
| IGameSystem / SetRecordID | Not identified. |

---

# Part 2: CET macOS structure layout (Cyberpunk 2077 2.3.1, arm64)

Static comparison of the structures Cyber Engine Tweaks (CET) 1.37.1 touches directly against the Apple Silicon game. Mac RVA = address − 0x100000000. SDK under review: `RED4ext.SDK-macos` (the `__APPLE__` headers). `RED4EXT_ASSERT_OFFSET` / `RED4EXT_ASSERT_SIZE` are empty under this SDK (`include/RED4ext/Common.hpp`), so the header numbers are documentation until Mac code confirms them.

CET source checked: `cet/CyberEngineTweaks/src/scripting/FunctionOverride.cpp`, `LuaVM_Hooks.cpp`, `NativeProxy.cpp`, `RTTIHelper.cpp`, `RTTIExtender.cpp`, `GameOptions.h`, `reverse/BasicTypes.h`, `reverse/RenderContext.h`.

Verdicts: **MATCH** = Mac instruction uses the SDK offset/size. **DIFFERS** = Mac instruction uses a different offset or size. **UNSURE** = not enough Mac code to decide.

## Table

| type.field | SDK (Mac view) | Mac code | verdict | evidence |
|---|---|---|---|---|
| CBaseFunction size | 0xB0 | fields written through +0xAC; derived objects start at +0xB0 | MATCH | Class ctor `0x2173760`: `stp w24, wzr, [x19, #0xa8]` then `str x22, [x19, #0xb0]`. Dtor `0x2172010` frees returnType at +0x18, params ptr +0x28 / count +0x34, localVars ptr +0x38 / count +0x44, hashmap at +0x48, bytecode at +0x80. |
| CBaseFunction.fullName | +0x08 CName | +0x08 | MATCH | Class ctor `0x2173790`: `stp x8, x2, [x0]` stores vptr and fullName (x2). |
| CBaseFunction.shortName | +0x10 | +0x10 | MATCH | `0x21737a0`: `str x3, [x0, #0x10]`. |
| CBaseFunction.returnType | +0x18 | +0x18 | MATCH | `0x2173794`: `stp xzr, xzr, [x0, #0x18]` zeros returnType and unk20. |
| CBaseFunction.unk20 | +0x20 | +0x20 | MATCH | Same `stp` as returnType. |
| CBaseFunction.params | +0x28 DynArray (0x10) | ptr +0x28, size +0x34 | MATCH | `0x217379c` constructs at `this+0x28`; `0x21737b8`: `str wzr, [x19, #0x34]`. Cleanup `0x2172aec`: `ldr x8, [x22, #0x28]` (entries), `0x2172abc`: `ldr w8, [x0, #0x34]` (size). |
| CBaseFunction.localVars | +0x38 DynArray | ptr +0x38, size +0x44 | MATCH | `0x21737c4` constructs at `this+0x38`; `0x21737dc`: `str wzr, [x19, #0x44]`. |
| CBaseFunction.unk48 | +0x48 HashMap, 0x30 bytes | zeros +0x48..+0x68, sentinel +0x70 | MATCH | `0x21737f4`–`0x2173808`: `stur q0, [x19, #0x48]`, `stur q0, [x19, #0x58]`, `str d0, [x19, #0x68]` (0xFFFFFFFF), `str x8, [x19, #0x70]`. |
| CBaseFunction.bytecode | CCompiledCode +0x78; code pointer consumed at +0x80 | ctor at +0x78; reader uses +0x80 | MATCH | `0x2173814`: `add x0, x19, #0x78; bl 0x21fd30c`. Frame builder `0x21ff064`: `add x0, x3, #0x80; bl 0x225ff68`; result stored as frame.code. |
| CBaseFunction.flags | +0xA8 u32; bit0 isNative, bit1 isStatic | +0xA8 | MATCH | Class ctor ORs flags with 1 (`0x2173798`: `orr w24, w6, #1`) and stores at +0xA8. Static ctor ORs with 3 (`0x2173898`: `orr w23, w5, #3`). InternalExecute `0x2173168`: `ldrb w8, [x0, #0xa8]; tbnz w8, #0`. |
| CBaseFunction.unkAC | +0xAC i32 (local-var byte size) | +0xAC | MATCH | Stored as the high half of the flags `stp` (`wzr`). InternalExecute `0x2173170`: `ldr w8, [x22, #0xac]` then aligns and allocates. RunPureScript uses the same offset (the virtual at script-stack slot +0x28 does `ldr w1, [x1, #0xac]`). |
| CGlobalFunction size | 0xB8 | heap alloc 0xB8; last store is regIndex at +0xB0 | MATCH | Global ctor `0x2173ab8`: `str w0, [x19, #0xb0]` and no store past that. Cast registrar allocates with `mov w0, #0xb8` before the global thunk `0x2173ad4` (same pattern as the scripted site below). |
| CGlobalFunction.regIndex | +0xB0 u32 | +0xB0 | MATCH | `0x2173ab8`: `str w0, [x19, #0xb0]`. Vtable `0x6FFACC8` slot +0x20 is `0x2173da8`: `ldr w0, [x0, #0xb0]; ret`. |
| CGlobalFunction.flags initializer | flags at +0xA8 | 8-byte load from `0x6ADAE88` stored at +0xA8 | MATCH | `0x2173a94`: `adrp x8, #0x6ada000; ldr d0, [x8, #0xe88]; str d0, [x19, #0xa8]`. Covers flags and unkAC together. |
| CClassFunction size | 0xC0 | static objects 0xC0 apart; last field at +0xB8 | MATCH | Registrar `0xAA417C`. Fast path: guard `0x7543520`, object `0x7543650`; next guard `0x7543528`, object `0x7543710`. Difference 0xC0. Slow path `0xAA4434` / `0xAA4494` builds the same two addresses (`x20+0x130` with x20=`0x7543520`, then `x20+0x1E8` with x20=`0x7543528`). Ctor writes regIndex at +0xB8, which fits in 0xC0 with 4 bytes of tail padding. |
| CClassFunction.parent | +0xB0 | +0xB0 | MATCH | `0x2173830`: `str x22, [x19, #0xb0]`. GetParent slot `0x2173d98`: `ldr x0, [x0, #0xb0]; ret`. |
| CClassFunction.regIndex | +0xB8 u32 | +0xB8 | MATCH | `0x2173840`: `str w0, [x19, #0xb8]`. GetRegIndex slot `0x2173da0`: `ldr w0, [x0, #0xb8]; ret`. |
| CClassStaticFunction size | 0xC0 (same as CClassFunction) | static objects 0xC0 apart | MATCH | Registrar `0x1F2F524`, page `0x7CCC000`. Object 1 = `0x7CCCB10` (fast path `x20=0x270` plus `0x8A0`; slow path `x20=0x268` plus `0x8A8`). Object 2 = `0x7CCCBD0` (`x20=0x278` plus `0x958`). Difference 0xC0. Both call thunk `0x217395C` → ctor `0x2173864`. A raw subtract of the slow-path immediates (0x960−0x8A8=0xB8) is wrong: x20 changes by 8 between the two stubs. |
| CClassStaticFunction.parent / regIndex | +0xB0 / +0xB8 | same | MATCH | `0x2173930`: `str x21, [x19, #0xb0]`. `0x217393C`: `str w0, [x19, #0xb8]`. Same vptr as CClassFunction (`0x6FFAC88`), so GetParent/GetRegIndex are the same two getters. |
| CClassFunction::Create ABI | Mac SDK passes this-adjust 0 (`Functions.hpp` `__APPLE__` branch) | ctor is (this, parent, fullName, shortName, handler, adjust, flags) | MATCH | `0x2173778`–`0x2173798`: x5 kept as adjust (passed to `0x21FD4C8`), w6 ORed into flags. Static ctor has no adjust: w5 is flags (`0x2173898`), handler call is `bl 0x21FD4F4` with x0 only. SDK `CClassStaticFunction::Create` has no Apple branch and that Windows signature matches. |
| CScriptedFunction size | 0xB8 (parent at +0xB0, no regIndex) | `mov w0, #0xb8` then scripted ctor | MATCH | Script loader `0x21EA7C4`: `mov w0, #0xb8; bl 0x4a3bc5c; bl 0x2173c28`. Scripted ctor stores parent at +0xB0 and does not write +0xB8. |
| IFunction vtable after the destructor | header comments: GetParent +0x10, GetRegIndex +0x18, GetInvokable +0x20 | GetParent +0x18, GetRegIndex +0x20, GetInvokable +0x28 | DIFFERS from the comments; matches Itanium | Class vptr `0x6FFAC88`: +0x00 `0x2173D80` GetAllocator, +0x08 `0x941D1C` complete dtor, +0x10 `0x2173D84` deleting dtor, +0x18 GetParent, +0x20 GetRegIndex, +0x28 `0x339B2C` GetInvokable. Global vptr `0x6FFACC8` has the same shift; its GetRegIndex reads +0xB0. One vptr at offset 0. The extra slot does not grow the object. |
| CStackFrame.code | +0x00 u8* | +0x00 | MATCH | Builder `0x21FF06C`: `stp x0, x25, [x19]` (x0 is the pointer returned from function+0x80). Opcode walker `0x9439B0`: `ldr x8, [x1]; add x9, x8, #1; str x9, [x1]; ldrb w8, [x8]`. |
| CStackFrame.func | +0x08 | +0x08 | MATCH | Same `stp` writes the function (original x3) at +0x08. |
| CStackFrame.localVars / params | +0x10 / +0x18 | two pointers at +0x10 / +0x18 | MATCH | `0x21FF070`: `stp x24, x23, [x19, #0x10]` (incoming x4, x5). |
| CStackFrame.unk20 / unk28 | +0x20 / +0x28 | +0x20 zero, +0x28 pointer | MATCH | `0x21FF074`: `str xzr, [x19, #0x20]`. `0x21FF07C`: `str x22, [x19, #0x28]` or copied from parent+0x28. |
| CStackFrame.data / dataType | +0x30 / +0x38 | +0x30 pair | MATCH | `0x21FF094`: `ldp x8, x9, [x20, #0x30]; stp x8, x9, [x19, #0x30]`. Opcode walker `0x94399C`: `stp xzr, xzr, [x1, #0x30]`. |
| CStackFrame.context / parent | +0x40 / +0x48 | +0x40 / +0x48 | MATCH | `0x21FF09C`: `stp x21, x20, [x19, #0x40]`. Opcode walker `0x9439AC`: `ldr x0, [x1, #0x40]`. |
| CStackFrame.unk50 / unk58 | +0x50 u16 / +0x58 i64 | halfword +0x50, qword +0x58 | MATCH | `0x21FF0A0`: `ldrh w8, [x20, #0x50]` then `strh` at +0x50. `0x21FF0C8`: `str x8, [x19, #0x58]`. |
| CStackFrame.paramFlags | +0x60 u16 | +0x60 u16 | MATCH | Builder `0x21FF0CC`: `str wzr, [x19, #0x60]` zeros the dword covering paramFlags, currentParam, useDirectData. Cleanup `0x2172AFC`: `ldrh w28, [x21, #0x60]` (x21 = frame). |
| CStackFrame.currentParam | +0x62 u8 | +0x62 | MATCH | Opcode walker `0x9439A0`: `ldrb w8, [x1, #0x62]; add w8, w8, #1; strb w8, [x1, #0x62]`. This is the byte CET increments in `HandleOverridenFunction` and the LuaVM hooks. |
| CStackFrame.useDirectData | +0x63 bool | covered by the dword zero at +0x60 | UNSURE | No lone `ldrb [frame, #0x63]` was isolated. CET does not read this field. The dword store is consistent with the SDK packing. |
| CBaseStack.context18 / context20 / unk28 / unk10 | +0x18 / +0x20 / +0x28 / +0x10 | same | MATCH | Every stack ctor does `stp x?, x1, [x0, #0x10]` and `str x1, [x0, #0x20]`, then stores the `bl 0x223809C` result at +0x28. GetContext `0x21742CC` returns `[this+0x18]`, else the IScriptable path uses `[this+0x28]` and `[this+0x20]`. |
| CStack.args / argsCount / result | +0x30 / +0x38 u32 / +0x40 CStackType* | same | MATCH | Ctor `0x217446C`: `str x3, [x0, #0x30]; str w4, [x0, #0x38]; str x5, [x0, #0x40]`; vptr `0x6FFAEA0` (`add x8, x8, #0xea0` at `0x2174478`). Ctor `0x21744F4`: `stp x0, x21, [x22, #0x28]` (unk28, args), `str w20, [x22, #0x38]`, `str x19, [x22, #0x40]`. |
| CStack.GetResultAddr / GetResultType | result at +0x40; CStackType.value +0x08, type +0x00 | same, vtable slots +0x10 / +0x18 | MATCH | Vptr `0x6FFAEA0` slot +0x10 = `0x21746F8`: `ldr x8, [x0, #0x40]; ldr x0, [x8, #8]`. Slot +0x18 = `0x2174710`: `ldr x8, [x0, #0x40]; ldr x0, [x8]`. Windows slots are +0x08 / +0x10; Mac is +8 after the two destructor slots. |
| CScriptStack.args / value / type | +0x30 u8* / +0x38 / +0x40; size 0x48 | same; vptr `0x6FFAF28` | MATCH | Ctor `0x2174AC8`: `stp x0, x21, [x22, #0x28]` (unk28, args=x2), `stp x20, x19, [x22, #0x38]` (value=x3, type=x4). Locals that hold this object are 0x48 bytes (`0x940780`: object at x29−0x70, next local at x29−0x28). GetResultAddr slot +0x10 = `0x15CCC80`: `ldr x0, [x0, #0x38]`. GetResultType slot +0x18 = `0x2174BFC`: `ldr x0, [x0, #0x40]`. This is the object passed to RunPureScript by 173 functions via thunk `0x2173390` and all 161 callers of thunk `0x217347C`. |
| CStack passed into RunPureScript | CET's hook types the argument as CScriptStack | also receives a real CStack | MATCH for both SDK types | Thunk `0x21732C4` (10 callers, 7 functions) always constructs with CStack ctor `0x21744F4`. Thunk `0x2173390` also has 13 functions on ctor `0x217446C` and 33 on `0x21744F4`. Thunks do not touch x0/x1; they `bl 0x2172BF4`. CET's `apStack->value != GetResultAddr()` distinguishes them: CScriptStack.value is the result pointer, CStack's +0x38 is argsCount, and CStack.args at +0x30 is CStackType*. |
| Third stack (not in the SDK) | none; CET comment says two CScriptStack shapes | vptr `0x6FFAEE8`, result at +0x48, a pointer at +0x38, args at +0x30, dword at +0x54; about 0x58 bytes | UNSURE for CET's array-args branch | Ctor `0x2174728`: vptr store at `0x2174764`, `stp x0, x20, [x21, #0x28]`, `stp x19, xzr, [x21, #0x38]`, constructs at `this+0x48`, `str wzr, [x21, #0x54]`. GetResultAddr slot +0x10 = `0x2174A04`: `ldr x0, [x0, #0x48]`. GetResultType slot +0x18 = `0x2174A0C`: `ldr x0, [x0, #0x40]`. Two call sites (`0x223C2F0`, `0x223C7E8`) then call RunPureScript thunk `0x2173390`. They pass args = `sp` and value = a small local. `value (+0x38) != GetResultAddr (+0x48)`, so CET takes the CStackType-array branch. Nothing here shows that `sp` is a CStackType array. |
| CProperty.valueOffset | +0x20 u32 | +0x20 | MATCH | Cleanup `0x2172AF8`: `ldr w19, [x23, #0x20]` where x23 is `params[i]` from entries at function+0x28. This is the offset CET adds to a byte-args base. |
| CProperty flag byte | flags u64 at +0x28; isOut is bit 9 | a byte is loaded at +0x2A | UNSURE which bit | `0x2172B14`: `ldrb w8, [x23, #0x2a]`. Bit 9 of a u64 is byte +0x29 bit 1, so this load is not by itself proof of isOut. valueOffset is the field CET needs and it matches. |
| OpcodeHandlers table | Mac SDK `0x908B798` | page `0x908B000` + 0x798 | MATCH | Walker `0x9439C0`: `adrp x22, #0x908b000; add x22, x22, #0x798; ldr x8, [x22, x8, lsl #3]; blr x8`. The table itself is in zerofill `__DATA` (file has no pointer bytes there). The walker above is the consumer, and it passes the frame in x1. |
| TweakDB.unk160 | +0x160 u8 | +0x160 | MATCH | Load (hash 3602585178): Windows `0x50EC26` `inc byte ptr [rdi + 0x160]` (rdi = this); Mac `0x2B75650` `ldrb w8, [x20, #0x160]; add w8, w8, #1; strb w8, [x20, #0x160]` (x20 = this). Reader: Mac `0x2B743E0` `ldrb w0, [x8, #0x160]` on the singleton at `0x80C92D0`. Windows `0x3447E2` `mov al, byte ptr [rsi + 0x160]; xchg byte ptr [rdi + 9], al` stamps that byte into the TweakDBID at +9. Mac `0x9C8520` compares the byte at ID+9 with `0x2B743C0`. CET's `unk160 = 0` (`LuaVM_Hooks.cpp:340`) is the same byte. |
| TweakDB.flatDataBuffer / capacity / end | +0x148 / +0x150 / +0x158 | not re-derived | not re-checked | The +0x160 accesses above sit immediately after this triple in the SDK. No Mac instruction in the load/reader disagreed with +0x160, which is the field CET writes. |
| Function allocator, hash 160045886 | CET hooks one function and passes a larger size | Windows ignores the size; Mac inlines 0xB8 | DIFFERS in call shape, not in object size | Windows `0x2046E96`: `mov edx, 0xb8` then call, so the hooked size is discarded. Mac scripted site `0x21EA7C4`: `mov w0, #0xb8; bl 0x4a3bc5c`. No single Mac function for this hash; 105 direct calls of `0x4A3BC5C`. Native class/static functions are constructed in static storage, not through this allocator. |

## Sizes CET memcpy-swaps

`GetFunctionSize` (`FunctionOverride.cpp`) picks `sizeof(CClassStaticFunction)` for static native, `sizeof(CGlobalFunction)` for static non-native, and `sizeof(CClassFunction)` for everything else. `s_cMaxFunctionSize` is the max of those three. On the Mac SDK those sizes are 0xC0, 0xB8, and 0xC0.

| object | SDK size | Mac allocation | GetFunctionSize when CET swaps it | verdict |
|---|---|---|---|---|
| Native CClassFunction | 0xC0 | static stride 0xC0 | 0xC0 (not static) | MATCH |
| Native CClassStaticFunction | 0xC0 | static stride 0xC0 | 0xC0 (static and native) | MATCH |
| CGlobalFunction | 0xB8 | heap 0xB8 | 0xB8 (static, not native) | MATCH |
| CScriptedFunction | 0xB8 | heap 0xB8 (`mov w0, #0xb8`) | 0xC0 if `flags.isStatic` is clear, because the helper has no scripted case | DIFFERS |

Copy of the description fields (`CopyFunctionDescription`) uses fullName, shortName, returnType, params, localVars, unk20, unk48, unkAC, flags, bytecode, and parent. Every one of those offsets matches the Mac ctors above.

## Fixes CET needs on Mac

1. Scripted function bodies are 0xB8, not 0xC0. `GetFunctionSize` returns `sizeof(CClassFunction)` (0xC0) for every non-static function, and the three-way `memcpy` in `Override` copies that many bytes. A scripted object has parent at +0xB0 and nothing at +0xB8, and it is allocated with `mov w0, #0xb8`. Copying 0xC0 overflows the block by 8. The same sizes exist on Windows; this is not an Itanium-only layout change.

2. Hooking hash 160045886 does not enlarge Mac allocations. Windows `0x2046E90` overwrites the size with `mov edx, 0xb8`. On Mac the size is an immediate at each call site (`bl 0x4a3bc5c`), and native functions are not heap-allocated at all. `HookCreateFunction`'s `max(s_cMaxFunctionSize, sizeof(CScriptedFunction))` never reaches those sites.

3. Do not hardcode the Windows vtable slot numbers from the SDK comments. A virtual destructor is two slots, so IFunction's GetParent / GetRegIndex / GetInvokable are at +0x18 / +0x20 / +0x28, and IStack's GetResultAddr / GetResultType are at +0x10 / +0x18. Field offsets are unchanged, and a C++ virtual call compiled from the SDK declarations uses the Mac slots. CET's swap path uses fields, not these slots.

4. `CClassFunction::Create` already passes the Mac this-adjust argument. `CClassStaticFunction::Create` needs no extra argument: the Mac ctor takes flags in w5 and has no adjust. Leave those signatures as the Mac SDK has them.

5. `TweakDB::unk160` stays at +0x160. The load increments it (Windows `0x50EC26`, Mac `0x2B75650`) and the cache check reads it (Mac `0x2B743E0`). CET's store of 0 is the right byte. No offset fix.

6. `CStackFrame` fields CET walks (`code`, `func`, `context`, `paramFlags`, `currentParam`, `data`, `dataType`) match, including currentParam as the byte at +0x62. `CStack` and the common `CScriptStack` (vptr `0x6FFAF28`) match, and CET's `value != GetResultAddr()` test tells them apart correctly. The third stack (vptr `0x6FFAEE8`, two call sites into the hooked function) keeps the result at +0x48. Overrides that hit that path need a separate look before trusting the CStackType-array branch.

7. Not confirmed, so do not treat them as Mac-ready. `RenderContext` (`reverse/RenderContext.h`) is a Windows D3D12 map (`devices` at 0xC97F38, queue at 0x13BC4D0); the Mac game is not that object. `GameOption` (`unk28` +0x28, `Boolean` +0x30, `Integer.isBounded` +0x40) was not checked against Mac code. `TEMP_SpawnSettings` / `TEMP_Spawner` in `RTTIExtender.cpp` are marked as guesses there and were not checked. `CClass`, `CEnum`, `Handle` / `WeakHandle`, `CString`, `CRTTISystem`, `ScriptGameInstance` (CET asserts 0x18), and TweakDB flat records were not re-derived; nothing in the code above contradicted the fields CET actually reads (DynArray and HashMap inside function objects, and `CProperty.valueOffset`, do match).
