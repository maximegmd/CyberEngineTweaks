#include <stdafx.h>

#include "Scripting.h"

#include "FunctionOverride.h"
#include "GameOptions.h"

#include <sol_imgui/sol_imgui.h>
#include <lsqlite3/lsqlite3.h>

#include <d3d12/D3D12.h>

#include <reverse/Type.h>
#include <reverse/SingletonReference.h>
#include <reverse/StrongReference.h>
#include <reverse/WeakReference.h>
#include <reverse/ResourceAsyncReference.h>
#include <reverse/Enum.h>
#include <reverse/TweakDB.h>
#include <reverse/RTTILocator.h>
#include <reverse/RTTIHelper.h>
#include <reverse/RTTIExtender.h>
#include <reverse/Converter.h>

#include "Utils.h"

#ifndef NDEBUG
#include "GameHooks.h"
#include "GameDump.h"
#include <RED4ext/Dump/Reflection.hpp>
#endif

static RTTILocator s_stringType{RED4ext::FNV1a("String")};

Scripting::Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions)
    : m_sandbox(this, aBindings)
    , m_store(m_sandbox, aPaths, aBindings)
    , m_override(this, aOptions)
    , m_paths(aPaths)
    , m_d3d12(aD3D12)
    , m_mapper(m_lua.AsRef(), m_global)
{
    CreateLogger(aPaths.CETRoot() / "scripting.log", "scripting");
}

void Scripting::Initialize()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();

    luaVm.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package,
                         sol::lib::os, sol::lib::table, sol::lib::bit32);
    luaVm.require("sqlite3", luaopen_lsqlite3);

    sol_ImGui::InitBindings(luaVm);

    luaVm["print"] = [](sol::variadic_args aArgs, sol::this_state aState)
    {
        std::ostringstream oss;
        sol::state_view s(aState);
        for (auto it = aArgs.cbegin(); it != aArgs.cend(); ++it)
        {
            if (it != aArgs.cbegin())
            {
                oss << " ";
            }
            std::string str = s["tostring"]((*it).get<sol::object>());
            oss << str;
        }

        spdlog::get("scripting")->info(oss.str());
    };

    // global fallback table for all environments
    sol::table luaGlobal = sol::table(luaVm, sol::create);
    luaVm[m_global] = luaGlobal;

    luaGlobal["GetVersion"] = []() -> std::string
    {
        return CET_BUILD_COMMIT;
    };

    luaGlobal["GetDisplayResolution"] = [this]() -> std::tuple<float, float>
    {
        const auto resolution = m_d3d12.GetResolution();
        return {static_cast<float>(resolution.cx), static_cast<float>(resolution.cy)};
    };

    // fake game object to prevent errors from older autoexec.lua
    luaVm["Game"] = sol::table(luaVm, sol::create);
    luaGlobal["Game"] = luaVm["Game"];

    // execute autoexec.lua inside our default script directory
    current_path(m_paths.CETRoot() / "scripts");
    if (std::filesystem::exists("autoexec.lua"))
        luaVm.do_file("autoexec.lua");
    else
        spdlog::get("scripting")->warn("WARNING: missing CET autoexec.lua!");

    // initialize sandbox
    m_sandbox.Initialize();

    // setup logger for console sandbox
    auto& consoleSB = m_sandbox[0];
    auto& consoleSBEnv = consoleSB.GetEnvironment();
    consoleSBEnv["__logger"] = spdlog::get("scripting");

    // set current path for following scripts to our ModsPath
    current_path(m_paths.ModsRoot());

    // load mods
    m_store.LoadAll();
}

