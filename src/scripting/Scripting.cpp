#include <stdafx.h>

#include "Scripting.h"

#include "FunctionOverride.h"
#include "GameOptions.h"
#include "LuaVM.h"


#include <sol_imgui/sol_imgui.h>
#include <lsqlite3/lsqlite3.h>

#include <d3d12/D3D12.h>

#include <reverse/Type.h>
#include <reverse/BasicTypes.h>
#include <reverse/SingletonReference.h>
#include <reverse/StrongReference.h>
#include <reverse/Converter.h>
#include <reverse/WeakReference.h>
#include <reverse/Enum.h>
#include <reverse/TweakDB.h>
#include <reverse/RTTILocator.h>

#include "Utils.h"

#ifndef NDEBUG
#include "GameHooks.h"
#include "GameDump.h"
#include <RED4ext/Dump/Reflection.hpp>
#endif

static RTTILocator s_stringType{RED4ext::FNV1a("String")};

struct Context
{
    sol::protected_function ScriptFunction;
    RED4ext::CClassFunction* RealFunction{nullptr};
    RED4ext::CClassFunction* FunctionDefinition{nullptr};
    sol::environment Environment;
    Scripting* pScripting;
    bool Forward;
};

void Scripting::HandleOverridenFunction(RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, int32_t* apOut, int64_t a4, Context* apCookie)
{
    auto* pRealFunction = apCookie->RealFunction;

    // Save state so we can rollback to it after we popped for ourself
    auto* pCode = apFrame->code;

    RED4ext::CStackType self;
    self.type = apContext->classType;
    self.value = apContext;

    {
        auto state = apCookie->pScripting->GetState();

        std::vector<sol::object> args;
        args.push_back(ToLua(state, self)); // Push self

        // Nasty way of popping all args
        for (auto& pArg : apCookie->FunctionDefinition->params)
        {
            auto* pType = pArg->type;
            auto* pAllocator = pType->GetAllocator();

            auto* pInstance = pAllocator->Alloc(pType->GetSize()).memory;
            pType->Init(pInstance);

            RED4ext::CStackType arg;
            arg.type = pArg->type;
            arg.value = pInstance;

            apFrame->unk30 = 0;
            apFrame->unk38 = 0;
            const auto opcode = *(apFrame->code++);
            GetScriptCallArray()[opcode](apFrame->context, apFrame, pInstance, nullptr);
            apFrame->code++; // skip ParamEnd

            args.push_back(ToLua(state, arg));

            pType->Destroy(pInstance);
            pAllocator->Free(pInstance);
        }

        const auto result = apCookie->ScriptFunction(as_args(args), apCookie->Environment);

        if (!apCookie->Forward)
        {
            RED4ext::CStackType redResult;
            redResult.type = apCookie->FunctionDefinition->returnType->type;
            redResult.value = apOut;

            if (apOut)
                ToRED(result.get<sol::object>(), &redResult);

            return;
        }
    }
    

    if (pRealFunction)
    {
        // Rollback so the real function will manage to pop everything
        apFrame->code = pCode;

        using TCallScriptFunction = bool(*)(RED4ext::IFunction* apFunction, RED4ext::IScriptable* apContext, RED4ext::CStackFrame* apFrame, int32_t* apOut, int64_t a4);
        static RED4ext::REDfunc<TCallScriptFunction> CallScriptFunction(0x224DC0);

        CallScriptFunction(pRealFunction, apContext, apFrame, apOut, a4);
    }
}

