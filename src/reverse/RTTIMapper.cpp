#include <stdafx.h>

#include "RTTIMapper.h"
#include "RTTIHelper.h"
#include "Type.h"
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

    sol::table luaGlobal = luaState.get<sol::table>(m_global);

    auto* pRtti = RED4ext::CRTTISystem::Get();

    RegisterSimpleTypes(luaState, luaGlobal);
    RegisterDirectTypes(luaState, luaGlobal, pRtti);
    RegisterScriptAliases(luaGlobal, pRtti);
    RegisterSpecialAccessors(luaState, luaGlobal);
}

void RTTIMapper::RegisterSimpleTypes(sol::state& aLuaState, sol::table& aLuaGlobal)
{
    aLuaState.new_usertype<Variant>("Variant",
        sol::meta_function::construct, sol::no_constructor);

    MakeSolUsertypeImmutable(aLuaState["Variant"], aLuaState);

    aLuaGlobal["ToVariant"] = [this](Type& aInstance) -> sol::object {
        auto* pType = aInstance.GetType();
        auto* pHandle = aInstance.GetHandle();

        if (!pType || !pHandle)
            return sol::nil;

        auto lockedState = m_lua.Lock();
        auto& luaState = lockedState.Get();

        return sol::object(luaState, sol::in_place, Variant(pType, pHandle));
    };

    aLuaGlobal["FromVariant"] = [this](const Variant& acVariant) -> sol::object {
        if (acVariant.type == 0 || acVariant.unknown != 0)
            return sol::nil;

        RED4ext::CStackType result;
        result.type = reinterpret_cast<RED4ext::IRTTIType*>(acVariant.type);
        result.value = reinterpret_cast<RED4ext::ScriptInstance>(acVariant.value);

        auto lockedState = m_lua.Lock();
        
        return Scripting::ToLua(lockedState, result);
    };
}

void RTTIMapper::RegisterDirectTypes(sol::state& aLuaState, sol::table& aLuaGlobal, RED4ext::CRTTISystem* apRtti)
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

    apRtti->types.for_each([&](RED4ext::CName aTypeName, RED4ext::IRTTIType* apType) {
        switch (apType->GetType())
        {
        case RED4ext::ERTTIType::Enum:
        {
            auto* pEnum = static_cast<RED4ext::CEnum*>(apType);
            auto luaEnum = make_object(aLuaState, EnumStatic(m_lua, apType));

            MakeSolUsertypeImmutable(luaEnum, aLuaState);

            aLuaGlobal[aTypeName.ToString()] = luaEnum;
            break;
        }
        case RED4ext::ERTTIType::Class:
        {
            auto* pClass = static_cast<RED4ext::CClass*>(apType);
            auto luaClass = make_object(aLuaState, ClassStatic(m_lua, apType));

            MakeSolUsertypeImmutable(luaClass, aLuaState);

            aLuaGlobal[aTypeName.ToString()] = luaClass;
            break;
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