void Scripting::PostInitialize()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();

    sol::table luaGlobal = luaVm[m_global];

    luaVm.new_usertype<Scripting>("__Game",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Scripting::Index,
        sol::meta_function::new_index, &Scripting::NewIndex);

    luaVm.new_usertype<Type>("__Type",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Type::Index,
        sol::meta_function::new_index, &Type::NewIndex);

    luaVm.new_usertype<Type::Descriptor>("Descriptor",
        sol::meta_function::to_string, &Type::Descriptor::ToString);

    luaVm.new_usertype<StrongReference>("StrongReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &StrongReference::Index,
        sol::meta_function::new_index, &StrongReference::NewIndex);

    luaVm.new_usertype<WeakReference>("WeakReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &WeakReference::Index,
        sol::meta_function::new_index, &WeakReference::NewIndex);

    luaVm.new_usertype<SingletonReference>("SingletonReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &SingletonReference::Index,
        sol::meta_function::new_index, &SingletonReference::NewIndex);

    luaVm.new_usertype<ClassReference>("ClassReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &ClassReference::Index,
        sol::meta_function::new_index, &ClassReference::NewIndex);

    luaVm.new_usertype<ResourceAsyncReference>("ResourceAsyncReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &ResourceAsyncReference::Index,
        sol::meta_function::new_index, &ResourceAsyncReference::NewIndex,
        "hash", sol::property(&ResourceAsyncReference::GetLuaHash));

    luaVm.new_usertype<UnknownType>("Unknown",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &UnknownType::Index,
        sol::meta_function::new_index, &UnknownType::NewIndex);

    luaVm.new_usertype<Enum>("Enum",
        sol::constructors<Enum(const std::string&, const std::string&),
                          Enum(const std::string&, uint32_t), Enum(const Enum&)>(),
        sol::meta_function::to_string, &Enum::ToString,
        sol::meta_function::equal_to, &Enum::operator==,
        "value", sol::property(&Enum::GetValueName, &Enum::SetValueByName));
    luaGlobal["Enum"] = luaVm["Enum"];

    luaVm.new_usertype<Vector3>("Vector3",
        sol::constructors<Vector3(float, float, float), Vector3(float, float), Vector3(float), Vector3(sol::table), 
                          Vector3(const Vector3&), Vector3()>(),
        sol::meta_function::to_string, &Vector3::ToString,
        sol::meta_function::equal_to, &Vector3::operator==,
        "x", &Vector3::X,
        "y", &Vector3::Y,
        "z", &Vector3::Z);
    luaGlobal["Vector3"] = luaVm["Vector3"];
    luaGlobal["ToVector3"] = [](sol::table aTable) { return Vector3(aTable); };

    luaVm.new_usertype<Vector4>("Vector4",
        sol::constructors<Vector4(float, float, float, float), Vector4(float, float, float), Vector4(float, float),
                          Vector4(float), Vector4(sol::table), Vector4(const Vector4&), Vector4()>(),
        sol::meta_function::to_string, &Vector4::ToString,
        sol::meta_function::equal_to, &Vector4::operator==,
        "x", &Vector4::X,
        "y", &Vector4::Y,
        "z", &Vector4::Z,
        "w", &Vector4::W);
    luaGlobal["Vector4"] = luaVm["Vector4"];
    luaGlobal["ToVector4"] = [](sol::table aTable) { return Vector4(aTable); };

    luaVm.new_usertype<EulerAngles>("EulerAngles",
        sol::constructors<EulerAngles(float, float, float), EulerAngles(float, float), EulerAngles(float),
                          EulerAngles(sol::table), EulerAngles(const EulerAngles&), EulerAngles()>(),
        sol::meta_function::to_string, &EulerAngles::ToString,
        sol::meta_function::equal_to, &EulerAngles::operator==,
        "roll", &EulerAngles::Roll,
        "pitch", &EulerAngles::Pitch,
        "yaw", &EulerAngles::Yaw);
    luaGlobal["EulerAngles"] = luaVm["EulerAngles"];
    luaGlobal["ToEulerAngles"] = [](sol::table aTable) { return EulerAngles(aTable); };

    luaVm.new_usertype<Quaternion>("Quaternion",
        sol::constructors<Quaternion(float, float, float, float), Quaternion(float, float, float),
                          Quaternion(float, float), Quaternion(float), Quaternion(sol::table),
                          Quaternion(const Quaternion&), Quaternion()>(),
        sol::meta_function::to_string, &Quaternion::ToString,
        sol::meta_function::equal_to, &Quaternion::operator==,
        "i", &Quaternion::i,
        "j", &Quaternion::j,
        "k", &Quaternion::k,
        "r", &Quaternion::r);
    luaGlobal["Quaternion"] = luaVm["Quaternion"];
    luaGlobal["ToQuaternion"] = [](sol::table aTable) { return Quaternion(aTable); };

    luaVm.new_usertype<CName>("CName",
        sol::constructors<CName(const std::string&), CName(uint32_t, uint32_t), CName(sol::table),
                          CName(const CName&), CName()>(),
        sol::meta_function::to_string, &CName::ToString,
        sol::meta_function::equal_to, &CName::operator==,
        "hash_lo", sol::property(&CName::GetHashLo, &CName::SetHashLo),
        "hash_hi", sol::property(&CName::GetHashHi, &CName::SetHashHi),
        "value", sol::property(&CName::ToString));
    luaGlobal["CName"] = luaVm["CName"];
    luaGlobal["ToCName"] = [](sol::table aTable) { return CName(aTable); };

    luaVm.new_usertype<TweakDBID>("TweakDBID",
        sol::constructors<TweakDBID(const std::string&), TweakDBID(const TweakDBID&, const std::string&),
                          TweakDBID(uint32_t, uint8_t), TweakDBID(sol::table), TweakDBID(const TweakDBID&),
                          TweakDBID()>(),
        sol::meta_function::to_string, &TweakDBID::ToString,
        sol::meta_function::equal_to, &TweakDBID::operator==,
        sol::meta_function::addition, &TweakDBID::operator+,
        sol::meta_function::concatenation, &TweakDBID::operator+,
        "hash", &TweakDBID::nameHash,
        "length", &TweakDBID::nameLength);
    luaGlobal["TweakDBID"] = luaVm["TweakDBID"];
    luaGlobal["ToTweakDBID"] = [](sol::table aTable) { return TweakDBID(aTable); };

    luaVm.new_usertype<ItemID>("ItemID",
        sol::constructors<ItemID(const TweakDBID&, uint32_t, uint16_t, uint8_t),
                          ItemID(const TweakDBID&, uint32_t, uint16_t), ItemID(const TweakDBID&, uint32_t),
                          ItemID(const TweakDBID&), ItemID(sol::table), ItemID(const ItemID&), ItemID()>(),
        sol::meta_function::to_string, &ItemID::ToString,
        sol::meta_function::equal_to, &ItemID::operator==,
        "id", sol::property(&ItemID::GetTweakDBID, &ItemID::SetTweakDBID),
        "tdbid", sol::property(&ItemID::GetTweakDBID, &ItemID::SetTweakDBID),
        "rng_seed", &ItemID::rngSeed,
        "unknown", &ItemID::unk0C,
        "maybe_type", &ItemID::unk0E);
    luaGlobal["ItemID"] = luaVm["ItemID"];
    luaGlobal["ToItemID"] = [](sol::table aTable) { return ItemID(aTable); };

    luaGlobal["NewObject"] = [this](const std::string& acName, sol::this_environment aEnv) -> sol::object
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();
        auto* pType = pRtti->GetType(RED4ext::FNV1a(acName.c_str()));

        if (!pType)
        {
            const sol::environment cEnv = aEnv;
            std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->info("Type '{}' not found.", acName);
            return sol::nil;
        }

        // Always try to return instance wrapped in Handle<> if the type allows it.
        // See NewHandle() for more info.
        return RTTIHelper::Get().NewHandle(pType, sol::nullopt);
    };

    luaGlobal["GetSingleton"] = [this](const std::string& acName, sol::this_environment aThisEnv)
    {
        return this->GetSingletonHandle(acName, aThisEnv);
    };

    luaVm.new_usertype<GameOptions>("GameOptions",
        sol::meta_function::construct, sol::no_constructor,
        "Print", &GameOptions::Print,
        "Get", &GameOptions::Get,
        "GetBool", &GameOptions::GetBool,
        "GetInt", &GameOptions::GetInt,
        "GetFloat", &GameOptions::GetFloat,
        "Set", &GameOptions::Set,
        "SetBool", &GameOptions::SetBool,
        "SetInt", &GameOptions::SetInt,
        "SetFloat", &GameOptions::SetFloat,
        "Toggle", &GameOptions::Toggle,
        "Dump", &GameOptions::Dump,
        "List", &GameOptions::List);

    luaGlobal["GameOptions"] = luaVm["GameOptions"];

    luaVm.new_usertype<TweakDB>("__TweakDB",
        sol::meta_function::construct, sol::no_constructor,
        "DebugStats", &TweakDB::DebugStats,
        "GetRecords", &TweakDB::GetRecords,
        "GetRecord", sol::overload(&TweakDB::GetRecordByName, &TweakDB::GetRecord),
        "Query", sol::overload(&TweakDB::QueryByName, &TweakDB::Query),
        "GetFlat", sol::overload(&TweakDB::GetFlatByName, &TweakDB::GetFlat),
        "SetFlats", sol::overload(&TweakDB::SetFlatsByName, &TweakDB::SetFlats),
        "SetFlat", sol::overload(&TweakDB::SetFlatByNameAutoUpdate, &TweakDB::SetFlatAutoUpdate),
        "SetFlatNoUpdate", sol::overload(&TweakDB::SetFlatByName, &TweakDB::SetFlat),
        "Update", sol::overload(&TweakDB::UpdateRecordByName, &TweakDB::UpdateRecordByID, &TweakDB::UpdateRecord),
        "CreateRecord", &TweakDB::CreateRecord,
        "CloneRecord", sol::overload(&TweakDB::CloneRecordByName, &TweakDB::CloneRecord),
        "DeleteRecord", &TweakDB::DeleteRecord);

    luaGlobal["TweakDB"] = TweakDB(m_lua.AsRef());

    luaGlobal["Override"] = sol::overload(
        [this](const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction,
               sol::this_environment aThisEnv) {
            m_override.Override(acTypeName, acFullName, acFullName, true, aFunction, aThisEnv);
        },
        [this](const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
               sol::protected_function aFunction, sol::this_environment aThisEnv) {
            m_override.Override(acTypeName, acFullName, acShortName, true, aFunction, aThisEnv);
        });

    luaGlobal["Observe"] = sol::overload(
        [this](const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction,
               sol::this_environment aThisEnv) {
            m_override.Override(acTypeName, acFullName, acFullName, false, aFunction, aThisEnv);
        },
        [this](const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
               sol::protected_function aFunction, sol::this_environment aThisEnv) {
            m_override.Override(acTypeName, acFullName, acShortName, false, aFunction, aThisEnv);
        });

    luaGlobal["GameDump"] = [this](Type* apType)
    {
        return apType ? apType->GameDump() : "Null";
    };

    luaGlobal["Dump"] = [this](Type* apType, bool aDetailed)
    {
        return apType != nullptr ? apType->Dump(aDetailed) : Type::Descriptor{};
    };

    luaGlobal["DumpType"] = [this](const std::string& acName, bool aDetailed)
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();
        auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
        if (!pType || pType->GetType() == RED4ext::ERTTIType::Simple)
            return Type::Descriptor();

        const ClassType type(m_lua.AsRef(), pType);
        return type.Dump(aDetailed);
    };
    
    luaGlobal["DumpAllTypeNames"] = [this](sol::this_environment aThisEnv)
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();

        uint32_t count = 0;
        pRtti->types.for_each([&count](RED4ext::CName name, RED4ext::IRTTIType*& type)
        {
            spdlog::info(name.ToString());
            count++;
        });
        const sol::environment cEnv = aThisEnv;
        std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->info("Dumped {} types", count);
    };

