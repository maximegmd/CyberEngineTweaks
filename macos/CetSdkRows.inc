        // CET macOS rows (address survey, macos/ADDRESS_SURVEY.md). Format: { hash, Mac RVA },  // name. Only MAPPED rows (body
        // compared with the Windows function) go here; anything else resolves to the logged trap stub (macos/MacShims.cpp).
        { 3592689218, 0x3D8D820 },  // CRunningState_OnTick (red::GameAppRunningState::OnTick; vtable 0x726F160 slot +0x28)
        { 2447710505, 0x3D8D710 },  // CInitializationState_OnTick (vtable 0x726F118 slot +0x28)
        { 4233370276, 0x3D8C0CC },  // CBaseInitializationState_OnTick (vtable 0x726F0C0 slot +0x28)
        { 4069332669, 0x3D8D94C },  // CShutdownState_OnTick (vtable 0x726F1A8 slot +0x28)
        { 2050111212, 0x3FD4D70 },  // PlayerSystem_OnPlayerSpawned (the body; Codeware hooks it too, CET chains)
        { 326438016, 0x3453B14 },   // CScript_TDBIDConstructorDerive: macOS ABI uint64_t(const TDBID* base, const char* name)
        { 3515162577, 0x2B79D10 },  // CScript_ToStringDEBUG (native handler of TweakDBID.ToStringDEBUG)
        { 3455393801, 0x22111F8 },  // CScript_Log (native handler)
        { 2135235617, 0x22112D8 },  // CScript_LogError (native handler)
        { 3222609133, 0x22113B8 },  // CScript_LogWarning (native handler)
        { 1663049434, 0x2211498 },  // CScript_LogChannel (native handler)
        { 2841780134, 0x22116C8 },  // CScript_LogChannelWarning (native handler)