void Scripting::Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
    bool aAbsolute, sol::protected_function aFunction, sol::this_environment aThisEnv)
{
    auto pRtti = RED4ext::CRTTISystem::Get();
    auto pClassType = pRtti->GetClass(acTypeName.c_str());

    if (!pClassType)
    {
        spdlog::get("scripting")->error("Class type {} not found", acTypeName);
        return;
    }

    // At this point r15 is a pointer to CStack*
    /*
sub rsp, 56
mov rax, 0xDEADBEEFC0DEBAAD
mov qword ptr[rsp + 32], rax
mov rax, 0xDEADBEEFC0DEBAAD
call rax
add rsp, 56
ret
     */
    uint8_t payload[] = {0x48, 0x83, 0xEC, 0x38, 0x48, 0xB8, 0xAD, 0xBA, 0xDE, 0xC0, 0xEF, 0xBE, 0xAD,
                         0xDE, 0x48, 0x89, 0x44, 0x24, 0x20, 0x48, 0xB8, 0xAD, 0xBA, 0xDE, 0xC0, 0xEF,
                         0xBE, 0xAD, 0xDE, 0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x38, 0xC3};

    auto funcAddr = reinterpret_cast<uintptr_t>(&Scripting::HandleOverridenFunction);

    auto* pContext = TiltedPhoques::New<Context>();
    pContext->ScriptFunction = aFunction;
    pContext->Environment = aThisEnv;
    pContext->pScripting = this;

    std::memcpy(payload + 6, &pContext, 8);
    std::memcpy(payload + 21, &funcAddr, 8);

    auto* pExecutablePayload = static_cast<void (*)(RED4ext::IScriptable*, RED4ext::CStackFrame*, void*, int64_t)>(
        FunctionOverride::Get().MakeExecutable(payload, std::size(payload)));

    if (!pExecutablePayload)
    {
        spdlog::get("scripting")->error("Unable to create the override payload!");
        return;
    }

    // Get the real function
    const auto pRealFunction = pClassType->GetFunction(RED4ext::FNV1a(acFullName.c_str()));
    const auto pFunc =
        RED4ext::CClassFunction::Create(pClassType, acFullName.c_str(), acShortName.c_str(), pExecutablePayload);

    if (pRealFunction)
    {
        pFunc->returnType = pRealFunction->returnType;
        for (auto p : pRealFunction->params)
        {
            pFunc->params.PushBack(p);
        }

        for (auto p : pRealFunction->localVars)
        {
            pFunc->localVars.PushBack(p);
        }

        pFunc->unk7C = pRealFunction->unk7C;
    }
    pFunc->flags.isNative = true;

    if (!pRealFunction)
    {
        if (pFunc->flags.isStatic)
            pClassType->staticFuncs.PushBack(pFunc);
        else
            pClassType->funcs.PushBack(pFunc);

        pContext->FunctionDefinition = pFunc;
    }
    else
    {
        // Swap the content of the real function with the one we just created
        std::array<char, sizeof(RED4ext::CClassFunction)> tmpBuffer;

        std::memcpy(&tmpBuffer, pRealFunction, sizeof(RED4ext::CClassFunction));
        std::memcpy(pRealFunction, pFunc, sizeof(RED4ext::CClassFunction));
        std::memcpy(pFunc, &tmpBuffer, sizeof(RED4ext::CClassFunction));

        // Now pFunc is the real function
        pContext->RealFunction = pFunc;
        pContext->FunctionDefinition = pRealFunction;
    }

    pContext->Forward = !aAbsolute;
}

Scripting::Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12)
    : m_store(m_sandbox, aPaths, aBindings)
    , m_paths(aPaths)
    , m_d3d12(aD3D12)
{
    CreateLogger(aPaths.CETRoot() / "scripting.log", "scripting");
}