#ifndef NDEBUG
    luaGlobal["DumpVtables"] = [this]()
    {
        // Hacky RTTI dump, this should technically only dump IScriptable instances and RTTI types as they are guaranteed to have a vtable
        // but there can occasionally be Class types that are not IScriptable derived that still have a vtable
        // some hierarchies may also not be accurately reflected due to hash ordering
        // technically this table is flattened and contains all hierarchy, but traversing the hierarchy first reduces
        // error when there are classes that instantiate a parent class but don't actually have a subclass instance
        GameMainThread::Get().AddTask(&GameDump::DumpVTablesTask::Run);
    };
    luaGlobal["DumpReflection"] = [this](bool aVerbose, bool aExtendedPath, bool aPropertyHolders)
    {
        RED4ext::GameReflection::Dump(m_paths.CETRoot() / "dumps", aVerbose, aExtendedPath, aPropertyHolders);
    };
#endif

    // Doesn't require RTTI, but still shouldn't be used before onInit as there is no guarantee that the required mod will be loaded before
    luaGlobal["GetMod"] = [this](const std::string& acName) -> sol::object
    {
        return GetMod(acName);
    };

    luaVm["Game"] = this;
    luaGlobal["Game"] = luaVm["Game"];

    RTTIExtender::Initialize();
    m_mapper.Register();
    m_sandbox.PostInitialize();

    RegisterOverrides();
}

