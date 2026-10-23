#pragma once

// CET macOS: macOS hook targets that have no address hash of their own (macos/ADDRESS_SURVEY.md). Each is used only after its
// first instructions match, so a different game build skips the feature with a log line instead of hooking the wrong code.

#include <cstdint>

namespace CetMac::Rva
{
// The script binder's load function, bool(binder, source, x2, x3). On Windows CET hooks CScriptDataBinder::LoadOpcodes
// (CScript_TranslateBytecode, 3442875632), called once per load from this function's Windows twin; on macOS LoadOpcodes is inlined
// here (survey note 6). Prologue: sub sp, sp, #0x80; stp x26, x25; stp x24, x23; stp x22, x21.
constexpr uintptr_t ScriptBinderLoad = 0x1113E44;
constexpr uint32_t ScriptBinderLoadPrologue[] = {0xD10203FF, 0xA90367FA, 0xA9045FF8, 0xA90557F6};
} // namespace CetMac::Rva
