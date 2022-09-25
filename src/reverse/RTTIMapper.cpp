#include <stdafx.h>

#include "RTTIMapper.h"
#include "RTTIHelper.h"
#include "RTTILocator.h"
#include "BasicTypes.h"
#include "Type.h"
#include "Enum.h"
#include "EnumStatic.h"
#include "ClassStatic.h"
#include "scripting/Scripting.h"
#include "Utils.h"

RTTIMapper::RTTIMapper(const LockableState& acLua, const std::string& acGlobal)
    : m_lua(acLua)
    , m_global(acGlobal)
{
}

RTTIMapper::~RTTIMapper()
{
    RTTIHelper::Shutdown();
}

void RTTIMapper::Register()
{
    RTTIHelper::Initialize(m_lua);

    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    auto luaGlobal = luaState.get<sol::table>(m_global);
    auto* pRtti = RED4ext::CRTTISystem::Get();

    RegisterSimpleTypes(luaState, luaGlobal);
    RegisterDirectTypes(luaState, luaGlobal, pRtti);
    RegisterDirectGlobals(luaGlobal, pRtti);
    RegisterScriptAliases(luaGlobal, pRtti);
    RegisterSpecialAccessors(luaState, luaGlobal);
}

void RTTIMapper::Refresh()
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    auto luaGlobal = luaState.get<sol::table>(m_global);
    auto* pRtti = RED4ext::CRTTISystem::Get();

    RegisterDirectTypes(luaState, luaGlobal, pRtti);
    RegisterDirectGlobals(luaGlobal, pRtti);
}

