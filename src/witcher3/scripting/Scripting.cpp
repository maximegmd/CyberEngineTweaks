#include <stdafx.h>

#include "scripting/Scripting.h"
#include "scripting/Texture.h"

#include "EngineTweaks.h"
#include <lsqlite3/lsqlite3.h>
#include <reverse/Type.h>
#include <reverse/RTTIHelper.h>
#include <sol_imgui/sol_imgui.h>
#include <Utils.h>

static constexpr bool s_cThrowLuaErrors = true;

void Scripting::PostInitializeMods()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();
    auto& globals = m_sandbox.GetGlobals();

    globals["GetMod"] = [this](const std::string& acName) -> sol::object
    {
        return GetMod(acName);
    };

    globals["GameDump"] = [this](const Type* acpType)
    {
        return acpType ? acpType->GameDump() : "Null";
    };

    globals["Dump"] = [this](const Type* acpType, bool aDetailed)
    {
        return acpType != nullptr ? acpType->Dump(aDetailed) : Type::Descriptor{};
    };

    globals["Game"] = this;

    RTTIHelper::PostInitialize();
    m_mapper.Register();

    RegisterOverrides();

    m_sandbox.PostInitializeMods();

    TriggerOnInit();
    if (EngineTweaks::Get().GetOverlay().IsEnabled())
        TriggerOnOverlayOpen();
}

void Scripting::PostInitializeScripting()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();
    auto& globals = m_sandbox.GetGlobals();

    if (luaVm["__Game"] != sol::nil)
    {
        m_mapper.Refresh();
        //m_override.Refresh();
        return;
    }

    luaVm.new_usertype<Scripting>("__Game", sol::meta_function::construct, sol::no_constructor, sol::meta_function::index, &Scripting::Index);

    luaVm.new_usertype<Type>("__Type", sol::meta_function::construct, sol::no_constructor, sol::meta_function::index, &Type::Index, sol::meta_function::new_index, &Type::NewIndex);

    luaVm.new_usertype<ClassType>(
        "__ClassType", sol::meta_function::construct, sol::no_constructor, sol::base_classes, sol::bases<Type>(), sol::meta_function::index, &ClassType::Index,
        sol::meta_function::new_index, &ClassType::NewIndex);

    globals.new_usertype<Type::Descriptor>("Descriptor", sol::meta_function::to_string, &Type::Descriptor::ToString);

    luaVm.new_usertype<Vector3>(
        "Vector3", sol::constructors<Vector3(float, float, float), Vector3(float, float), Vector3(float), Vector3(const Vector3&), Vector3()>(), sol::meta_function::to_string,
        &Vector3::ToString, sol::meta_function::equal_to, &Vector3::operator==, "x", &Vector3::x, "y", &Vector3::y, "z", &Vector3::z);
    globals["Vector3"] = luaVm["Vector3"];

    globals["ToVector3"] = [](sol::table table) -> Vector3
    {
        return Vector3{table["x"].get_or(0.f), table["y"].get_or(0.f), table["z"].get_or(0.f)};
    };

    luaVm.new_usertype<Vector4>(
        "Vector4",
        sol::constructors<Vector4(float, float, float, float), Vector4(float, float, float), Vector4(float, float), Vector4(float), Vector4(const Vector4&), Vector4()>(),
        sol::meta_function::to_string, &Vector4::ToString, sol::meta_function::equal_to, &Vector4::operator==, "x", &Vector4::x, "y", &Vector4::y, "z", &Vector4::z, "w",
        &Vector4::w);
    globals["Vector4"] = luaVm["Vector4"];

    globals["ToVector4"] = [](sol::table table) -> Vector4
    {
        return Vector4{table["x"].get_or(0.f), table["y"].get_or(0.f), table["z"].get_or(0.f), table["w"].get_or(0.f)};
    };

    luaVm.new_usertype<EulerAngles>(
        "EulerAngles", sol::constructors<EulerAngles(float, float, float), EulerAngles(float, float), EulerAngles(float), EulerAngles(const EulerAngles&), EulerAngles()>(),
        sol::meta_function::to_string, &EulerAngles::ToString, sol::meta_function::equal_to, &EulerAngles::operator==, "roll", &EulerAngles::roll, "pitch", &EulerAngles::pitch,
        "yaw", &EulerAngles::yaw);
    globals["EulerAngles"] = luaVm["EulerAngles"];

    globals["ToEulerAngles"] = [](sol::table table) -> EulerAngles
    {
        return EulerAngles{table["roll"].get_or(0.f), table["pitch"].get_or(0.f), table["yaw"].get_or(0.f)};
    };

    luaVm.new_usertype<Quaternion>(
        "Quaternion",
        sol::constructors<
            Quaternion(float, float, float, float), Quaternion(float, float, float), Quaternion(float, float), Quaternion(float), Quaternion(const Quaternion&), Quaternion()>(),
        sol::meta_function::to_string, &Quaternion::ToString, sol::meta_function::equal_to, &Quaternion::operator==, "i", &Quaternion::i, "j", &Quaternion::j, "k", &Quaternion::k,
        "r", &Quaternion::r);
    globals["Quaternion"] = luaVm["Quaternion"];

    globals["ToQuaternion"] = [](sol::table table) -> Quaternion
    {
        return Quaternion{table["i"].get_or(0.f), table["j"].get_or(0.f), table["k"].get_or(0.f), table["r"].get_or(0.f)};
    };

    m_sandbox.PostInitializeScripting();

    RTTIHelper::Initialize(m_lua.AsRef(), m_sandbox);

    TriggerOnHook();
}

void Scripting::RegisterOverrides()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();
}

sol::object Scripting::GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv)
{
    return sol::nil;
}

size_t Scripting::Size(CBaseRTTIType* apRttiType)
{
    return 0;
}

sol::object Scripting::ToLua(LockedState& aState, CStackType& aResult)
{
    return sol::nil;
}

CStackType Scripting::ToRED(sol::object aObject, CBaseRTTIType* apRttiType, TiltedPhoques::Allocator* apAllocator)
{
    CStackType result;

    const bool hasData = aObject != sol::nil;

    if (apRttiType)
    {
        result.type = apRttiType;
    }

    return result;
}

void Scripting::ToRED(sol::object aObject, CStackType& aRet)
{
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);

    const auto result = ToRED(aObject, aRet.type, &s_scratchMemory);

    aRet.type->Copy(aRet.value, result.value);

    //if (aRet.type->GetType() != RED4ext::ERTTIType::Class)
    //    aRet.type->Destruct(result.value);

    s_scratchMemory.Reset();
}