void Scripting::Initialize()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);
    m_lua.require("sqlite3", luaopen_lsqlite3);

    sol_ImGui::InitBindings(m_lua);

    m_lua["GetDisplayResolution"] = [this]() -> std::tuple<float, float>
    {
        const auto resolution = m_d3d12.GetResolution();
        return {static_cast<float>(resolution.cx), static_cast<float>(resolution.cy)};
    };

    m_lua["GetVersion"] = []() -> std::string
    {
        return CET_BUILD_COMMIT;
    };

    m_lua["GetMod"] = [this](const std::string& acName) -> sol::object
    {
        return GetMod(acName);
    };

    m_lua["ToVector3"] = [](sol::table table) -> Vector3
    {
        return Vector3
        {
            table["x"].get_or(0.f),
            table["y"].get_or(0.f),
            table["z"].get_or(0.f)
        };
    };

    m_lua.new_usertype<Scripting>("__Game",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Scripting::Index,
        sol::meta_function::new_index, &Scripting::NewIndex);

    m_lua.new_usertype<Type>("__Type",
        sol::meta_function::construct, sol::no_constructor,
        sol::meta_function::index, &Type::Index,
        sol::meta_function::new_index, &Type::NewIndex);

    m_lua.new_usertype<StrongReference>("StrongReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &StrongReference::Index,
        sol::meta_function::new_index, &StrongReference::NewIndex);

    m_lua.new_usertype<WeakReference>("WeakReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &WeakReference::Index,
        sol::meta_function::new_index, &WeakReference::NewIndex);

    m_lua.new_usertype<SingletonReference>("SingletonReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &SingletonReference::Index,
        sol::meta_function::new_index, &SingletonReference::NewIndex);

    m_lua.new_usertype<ClassReference>("ClassReference",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &ClassReference::Index,
        sol::meta_function::new_index, &ClassReference::NewIndex);

    m_lua.new_usertype<UnknownType>("Unknown",
        sol::meta_function::construct, sol::no_constructor,
        sol::base_classes, sol::bases<Type>(),
        sol::meta_function::index, &UnknownType::Index,
        sol::meta_function::new_index, &UnknownType::NewIndex);

    m_lua.new_usertype<GameOptions>("GameOptions",
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

    m_lua.new_usertype<Vector3>("Vector3",
        sol::constructors<Vector3(float, float, float), Vector3(float, float), Vector3(float), Vector3()>(),
        sol::meta_function::to_string, &Vector3::ToString,
        "x", &Vector3::x,
        "y", &Vector3::y,
        "z", &Vector3::z);

    m_lua["ToVector3"] = [](sol::table table) -> Vector3
    {
        return Vector3
        {
            table["x"].get_or(0.f),
            table["y"].get_or(0.f),
            table["z"].get_or(0.f)
        };
    };

    m_lua.new_usertype<Vector4>("Vector4",
        sol::constructors<Vector4(float, float, float, float), Vector4(float, float, float), Vector4(float, float), Vector4(float), Vector4()>(),
        sol::meta_function::to_string, &Vector4::ToString,
        "x", &Vector4::x,
        "y", &Vector4::y,
        "z", &Vector4::z,
        "w", &Vector4::w);

    m_lua.new_usertype<Enum>("Enum",
        sol::constructors<Enum(const std::string&, const std::string&), Enum(const std::string&, uint32_t)>(),
        sol::meta_function::to_string, &Enum::ToString,
        "value", sol::property(&Enum::GetValueName, &Enum::SetValueByName));

    m_lua["ToVector4"] = [](sol::table table) -> Vector4
    {
        return Vector4
        {
            table["x"].get_or(0.f),
            table["y"].get_or(0.f),
            table["z"].get_or(0.f),
            table["w"].get_or(0.f)
        };
    };

    m_lua.new_usertype<EulerAngles>("EulerAngles",
        sol::constructors<EulerAngles(float, float, float), EulerAngles(float, float), EulerAngles(float), EulerAngles()>(),
        sol::meta_function::to_string, &EulerAngles::ToString,
        "pitch", &EulerAngles::pitch,
        "yaw", &EulerAngles::yaw,
        "roll", &EulerAngles::roll);

    m_lua["ToEulerAngles"] = [](sol::table table) -> EulerAngles
    {
        return EulerAngles
        {
            table["pitch"].get_or(0.f),
            table["roll"].get_or(0.f),
            table["yaw"].get_or(0.f)
        };
    };

    m_lua.new_usertype<Quaternion>("Quaternion",
        sol::constructors<Quaternion(float, float, float, float), Quaternion(float, float, float), Quaternion(float, float), Quaternion(float), Quaternion()>(),
        sol::meta_function::to_string, &Quaternion::ToString,
        "i", &Quaternion::i,
        "j", &Quaternion::j,
        "k", &Quaternion::k,
        "r", &Quaternion::r);

    m_lua["ToQuaternion"] = [](sol::table table) -> Quaternion
    {
        return Quaternion
        {
            table["i"].get_or(0.f),
            table["j"].get_or(0.f),
            table["k"].get_or(0.f),
            table["r"].get_or(0.f)
        };
    };

    m_lua.new_usertype<CName>("CName",
        sol::constructors<CName(const std::string&), CName(uint32_t), CName(uint32_t, uint32_t), CName()>(),
        sol::meta_function::to_string, &CName::ToString,
        "hash_lo", &CName::hash_lo,
        "hash_hi", &CName::hash_hi,
        "value", sol::property(&CName::AsString));

    m_lua["ToCName"] = [](sol::table table) -> CName
    {
        return CName
        {
            table["hash_lo"].get_or<uint32_t>(0),
            table["hash_hi"].get_or<uint32_t>(0)
        };
    };

    m_lua.new_usertype<TweakDBID>("TweakDBID",
        sol::constructors<TweakDBID(const std::string&), TweakDBID(const TweakDBID&, const std::string&), TweakDBID(uint32_t, uint8_t), TweakDBID()>(),
        sol::meta_function::to_string, &TweakDBID::ToString,
        "hash", &TweakDBID::name_hash,
        "length", &TweakDBID::name_length);

    m_lua["ToTweakDBID"] = [](sol::table table) -> TweakDBID
    {
        return TweakDBID
        {
            table["hash"].get_or<uint32_t>(0),
            table["length"].get_or<uint8_t>(0)
        };
    };

    m_lua.new_usertype<ItemID>("ItemID",
        sol::constructors<ItemID(const TweakDBID&, uint32_t, uint16_t, uint8_t), ItemID(const TweakDBID&, uint32_t, uint16_t), ItemID(const TweakDBID&, uint32_t), ItemID(const TweakDBID&), ItemID()>(),
        sol::meta_function::to_string, &ItemID::ToString,
        "id", &ItemID::id,
        "tdbid", &ItemID::id,
        "rng_seed", &ItemID::rng_seed,
        "unknown", &ItemID::unknown,
        "maybe_type", &ItemID::maybe_type);

    m_lua["ToItemID"] = [](sol::table table) -> ItemID
    {
        return ItemID
        {
            table["id"].get_or<TweakDBID>(0),
            table["rng_seed"].get_or<uint32_t>(2),
            table["unknown"].get_or<uint16_t>(0),
            table["maybe_type"].get_or<uint8_t>(0),
        };
    };

    m_lua["NewObject"] = [this](const std::string& acName) -> sol::object
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();
        auto* pType = pRtti->GetType(RED4ext::FNV1a(acName.c_str()));
        if (pType)
        {
            RED4ext::CClass* pClass = nullptr;
            if (pType->GetType() == RED4ext::ERTTIType::Handle)
            {
                auto* pInnerType = static_cast<RED4ext::CHandle*>(pType)->GetInnerType();
                pClass = pInnerType->GetType() == RED4ext::ERTTIType::Class ? static_cast<RED4ext::CClass*>(pInnerType)
                                                                       : nullptr;
            }
            else if (pType->GetType() == RED4ext::ERTTIType::Class)
            {
                pClass = static_cast<RED4ext::CClass*>(pType);
            }

            if (pClass && !pClass->flags.isAbstract)
            {
                auto state = GetState();
                
                if (pType->GetType() == RED4ext::ERTTIType::Handle)
                {
                    RED4ext::CStackType stackType;
                    RED4ext::Handle<RED4ext::IScriptable> clsHandle(pClass->AllocInstance());
                    stackType.type = pType;
                    stackType.value = &clsHandle;
                    return ToLua(state, stackType);
                }
                else
                {
                    RED4ext::CStackType stackType;
                    stackType.type = pClass;
                    stackType.value = pClass->AllocInstance();
                    return ToLua(state, stackType);
                }
            }
        }

        return sol::nil;
    };

    m_lua.new_usertype<Type::Descriptor>("Descriptor",
        sol::meta_function::to_string, &Type::Descriptor::ToString);

    m_lua["Game"] = this;
    m_lua["GetSingleton"] = [this](const std::string& acName, sol::this_environment aThisEnv)
    {
        return this->GetSingletonHandle(acName, aThisEnv);
    };

    m_lua["GameDump"] = [this](Type* apType)
    {
        return apType ? apType->GameDump() : "Null";
    };

    m_lua["Dump"] = [this](Type* apType, bool aDetailed)
    {
        return apType != nullptr ? apType->Dump(aDetailed) : Type::Descriptor{};
    };

    m_lua["DumpType"] = [this](const std::string& acName, bool aDetailed)
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();
        auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
        if (!pType || pType->GetType() == RED4ext::ERTTIType::Simple)
            return Type::Descriptor();

        const Type type({&m_lua, m_vmLock}, pType);
        return type.Dump(aDetailed);
    };
    
    m_lua["DumpAllTypeNames"] = [this](sol::this_environment aThisEnv)
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

    m_lua["print"] = [](sol::variadic_args aArgs, sol::this_state aState)
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

    m_lua.new_usertype<TweakDB>("__TweakDB",
        sol::meta_function::construct, sol::no_constructor,
        "DebugStats", &TweakDB::DebugStats,
        "GetRecord", &TweakDB::GetRecord,
        "Query", &TweakDB::Query,
        "GetFlat", &TweakDB::GetFlat,
        "SetFlat", &TweakDB::SetFlat,
        "Update", overload(&TweakDB::UpdateRecordByID, &TweakDB::UpdateRecord));

    m_lua["TweakDB"] = TweakDB({&m_lua, m_vmLock});

    m_lua["Override"] = sol::overload(
        [this](const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction,
               sol::this_environment aThisEnv) {
            this->Override(acTypeName, acFullName, acFullName, true, aFunction, aThisEnv);
        },
        [this](const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
               sol::protected_function aFunction, sol::this_environment aThisEnv) {
            this->Override(acTypeName, acFullName, acShortName, true, aFunction, aThisEnv);
        });

    m_lua["Observe"] = sol::overload(
        [this](const std::string& acTypeName, const std::string& acFullName, sol::protected_function aFunction,
               sol::this_environment aThisEnv) {
            this->Override(acTypeName, acFullName, acFullName, false, aFunction, aThisEnv);
        },
        [this](const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
               sol::protected_function aFunction, sol::this_environment aThisEnv) {
            this->Override(acTypeName, acFullName, acShortName, false, aFunction, aThisEnv);
        });

