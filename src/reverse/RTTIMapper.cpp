#include <stdafx.h>

#include "RTTIMapper.h"
#include "RTTIHelper.h"
#include "BasicTypes.h"
#include "Enum.h"
#include "EnumStatic.h"
#include "ClassStatic.h"
#include "scripting/Scripting.h"
#include "Utils.h"

RTTIMapper::RTTIMapper(const LockableState& acpLua, LuaSandbox& apSandbox)
    : m_lua(acpLua)
    , m_sandbox(apSandbox)
{
}

RTTIMapper::~RTTIMapper()
{
    RTTIHelper::Shutdown();
}

void RTTIMapper::Register()
{
    RTTIHelper::Initialize(m_lua, m_sandbox);

    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    auto* pRtti = RED4ext::CRTTISystem::Get();

    RegisterSimpleTypes(luaState, m_sandbox.GetGlobals());
    RegisterDirectTypes(luaState, m_sandbox.GetGlobals(), pRtti);
    RegisterDirectGlobals(m_sandbox.GetGlobals(), pRtti);
    RegisterScriptAliases(m_sandbox.GetGlobals(), pRtti);
    RegisterSpecialAccessors(luaState, m_sandbox.GetGlobals());
}

void RTTIMapper::Refresh()
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    sol::table luaGlobal = luaState.globals();
    auto* pRtti = RED4ext::CRTTISystem::Get();

    RegisterDirectTypes(luaState, luaGlobal, pRtti);
    RegisterDirectGlobals(luaGlobal, pRtti);
}

void RTTIMapper::RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal) const
{
    aLuaGlobal.new_usertype<Variant>("Variant", sol::meta_function::construct, sol::no_constructor);
    MakeSolUsertypeImmutable(aLuaGlobal["Variant"], aLuaState);

    aLuaGlobal["ToVariant"] = sol::overload(
        [](const Type& aInstance, sol::this_state aState) -> sol::object
        {
            const auto* pType = aInstance.GetType();
            auto* pValue = aInstance.GetValuePtr();

            if (!pType || !pValue)
            {
                luaL_error(aState, "ToVariant: Invalid instance.");
                return sol::nil;
            }

            return {aState, sol::in_place, Variant(pType, pValue)};
        },
        [](const Enum& aEnum, sol::this_state aState) -> sol::object
        {
            RED4ext::CStackType data;
            data.type = const_cast<RED4ext::CEnum*>(aEnum.GetType());
            data.value = const_cast<void*>(aEnum.GetValuePtr());

            return {aState, sol::in_place, Variant(data)};
        },
        [](sol::object aValue, const std::string& aTypeName, sol::this_state aState) -> sol::object
        {
            auto* pType = RED4ext::CRTTISystem::Get()->GetType(aTypeName.c_str());

            if (!pType)
            {
                luaL_error(aState, "ToVariant: Unknown type '%s'.", aTypeName.c_str());
                return sol::nil;
            }

            Variant variant(pType);
            RED4ext::CStackType data(pType, variant.GetDataPtr());

            Scripting::ToRED(aValue, data);

            return {aState, sol::in_place, std::move(variant)};
        },
        [](sol::object aValue, sol::this_state aState) -> sol::object
        {
            const RED4ext::CName typeName = TryResolveTypeName(aValue);

            if (typeName.IsNone())
            {
                luaL_error(aState, "ToVariant: Cannot resolve type from value.");
                return sol::nil;
            }

            auto* pType = RED4ext::CRTTISystem::Get()->GetType(typeName);

            if (!pType)
            {
                luaL_error(aState, "ToVariant: Unknown type '%s'.", typeName.ToString());
                return sol::nil;
            }

            Variant variant(pType);
            RED4ext::CStackType data(pType, variant.GetDataPtr());

            Scripting::ToRED(aValue, data);

            return {aState, sol::in_place, std::move(variant)};
        });

    aLuaGlobal["FromVariant"] = [this](const Variant& aVariant) -> sol::object
    {
        if (aVariant.IsEmpty())
            return sol::nil;

        RED4ext::CStackType data;
        data.type = aVariant.GetType();
        data.value = aVariant.GetDataPtr();

        auto luaLock = m_lua.Lock();

        return Scripting::ToLua(luaLock, data);
    };
}