void Scripting::RegisterOverrides()
{
    m_override.Override("QuestTrackerGameController", "OnUninitialize", "OnUninitialize", false, sol::nil, sol::nil, true);
}

const TiltedPhoques::Vector<VKBindInfo>& Scripting::GetBinds() const
{
    return m_store.GetBinds();
}

void Scripting::TriggerOnInit() const
{
    m_store.TriggerOnInit();
}

void Scripting::TriggerOnUpdate(float aDeltaTime) const
{
    m_store.TriggerOnUpdate(aDeltaTime);
}

void Scripting::TriggerOnDraw() const
{
    m_store.TriggerOnDraw();
}

void Scripting::TriggerOnOverlayOpen() const
{
    m_store.TriggerOnOverlayOpen();
}

void Scripting::TriggerOnOverlayClose() const
{
    m_store.TriggerOnOverlayClose();
}

sol::object Scripting::GetMod(const std::string& acName) const
{
    return m_store.GetMod(acName);
}

void Scripting::ReloadAllMods()
{
    m_override.Clear();
    RegisterOverrides();
    m_store.LoadAll();
}

bool Scripting::ExecuteLua(const std::string& acCommand)
{
    // TODO: proper exception handling!
    try
    {
        auto lock = std::scoped_lock{m_vmLock};

        const auto cResult = m_sandbox.ExecuteString(acCommand);
        if (cResult.valid())
            return true;
        const sol::error cError = cResult;
        spdlog::get("scripting")->error(cError.what());
    }
    catch(std::exception& e)
    {
        spdlog::get("scripting")->error(e.what());
    }
    return false;
}