#ifndef NDEBUG
    m_lua["DumpVtables"] = [this]()
    {
        // Hacky RTTI dump, this should technically only dump IScriptable instances and RTTI types as they are guaranteed to have a vtable
        // but there can occasionally be Class types that are not IScriptable derived that still have a vtable
        // some hierarchies may also not be accurately reflected due to hash ordering
        // technically this table is flattened and contains all hierarchy, but traversing the hierarchy first reduces
        // error when there are classes that instantiate a parent class but don't actually have a subclass instance
        GameMainThread::Get().AddTask(&GameDump::DumpVTablesTask::Run);
    };
    m_lua["DumpReflection"] = [this](bool aVerbose, bool aExtendedPath, bool aPropertyHolders)
    {
        
        RED4ext::GameReflection::Dump(m_paths.CETRoot() / "dumps", aVerbose, aExtendedPath, aPropertyHolders);
    };
#endif

    // execute autoexec.lua inside our default script directory
    current_path(m_paths.CETRoot() / "scripts");
    if (std::filesystem::exists("autoexec.lua"))
        m_lua.do_file("autoexec.lua");
    else
    {
        spdlog::get("scripting")->warn("WARNING: missing CET autoexec.lua!");
    }

    // initialize sandbox
    m_sandbox.Initialize();

    // setup logger for console sandbox
    auto& consoleSB = m_sandbox[0];
    auto& consoleSBEnv = consoleSB.GetEnvironment();
    consoleSBEnv["__logger"] = spdlog::get("scripting");

    // load mods
    ReloadAllMods();
}

