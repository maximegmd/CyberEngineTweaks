// CET macOS: the arm64 stand-in for FunctionOverride's xbyak thunk. On Windows each overridden script function gets a few bytes
// of generated x86 that call FunctionOverride::HandleOverridenFunction with the real function as an extra (5th) argument. Here a fixed
// pool of stubs is assembled into the dylib instead (no code is written at runtime). Stub i loads cet_override_slots[i] (the real
// function, set by FunctionOverride before the stub is used) into x4, the 5th integer argument, and tail-calls the address in
// cet_override_dispatch (HandleOverridenFunction). x0..x3 (context, frame, out, out type) pass through unchanged.
//
// Every stub is exactly 6 instructions (24 bytes): stub i is at cet_override_thunks + 24 * i.

#define CET_OVERRIDE_THUNK_COUNT 4096

    .section __TEXT,__text,regular,pure_instructions
    .p2align 4
    .private_extern _cet_override_thunks
_cet_override_thunks:
    .set thunk_index, 0
    .rept CET_OVERRIDE_THUNK_COUNT
    adrp x16, _cet_override_slots@PAGE
    add x16, x16, _cet_override_slots@PAGEOFF
    ldr x4, [x16, #(thunk_index * 8)]
    adrp x17, _cet_override_dispatch@PAGE
    ldr x17, [x17, _cet_override_dispatch@PAGEOFF]
    br x17
    .set thunk_index, thunk_index + 1
    .endr
    .private_extern _cet_override_thunks_end
_cet_override_thunks_end:

    .section __DATA,__data
    .p2align 3
    .private_extern _cet_override_dispatch
_cet_override_dispatch:
    .quad 0
    .private_extern _cet_override_slots
_cet_override_slots:
    .space CET_OVERRIDE_THUNK_COUNT * 8