void Scripting::CollectGarbage()
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    luaState.collect_garbage();
}

Scripting::LockedState Scripting::GetState() const noexcept
{
    return m_lua.Lock();
}

std::string Scripting::GetGlobalName() const noexcept
{
    return m_global;
}

sol::object Scripting::Index(const std::string& acName, sol::this_environment aThisEnv)
{
    if (const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return InternalIndex(acName, aThisEnv);
}

sol::object Scripting::NewIndex(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = std::move(aParam);
    return property;
}

sol::protected_function Scripting::InternalIndex(const std::string& acName, sol::this_environment aThisEnv)
{
    auto func = RTTIHelper::Get().ResolveFunction(acName);

    if (!func)
    {
        const sol::environment cEnv = aThisEnv;
        std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->error("Error: Function {} not found or is not a global.", acName);

        return sol::nil;
    }

    return NewIndex(acName, std::move(func));
}

sol::object Scripting::GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv)
{
    auto locked = m_lua.Lock();
    auto& lua = locked.Get();

    auto itor = m_singletons.find(acName);
    if (itor != std::end(m_singletons))
        return make_object(lua, itor->second);

    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
    if (!pType)
    {
        const sol::environment cEnv = aThisEnv;
        std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->info("Type '{}' not found.", acName);
        return sol::nil;
    }

    auto result = m_singletons.emplace(std::make_pair(acName, SingletonReference{m_lua.AsRef(), pType}));
    return make_object(lua, result.first->second);
}

size_t Scripting::Size(RED4ext::IRTTIType* apRttiType)
{
    if (apRttiType == s_stringType)
        return sizeof(RED4ext::CString);
    if (apRttiType->GetType() == RED4ext::ERTTIType::Handle)
        return sizeof(RED4ext::Handle<RED4ext::IScriptable>);
    if (apRttiType->GetType() == RED4ext::ERTTIType::WeakHandle)
        return sizeof(RED4ext::WeakHandle<RED4ext::IScriptable>);

    return Converter::Size(apRttiType);
}

