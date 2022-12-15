#include <stdafx.h>

#include "RTTIMapper.h"
#include "BasicTypes.h"
#include "scripting/Scripting.h"
#include "Utils.h"

RTTIMapper::RTTIMapper(const LockableState& acpLua, LuaSandbox& apSandbox)
    : m_lua(acpLua)
    , m_sandbox(apSandbox)
{
}

RTTIMapper::~RTTIMapper()
{
}

void RTTIMapper::Register()
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();
}

void RTTIMapper::Refresh()
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    sol::table luaGlobal = luaState.globals();
}

void RTTIMapper::RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal) const
{
   
}

void RTTIMapper::RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti)
{
    const bool reinit = aLuaGlobal["EnumStatic"] != sol::nil;

    if (!reinit)
    {
    }
}

void RTTIMapper::RegisterDirectGlobals(sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti)
{
}

void RTTIMapper::RegisterScriptAliases(sol::table& aLuaGlobal, red3lib::CRTTISystem* apRtti)
{
}

void RTTIMapper::RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal) const
{
    // Merge RTTI versions of basic types with our own versions
    // Allows usertype and RTTI functions to be used under the same name
   // ExtendUsertype<Vector4>("Vector4", aLuaState, aLuaGlobal);
   // ExtendUsertype<EulerAngles>("EulerAngles", aLuaState, aLuaGlobal);
   // ExtendUsertype<Quaternion>("Quaternion", aLuaState, aLuaGlobal);

    // Replace RTTI version of class with our own version
    aLuaGlobal["Vector3"] = aLuaState["Vector3"];
}

void RTTIMapper::SanitizeName(std::string& aName)
{
    std::ranges::replace(aName, '.', '_');
}

red3lib::CName RTTIMapper::TryResolveTypeName(sol::object aValue)
{
    if (IsLuaCData(aValue))
        return L"Uint64";

    switch (aValue.get_type())
    {
    case sol::type::string: return L"String";

    case sol::type::boolean: return L"Bool";

    case sol::type::userdata:
    {
        sol::userdata userdata = aValue;
        const sol::table metatable = userdata[sol::metatable_key];
        const auto name = metatable.raw_get_or<std::wstring>(L"__name", L"");

        if (!name.empty())
            return name.c_str() + 4;

        break;
    }

    case sol::type::table:
    {
        sol::table table = aValue;

        if (!table.empty())
        {
            const red3lib::CName innerType = TryResolveTypeName(table[1]);

            if (!innerType.IsNone())
                return std::wstring(L"array:").append(innerType.AsChar()).c_str();
        }

        break;
    }
    }

    return {};
}
