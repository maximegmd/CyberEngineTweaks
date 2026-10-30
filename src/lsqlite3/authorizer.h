#pragma once

// SQLite authorizer installed on every database lsqlite3 opens (see lsqlite_do_open). Header-only so it can be
// unit-tested on its own.

#include <cstring>

#include "sqlite3.h"

/* Mods get a database confined to their own folder. Without an authorizer, SQL could reach other files:
** ATTACH DATABASE '<any path>' (and VACUUM INTO, which attaches internally) reads, creates or overwrites files
** anywhere the game can write, and the SQL function load_extension() would load native code. */
inline int lsqlite_authorizer(void*, int action, const char* arg1, const char* arg2, const char*, const char*)
{
    switch (action)
    {
    case SQLITE_ATTACH:
        /* Only temporary or in-memory databases. arg1 is null when the file name is not a string literal, so a
        ** computed name is refused too. */
        if (arg1 != nullptr && (arg1[0] == '\0' || std::strcmp(arg1, ":memory:") == 0))
            return SQLITE_OK;
        return SQLITE_DENY;
    case SQLITE_PRAGMA:
        if (arg1 != nullptr && (sqlite3_stricmp(arg1, "temp_store_directory") == 0 || sqlite3_stricmp(arg1, "data_store_directory") == 0))
            return SQLITE_DENY;
        return SQLITE_OK;
    case SQLITE_FUNCTION:
        /* arg2 is the function name */
        if (arg2 != nullptr && sqlite3_stricmp(arg2, "load_extension") == 0)
            return SQLITE_DENY;
        return SQLITE_OK;
    default: return SQLITE_OK;
    }
}