sol::object Scripting::ToLua(LockedState& aState, RED4ext::CStackType& aResult)
{
    auto* pType = aResult.type;

    auto& state = aState.Get();

    if (pType == nullptr)
        return sol::nil;
    if (pType == s_stringType)
        return make_object(state, std::string(static_cast<RED4ext::CString*>(aResult.value)->c_str()));
    if (pType->GetType() == RED4ext::ERTTIType::Handle)
    {
        const auto handle = *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(aResult.value);
        if (handle)
            return make_object(state, StrongReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::WeakHandle)
    {
        const auto handle = *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(aResult.value);
        if (!handle.Expired())
            return make_object(state, WeakReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::Array)
    {
        auto* pArrayType = static_cast<RED4ext::CArray*>(pType);
        const uint32_t cLength = pArrayType->GetLength(aResult.value);
        sol::table result(state, sol::create);
        for (auto i = 0u; i < cLength; ++i)
        {
            RED4ext::CStackType el;
            el.value = pArrayType->GetElement(aResult.value, i);
            el.type = pArrayType->GetInnerType();

            result[i + 1] = ToLua(aState, el);
        }

        return result;
    }
    else if (pType->GetType() == RED4ext::ERTTIType::ResourceAsyncReference)
    {
        auto* pInstance = static_cast<RED4ext::ResourceAsyncReference<void>*>(aResult.value);
        if (pInstance)
            return make_object(state, ResourceAsyncReference(aState, aResult.type, pInstance));
    }
    else
    {
        auto result = Converter::ToLua(aResult, aState);
        if (result != sol::nil)
            return result;
    }

    return sol::nil;
}

RED4ext::CStackType Scripting::ToRED(sol::object aObject, RED4ext::IRTTIType* apRttiType,
                                     TiltedPhoques::Allocator* apAllocator)
{
    RED4ext::CStackType result;

    const bool hasData = aObject != sol::nil;

    if (apRttiType)
    {
        result.type = apRttiType;

        if (apRttiType == s_stringType)
        {
            std::string str;
            if (hasData)
            {
                sol::state_view v(aObject.lua_state());
                str = v["tostring"](aObject);
            }
            result.value = apAllocator->New<RED4ext::CString>(str.c_str());
        }
        else if (apRttiType->GetType() == RED4ext::ERTTIType::Handle)
        {
            if (aObject.is<StrongReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRttiType)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(
                            aObject.as<StrongReference>().m_strongHandle);
                    else
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>();
                }
            }
            else if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRttiType)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(
                            aObject.as<WeakReference>().m_weakHandle);
                    else
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>();
                }
            }
            else if (aObject.is<ClassReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRttiType)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<ClassReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(
                        static_cast<RED4ext::IScriptable*>(aObject.as<ClassReference>().GetHandle()));
                }
            }
        }
        else if (apRttiType->GetType() == RED4ext::ERTTIType::WeakHandle)
        {
            if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRttiType)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>(
                            aObject.as<WeakReference>().m_weakHandle);
                    else
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>();
                }
            }
            else if (aObject.is<StrongReference>()) // Handle Implicit Cast
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRttiType)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>(
                            aObject.as<StrongReference>().m_strongHandle);
                    else
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>();
                }
            }
        }
        else if (apRttiType->GetType() == RED4ext::ERTTIType::Array)
        {
            if (!hasData || aObject.get_type() == sol::type::table)
            {
                auto* pArrayType = static_cast<RED4ext::CArray*>(apRttiType);
                auto pMemory = static_cast<RED4ext::DynArray<void*>*>(apAllocator->Allocate(apRttiType->GetSize()));
                apRttiType->Init(pMemory);

                if (hasData)
                {
                    auto tbl = aObject.as<sol::table>();

                    if (tbl.size() > 0)
                    {
                        auto* pArrayInnerType = pArrayType->GetInnerType();
                        const auto shouldDestroy = pArrayInnerType->GetType() != RED4ext::ERTTIType::Class;

                        // Copy elements from the table into the array
                        pArrayType->Resize(pMemory, tbl.size());
                        for (uint32_t i = 1; i <= tbl.size(); ++i)
                        {
                            RED4ext::CStackType type = ToRED(tbl.get<sol::object>(i), pArrayInnerType, apAllocator);

                            // Break on first incompatible element
                            if (!type.value)
                            {
                                pArrayType->Destroy(pMemory);
                                pMemory = nullptr;
                                break;
                            }

                            const auto pElement = pArrayType->GetElement(pMemory, i - 1);
                            pArrayInnerType->Assign(pElement, type.value);

                            if (shouldDestroy)
                                pArrayInnerType->Destroy(type.value);
                        }
                    }
                }

                result.value = pMemory;
            }
        }
        else if (apRttiType->GetType() == RED4ext::ERTTIType::ScriptReference)
        {
            if (aObject.is<ClassReference>())
            {
                auto* pClassRef = aObject.as<ClassReference*>();
                auto* pScriptRef = apAllocator->New<RED4ext::ScriptRef<void>>();
                pScriptRef->innerType = pClassRef->m_pType;
                pScriptRef->ref = pClassRef->GetHandle();
                apRttiType->GetName(pScriptRef->hash);
                result.value = pScriptRef;
            }
        }
        else if (apRttiType->GetType() == RED4ext::ERTTIType::ResourceAsyncReference)
        {
            if (hasData)
            {
                uint64_t hash = 0;

                if (aObject.is<ResourceAsyncReference>())
                {
                    hash = aObject.as<ResourceAsyncReference*>()->GetHash();
                }
                else if (aObject.get_type() == sol::type::string)
                {
                    hash = RED4ext::FNV1a(aObject.as<std::string>().c_str());
                }
                else if (aObject.is<CName>())
                {
                    hash = aObject.as<CName*>()->hash;
                }
                else if (aObject.get_type() == sol::type::number)
                {
                    hash = aObject.as<uint64_t>();
                }
                else if (IsLuaCData(aObject))
                {
                    sol::state_view v(aObject.lua_state());
                    std::string str = v["tostring"](aObject);
                    hash = std::stoull(str);
                }

                if (hash != 0)
                {
                    auto* pRaRef = apAllocator->New<RED4ext::ResourceAsyncReference<void>>();
                    pRaRef->ref = reinterpret_cast<void*>(hash);

                    result.value = pRaRef;
                }
            }
        }
        else
        {
            return Converter::ToRED(aObject, apRttiType, apAllocator);
        }
    }

    return result;
}

