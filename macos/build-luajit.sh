#!/bin/bash
# Builds the openresty LuaJIT (the Lua CET uses on Windows, GC64) as a static arm64 library for the macOS dylib.
# Usage: build-luajit.sh <luajit2 source dir> <output dir>. The source is copied first, so the checkout stays clean.
set -euo pipefail
SRC="$1"; OUT="$2"
[[ -f "$SRC/src/luajit.h" || -f "$SRC/src/luajit_rolling.h" ]] || { echo "build-luajit.sh: $SRC is not a LuaJIT checkout" >&2; exit 1; }
STAMP="$OUT/.built-$(git -C "$SRC" rev-parse --short HEAD 2>/dev/null || echo src)"
if [[ -f "$STAMP" && -f "$OUT/lib/libluajit-5.1.a" ]]; then exit 0; fi
rm -rf "$OUT/work" && mkdir -p "$OUT/work" "$OUT/lib" "$OUT/include"
cp -R "$SRC/." "$OUT/work/"
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-14.0}"
make -C "$OUT/work" -j8 amalg BUILDMODE=static XCFLAGS="-DLUAJIT_ENABLE_LUA52COMPAT" TARGET_FLAGS="-arch arm64" >"$OUT/build.log" 2>&1 || { tail -40 "$OUT/build.log" >&2; exit 1; }
cp "$OUT/work/src/libluajit.a" "$OUT/lib/libluajit-5.1.a"
for h in lua.h lualib.h lauxlib.h luaconf.h lua.hpp luajit.h; do cp "$OUT/work/src/$h" "$OUT/include/"; done
rm -f "$OUT"/.built-* && touch "$STAMP"
echo "LuaJIT built: $OUT/lib/libluajit-5.1.a"