void RTTIMapper::RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal)
{
    aLuaState.new_usertype<Variant>("Variant",
        sol::meta_function::construct, sol::no_constructor);

    MakeSolUsertypeImmutable(aLuaState["Variant"], aLuaState);

    aLuaGlobal["ToVariant"] = sol::overload(
        [this](Type& aInstance, sol::this_state aState) -> sol::object {
            auto* pType = aInstance.GetType();
            auto* pValue = aInstance.GetValuePtr();

            if (!pType || !pValue)
            {
                luaL_error(aState, "ToVariant: Invalid instance.");
                return sol::nil;
            }

            auto luaLock = m_lua.Lock();
            auto& luaState = luaLock.Get();

            return sol::object(luaState, sol::in_place, Variant(pType, pValue));
        },
        [this](Enum& aEnum) -> sol::object {
            RED4ext::CStackType data;
            data.type = const_cast<RED4ext::CEnum*>(aEnum.GetType());
            data.value = const_cast<void*>(aEnum.GetValuePtr());

            auto luaLock = m_lua.Lock();
            auto& luaState = luaLock.Get();

            return sol::object(luaState, sol::in_place, Variant(data));
        },
        [this](sol::object aValue, const std::string& aTypeName, sol::this_state aState) -> sol::object {
            auto* pType = RED4ext::CRTTISystem::Get()->GetType(aTypeName.c_str());

            if (!pType)
            {
                luaL_error(aState, "ToVariant: Unknown type '%s'.", aTypeName.c_str());
                return sol::nil;
            }

            Variant variant(pType);
            RED4ext::CStackType data(pType, variant.GetDataPtr());

            Scripting::ToRED(aValue, data);

            auto luaLock = m_lua.Lock();
            auto& luaState = luaLock.Get();

            return sol::object(luaState, sol::in_place, std::move(variant));
        },
        [this](sol::object aValue, sol::this_state aState) -> sol::object {
            RED4ext::CName typeName = TryResolveTypeName(aValue);

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

            auto luaLock = m_lua.Lock();
            auto& luaState = luaLock.Get();

            return sol::object(luaState, sol::in_place, std::move(variant));
        }
    );

    aLuaGlobal["FromVariant"] = [this](Variant& aVariant) -> sol::object {
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
    bool reinit = aLuaState["EnumStatic"] != sol::nil;

    if (!reinit)
    {
        aLuaState.new_usertype<EnumStatic>("EnumStatic",
            sol::meta_function::construct, sol::no_constructor,
            sol::base_classes, sol::bases<Type>(),
            sol::meta_function::index, &EnumStatic::Index,
            sol::meta_function::new_index, &EnumStatic::NewIndex);

        aLuaState.new_usertype<ClassStatic>("ClassStatic",
            sol::meta_function::construct, sol::no_constructor,
            sol::base_classes, sol::bases<Type>(),
            sol::meta_function::index, &ClassStatic::Index,
            sol::meta_function::new_index, &ClassStatic::NewIndex,
            "new", sol::property(&ClassStatic::GetFactory));

        MakeSolUsertypeImmutable(aLuaState["EnumStatic"], aLuaState);
        MakeSolUsertypeImmutable(aLuaState["ClassStatic"], aLuaState);
    }

    apRtti->types.for_each([&](RED4ext::CName aTypeName, RED4ext::CBaseRTTIType* apType) {
        std::string typeName = aTypeName.ToString();
        SanitizeName(typeName);

        if (reinit && aLuaGlobal[typeName] != sol::nil)
            return;

        switch (apType->GetType())
        {
        case RED4ext::ERTTIType::Enum:
        {
            auto* pEnum = reinterpret_cast<RED4ext::CEnum*>(apType);
            auto luaEnum = make_object(aLuaState, EnumStatic(m_lua, apType));

            MakeSolUsertypeImmutable(luaEnum, aLuaState);

            aLuaGlobal[typeName] = luaEnum;
            break;
        }
        case RED4ext::ERTTIType::Class:
        {
            auto* pClass = reinterpret_cast<RED4ext::CClass*>(apType);
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
    apRtti->funcs.for_each([&aLuaGlobal](RED4ext::CName aOrigName, RED4ext::CGlobalFunction* apFunc) {
        const std::string cShortName = apFunc->shortName.ToString();
        const FuncFlags cFlags = *(FuncFlags*)(&apFunc->flags);

        if (aLuaGlobal[cShortName] == sol::nil && !cFlags.isExec)
        {
            const std::string cFullName = apFunc->fullName.ToString();
            const auto cIsClassFunc = cFullName.find("::") != std::string::npos;
            const auto cIsOperatorFunc = cShortName.find(";") != std::string::npos;

            if (!cIsClassFunc && !cIsOperatorFunc)
            {
                aLuaGlobal[cShortName] = RTTIHelper::Get().ResolveFunction(cShortName);
            }
        }
        });
}

void RTTIMapper::RegisterScriptAliases(sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti)
{
    apRtti->scriptToNative.for_each([&aLuaGlobal](RED4ext::CName aScriptName, RED4ext::CName aNativeName) {
        aLuaGlobal[aScriptName.ToString()] = aLuaGlobal[aNativeName.ToString()];
        });
}

void RTTIMapper::RegisterSpecialAccessors(sol::state& aLuaState, sol::table& aLuaGlobal)
{
    // Replace RTTI version of class with our own version
    aLuaGlobal["Vector3"] = aLuaState["Vector3"];

    // Merge RTTI versions of basic types with our own versions
    // Allows usertype and RTTI functions to be used under the same name
    ExtendUsertype<Vector4>("Vector4", aLuaState, aLuaGlobal);
    ExtendUsertype<EulerAngles>("EulerAngles", aLuaState, aLuaGlobal);
    ExtendUsertype<Quaternion>("Quaternion", aLuaState, aLuaGlobal);
    ExtendUsertype<ItemID>("ItemID", aLuaState, aLuaGlobal);

    // Add global alias `Game.GetSystemRequestsHandler()`
    // Replacement for `GetSingleton("inkMenuScenario"):GetSystemRequestsHandler()`
    RTTIHelper::Get().AddFunctionAlias("GetSystemRequestsHandler", "inkMenuScenario", "GetSystemRequestsHandler");
}

template <class T>
void RTTIMapper::ExtendUsertype(const std::string acTypeName, sol::state& aLuaState, sol::table& aLuaGlobal)
{
    auto& classStatic = aLuaGlobal.get<ClassStatic>(acTypeName);
    auto classIndexer = [&classStatic](const sol::object& acSelf, const std::string& acName, sol::this_environment aEnv) {
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
    std::replace(aName.begin(), aName.end(), '.', '_');
}

RED4ext::CName RTTIMapper::TryResolveTypeName(sol::object aValue)
{
    if (IsLuaCData(aValue))
        return "Uint64";

    switch (aValue.get_type())
    {
    case sol::type::string:
        return "String";

    case sol::type::boolean:
        return "Bool";

    case sol::type::userdata:
    {
        sol::userdata userdata = aValue;
        sol::table metatable = userdata[sol::metatable_key];
        std::string name = metatable.raw_get_or<std::string>("__name", "");

        if (!name.empty())
            return name.c_str() + 4;

        break;
    }

    case sol::type::table:
    {
        sol::table table = aValue;

        if (table.size() > 0)
        {
            RED4ext::CName innerType = TryResolveTypeName(table[1]);

            if (!innerType.IsNone())
                return std::string("array:").append(innerType.ToString()).c_str();
        }

        break;
    }
    }

    return RED4ext::CName();
}