void Scripting::ToRED(sol::object aObject, RED4ext::CStackType* apType)
{
    const bool hasData = aObject != sol::nil;

    if (apType->type)
    {
        if (apType->type == s_stringType)
        {
            std::string str;
            if (hasData)
            {
                sol::state_view v(aObject.lua_state());
                str = v["tostring"](aObject);
            }
            RED4ext::CString value(str.c_str());
            *static_cast<RED4ext::CString*>(apType->value) = value;
        }
        else if (apType->type->GetType() == RED4ext::ERTTIType::Handle)
        {
            if (aObject.is<StrongReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apType->type)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::Handle<RED4ext::IScriptable>(aObject.as<StrongReference>().m_strongHandle);
                    else
                        *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::Handle<RED4ext::IScriptable>();
                }
            }
            else if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apType->type)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::Handle<RED4ext::IScriptable>(aObject.as<WeakReference>().m_weakHandle);
                    else
                        *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::Handle<RED4ext::IScriptable>();
                }
            }
        }
        else if (apType->type->GetType() == RED4ext::ERTTIType::WeakHandle)
        {
            if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apType->type)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::WeakHandle<RED4ext::IScriptable>(aObject.as<WeakReference>().m_weakHandle);
                    else
                        *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::WeakHandle<RED4ext::IScriptable>();
                }
            }
            else if (aObject.is<StrongReference>()) // Handle Implicit Cast
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apType->type)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::WeakHandle<RED4ext::IScriptable>(aObject.as<StrongReference>().m_strongHandle);
                    else
                        *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(apType->value) =
                            RED4ext::WeakHandle<RED4ext::IScriptable>();
                }
            }
        }
        else if (apType->type->GetType() == RED4ext::ERTTIType::Array)
        {
            //TODO: Support arrays
        }
        else if (apType->type->GetType() == RED4ext::ERTTIType::ScriptReference)
        {
            if (aObject.is<ClassReference>())
            {
                auto* pClassRef = aObject.as<ClassReference*>();
                auto* pScriptRef = static_cast<RED4ext::ScriptRef<void>*>(apType->value);
                pScriptRef->innerType = pClassRef->m_pType;
                pScriptRef->ref = pClassRef->GetHandle();
            }
        }
        else
        {
            Converter::ToRED(aObject, apType);
        }
    }
}
