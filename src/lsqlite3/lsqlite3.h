#pragma once

#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_lsqlite3(lua_State* L);

#ifdef __cplusplus
}
#endif