const std::vector<VKBindInfo>& Scripting::GetBinds() const
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
    // set current path for following scripts to our ModsPath
    current_path(m_paths.ModsRoot());

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

Scripting::LockedState Scripting::GetState() const noexcept
{
    return { &m_lua, m_vmLock };
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

    auto& state = *aState.Get();

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
            auto* pArrayType = static_cast<RED4ext::CArray*>(apRttiType);
            const auto pMemory = static_cast<RED4ext::DynArray<void*>*>(apAllocator->Allocate(apRttiType->GetSize()));
            apRttiType->Init(pMemory);

            if (hasData && aObject.is<sol::table>())
            {
                auto* pArrayInnerType = pArrayType->GetInnerType();

                // Copy elements from the table into the array
                auto tbl = aObject.as<sol::table>();
                pArrayType->Resize(pMemory, tbl.size());
                for (uint32_t i = 1; i <= tbl.size(); ++i)
                {
                    RED4ext::CStackType type = ToRED(tbl.get<sol::object>(i), pArrayInnerType, apAllocator);
                    const auto pElement = pArrayType->GetElement(pMemory, i - 1);
                    pArrayInnerType->Assign(pElement, type.value);
                }
            }

            result.value = pMemory;
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

sol::object Scripting::Index(const std::string& acName, sol::this_environment aThisEnv)
{
    if(const auto itor = m_properties.find(acName); itor != m_properties.end())
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

sol::object Scripting::GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv)
{
    auto itor = m_singletons.find(acName);
    if (itor != std::end(m_singletons))
        return make_object(m_lua, itor->second);

    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
    if (!pType)
    {
        const sol::environment cEnv = aThisEnv;
        std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->info("Type '{}' not found or is not initialized yet.", acName);
        return sol::nil;
    }

    auto result = m_singletons.emplace(std::make_pair(acName, SingletonReference{{&m_lua, m_vmLock}, pType}));
    return make_object(m_lua, result.first->second);
}

sol::protected_function Scripting::InternalIndex(const std::string& acName, sol::this_environment aThisEnv)
{
    const sol::state_view state(m_lua);
    auto obj = make_object(state, [this, name = acName](sol::variadic_args aArgs, sol::this_environment aThisEnv)
    {
        std::string result;
        auto code = this->Execute(name, aArgs, result);
        if(!code)
        {
            const sol::environment cEnv = aThisEnv;
            std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Error: {}", (result.empty()) ? ("Unknown error") : (result));
        }
        return code;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, std::string& aReturnMessage) const
{
    auto* pRtti = RED4ext::CRTTISystem::Get();
    if (pRtti == nullptr)
    {
        aReturnMessage = "Could not retrieve RTTISystem instance.";
        return sol::nil;
    }

    if (s_stringType == nullptr)
    {
        aReturnMessage = "Could not retrieve String type instance.";
        return sol::nil;
    }

    RED4ext::CBaseFunction* pFunc = pRtti->GetFunction(RED4ext::FNV1a(aFuncName.c_str()));

    static const auto hashcpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");
    static const auto hashGameInstance = RED4ext::FNV1a("ScriptGameInstance");
    auto* pGIType = pRtti->GetType(RED4ext::FNV1a("ScriptGameInstance"));

    auto* pPlayerSystem = pRtti->GetClass(hashcpPlayerSystem);
    if (pPlayerSystem == nullptr)
    {
        aReturnMessage = "Could not retrieve cpPlayerSystem class.";
        return sol::nil;
    }

    auto* gameInstanceType = pRtti->GetClass(hashGameInstance);
    if (gameInstanceType == nullptr)
    {
        aReturnMessage = "Could not retrieve ScriptGameInstance class.";
        return sol::nil;
    }

    if (!pFunc)
    {
        pFunc = gameInstanceType->GetFunction(RED4ext::FNV1a(aFuncName.c_str()));
        if (!pFunc)
        {
            aReturnMessage = "Function '" + aFuncName + "' not found or is not a global.";
            return sol::nil;
        }
    }

    // 8KB should cut it
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 13);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    using CStackType = RED4ext::CStackType;
    const auto* engine = RED4ext::CGameEngine::Get();
    if (engine == nullptr)
    {
        aReturnMessage = "Could not retrieve GameEngine instance.";
        return sol::nil;
    }

    auto* framework = engine->framework;
    if (framework == nullptr)
    {
        aReturnMessage = "Could not retrieve GameFramework instance.";
        return sol::nil;
    }

    auto* gameInstance = framework->gameInstance;
    if (gameInstance == nullptr)
    {
        aReturnMessage = "Could not retrieve GameInstance instance.";
        return sol::nil;
    }

    RED4ext::CName name;

    std::vector<CStackType> args(pFunc->params.size);

    uint32_t argOffset = 0u;
    if (pFunc->params.size > 0)
    {
        auto* pType = pFunc->params[0]->type;
        // check if the first argument is expected to be ScriptGameInstance
        if (pType == pGIType)
        {
            argOffset = 1u;
            args[0].type = pGIType;
            args[0].value = &gameInstance;
        }
    }

    int32_t minArgs = 0;
    for (auto i = argOffset; i < pFunc->params.size; ++i)
    {
        if (!pFunc->params[i]->flags.isOut && !pFunc->params[i]->flags.isOptional)
        {
            minArgs++;
        }
    }

    if (minArgs > aArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' requires at least " + std::to_string(minArgs) + " parameter(s).";
        return sol::nil;
    }

    for (auto i = argOffset; i < pFunc->params.size; ++i)
    {
        if (pFunc->params[i]->flags.isOut) // Deal with out params
        {
            args[i] = ToRED(sol::nil, pFunc->params[i]->type, &s_scratchMemory);
        }
        else if (i - argOffset < aArgs.size())
        {
            args[i] = ToRED(aArgs[i - argOffset].get<sol::object>(), pFunc->params[i]->type, &s_scratchMemory);
        }
        else if (pFunc->params[i]->flags.isOptional) // Deal with optional params
        {
            args[i].value = nullptr;
        }

        if (!args[i].value && !pFunc->params[i]->flags.isOptional)
        {
            auto* pType = pFunc->params[i]->type;

            RED4ext::CName hash;
            pType->GetName(hash);
            if (!hash.IsEmpty())
            {
                std::string typeName = hash.ToString();
                aReturnMessage =
                    "Function '" + aFuncName + "' parameter " + std::to_string(i - argOffset) + " must be " + typeName + ".";
            }

            return sol::nil;
        }
    }

    const bool hasReturnType = (pFunc->returnType) != nullptr && (pFunc->returnType->type) != nullptr;

    uint8_t buffer[1000]{ 0 };
    CStackType result;
    if (hasReturnType)
    {
        result.value = &buffer;
        result.type = pFunc->returnType->type;
    }

    auto* pScriptable = gameInstance->GetInstance(pPlayerSystem);
    if (pScriptable == nullptr)
    {
        aReturnMessage = "Could not retrieve ScriptInstance from cpPlayerSystem instance.";
        return sol::nil;
    }
    RED4ext::CStack stack(pScriptable, args.data(), static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);

    const auto success = pFunc->Execute(&stack);
    if (!success)
    {
        aReturnMessage = "Function '" + aFuncName + "' failed to execute!";
        return sol::nil;
    }

    if (hasReturnType)
    {
        auto state = GetState();
        return ToLua(state, result);
    }

    return make_object(m_lua, true);
}
