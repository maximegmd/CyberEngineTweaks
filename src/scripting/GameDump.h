#pragma once

#include "Game_Hooks.h"
#include "common/Meta.h"

#include <map>
#include <set>
#include <unordered_set>

namespace RED4ext
{
struct CClass;
struct IRTTIType;
struct CProperty;
} // namespace RED4ext

namespace GameDump
{
struct DumpVTablesTask : MainThreadTask
{
    virtual void Run() override;
    virtual void Dispose() override;
};

} // namespace GameDump