void RTTIMapper::RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti)
{
    const bool reinit = aLuaGlobal["EnumStatic"] != sol::nil;

    if (!reinit)
    {
        aLuaGlobal.new_usertype<EnumStatic>(
            "EnumStatic", sol::meta_function::construct, sol::no_constructor, sol::base_classes, sol::bases<Type>(), sol::meta_function::index, &EnumStatic::Index,
            sol::meta_function::new_index, &EnumStatic::NewIndex);
        MakeSolUsertypeImmutable(aLuaGlobal["EnumStatic"], aLuaState);

        aLuaGlobal.new_usertype<ClassStatic>(
            "ClassStatic", sol::meta_function::construct, sol::no_constructor, sol::base_classes, sol::bases<Type>(), sol::meta_function::index, &ClassStatic::Index,
            sol::meta_function::new_index, &ClassStatic::NewIndex, "new", property(&ClassStatic::GetFactory));
        MakeSolUsertypeImmutable(aLuaGlobal["ClassStatic"], aLuaState);
    }

    apRtti->types.for_each(
        [&](RED4ext::CName aTypeName, RED4ext::CBaseRTTIType* apType)
        {
            std::string typeName = aTypeName.ToString();
            SanitizeName(typeName);

            if (reinit && aLuaGlobal[typeName] != sol::nil)
                return;

            switch (apType->GetType())
            {
            case RED4ext::ERTTIType::Enum:
            {
                auto luaEnum = make_object(aLuaState, EnumStatic(m_lua, apType));

                MakeSolUsertypeImmutable(luaEnum, aLuaState);

                aLuaGlobal[typeName] = luaEnum;
                break;
            }
            case RED4ext::ERTTIType::Class:
            {
                auto luaClass = make_object(aLuaState, ClassStatic(m_lua, apType));

                MakeSolUsertypeImmutable(luaClass, aLuaState);

                aLuaGlobal[typeName] = luaClass;
                break;
            }
            }
        });
}

void RTTIMapper::RegisterDirectGlobals(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti)
{
    apRtti->funcs.for_each(
        [&aLuaGlobal](RED4ext::CName, RED4ext::CGlobalFunction* apFunc)
        {
            const std::string cShortName = apFunc->shortName.ToString();

            if (aLuaGlobal[cShortName] == sol::nil && !apFunc->flags.isExec)
            {
                const std::string cFullName = apFunc->fullName.ToString();
                const auto cIsClassFunc = cFullName.find("::") != std::string::npos;
                const auto cIsOperatorFunc = cShortName.find(';') != std::string::npos;

                if (!cIsClassFunc && !cIsOperatorFunc)
                {
                    aLuaGlobal[cShortName] = RTTIHelper::Get().ResolveFunction(cShortName);
                }
            }
        });
}

void RTTIMapper::RegisterScriptAliases(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti)
{
    apRtti->scriptToNative.for_each([&aLuaGlobal](RED4ext::CName aScriptName, RED4ext::CName aNativeName)
                                    { aLuaGlobal[aScriptName.ToString()] = aLuaGlobal[aNativeName.ToString()]; });
}

void RTTIMapper::RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal) const
{
    // Add global alias `Game.GetSystemRequestsHandler()`
    // Replacement for `GetSingleton("inkMenuScenario"):GetSystemRequestsHandler()`
    RTTIHelper::Get().AddFunctionAlias("GetSystemRequestsHandler", "inkMenuScenario", "GetSystemRequestsHandler");

    // Merge RTTI versions of basic types with our own versions
    // Allows usertype and RTTI functions to be used under the same name
    ExtendUsertype<Vector4>("Vector4", aLuaState, aLuaGlobal);
    ExtendUsertype<EulerAngles>("EulerAngles", aLuaState, aLuaGlobal);
    ExtendUsertype<Quaternion>("Quaternion", aLuaState, aLuaGlobal);
    ExtendUsertype<ItemID>("ItemID", aLuaState, aLuaGlobal);

    // Replace RTTI version of class with our own version
    aLuaGlobal["Vector3"] = aLuaState["Vector3"];
}

template <class T> void RTTIMapper::ExtendUsertype(const std::string acTypeName, sol::state& aLuaState, sol::table& aLuaGlobal) const
{
    auto& classStatic = aLuaGlobal.get<ClassStatic>(acTypeName);
    auto classIndexer = [&classStatic](const sol::object&, const std::string& acName, sol::this_environment aEnv)
    {
        return classStatic.Index(acName, aEnv);
    };

    auto usertype = aLuaState.get<sol::usertype<T>>(acTypeName);
    usertype[sol::meta_function::index] = classIndexer;
    usertype[sol::meta_function::static_index] = classIndexer;

    aLuaState["__" + acTypeName] = aLuaGlobal[acTypeName];
    aLuaGlobal[acTypeName] = aLuaState[acTypeName];
}

void RTTIMapper::SanitizeName(std::string& aName)
{
    std::ranges::replace(aName, '.', '_');
}

RED4ext::CName RTTIMapper::TryResolveTypeName(sol::object aValue)
{
    if (IsLuaCData(aValue))
        return "Uint64";

    switch (aValue.get_type())
    {
    case sol::type::string: return "String";

    case sol::type::boolean: return "Bool";

    case sol::type::userdata:
    {
        sol::userdata userdata = aValue;
        const sol::table metatable = userdata[sol::metatable_key];
        const auto name = metatable.raw_get_or<std::string>("__name", "");

        if (!name.empty())
            return name.c_str() + 4;

        break;
    }

    case sol::type::table:
    {
        sol::table table = aValue;

        if (!table.empty())
        {
            const RED4ext::CName innerType = TryResolveTypeName(table[1]);

            if (!innerType.IsNone())
                return std::string("array:").append(innerType.ToString()).c_str();
        }

        break;
    }
    }

    return {};
}
