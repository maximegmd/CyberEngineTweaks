#include <stdafx.h>

#include "Scripting.h"

#include "GameHooks.h"
#include "Options.h"
#include "GameOptions.h"

#include <sol_imgui/sol_imgui.h>

#include <d3d12/D3D12.h>
#include <console/Console.h>

#include <reverse/Type.h>
#include <reverse/BasicTypes.h>
#include <reverse/SingletonReference.h>
#include <reverse/StrongReference.h>
#include <reverse/Converter.h>
#include <reverse/WeakReference.h>
#include <reverse/Enum.h>

#include "CETVersion.h"

#ifndef NDEBUG
#include "GameDump.h"
#include <RED4ext/Dump/Reflection.hpp>
#endif

void Scripting::Initialize()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);

    sol_ImGui::InitBindings(m_lua);
    
    m_lua["GetDisplayResolution"] = []() -> std::tuple<float, float>
    {
        const auto resolution = D3D12::Get().GetResolution();
        return {static_cast<float>(resolution.cx), static_cast<float>(resolution.cy)};
    };

    m_lua["GetVersion"] = []() -> std::string
    {
        return CET_BUILD_COMMIT;
    };

    m_lua["SetTrapInputInImGui"] = [](bool trap)
    {
        D3D12::Get().SetTrapInputInImGui(trap);
    };

    m_lua["IsTrapInputInImGui"] = []() -> bool
    {
        return D3D12::Get().IsTrapInputInImGui();
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

    m_lua["GetMod"] = [this](const std::string& acName) -> sol::object
    {
        return GetMod(acName);
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
                const sol::state_view state(m_lua);
                
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
    m_lua["GetSingleton"] = [this](const std::string& acName)
    {
        return this->GetSingletonHandle(acName);
    };

    m_lua["ReloadAllMods"] = [this]()
    {
        ReloadAllMods();
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

        const Type type(m_lua, pType);
        return type.Dump(aDetailed);
    };

    m_lua["DumpAllTypeNames"] = [this]()
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();

        uint32_t count = 0;
        pRtti->types.for_each([&count](RED4ext::CName name, RED4ext::IRTTIType*& type)
        {
            spdlog::info(name.ToString());
            count++;
        });
        Console::Get().Log(fmt::format("Dumped {} types", count));
    };

    m_lua["print"] = [](sol::variadic_args aArgs, sol::this_environment aEnvironment, sol::this_state aState)
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
        spdlog::info(oss.str());
        Console::Get().Log(oss.str());
    };

    m_lua["GetAsyncKeyState"] = [](int aKeyCode) -> bool
    {
        return GetAsyncKeyState(aKeyCode) & 0x8000 != 0;
    };

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
    m_lua["DumpReflection"] = [this]()
    {
        RED4ext::GameReflection::Dump(Options::Get().CETPath / "dumps");
    };
#endif

    // execute autoexec.lua inside our default script directory
    current_path(Options::Get().CETPath / "scripts");
    if (std::filesystem::exists("autoexec.lua"))
        m_lua.do_file("autoexec.lua");
    else
    {
        Console::Get().Log("WARNING: missing CET autoexec.lua!");
        spdlog::warn("Scripting::Initialize() - missing CET autoexec.lua!");
    }

    // set current path for following scripts to out ScriptsPath
    current_path(Options::Get().ScriptsPath);

    // load mods
    ReloadAllMods();
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

void Scripting::TriggerOnConsoleOpen() const
{
    m_store.TriggerOnConsoleOpen();
}

void Scripting::TriggerOnConsoleClose() const
{
    m_store.TriggerOnConsoleClose();
}

sol::object Scripting::GetMod(const std::string& acName) const
{
    return m_store.GetMod(acName);
}

void Scripting::ReloadAllMods()
{
    m_store.LoadAll(m_lua);
}

bool Scripting::ExecuteLua(const std::string& acCommand)
{
    // TODO: proper exception handling!
    try
    {
        m_lua.script(acCommand);
        return true;
    }
    catch(std::exception& e)
    {
        Console::Get().Log(e.what());
    }
    return false;
}

size_t Scripting::Size(RED4ext::IRTTIType* apRtti)
{
    static auto* pRtti = RED4ext::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

    if (apRtti == pStringType)
        return sizeof(RED4ext::CString);
    if (apRtti->GetType() == RED4ext::ERTTIType::Handle)
        return sizeof(RED4ext::Handle<RED4ext::IScriptable>);
    if (apRtti->GetType() == RED4ext::ERTTIType::WeakHandle)
        return sizeof(RED4ext::WeakHandle<RED4ext::IScriptable>);

    return Converter::Size(apRtti);
}

sol::object Scripting::ToLua(sol::state_view aState, RED4ext::CStackType& aResult)
{
    static auto* pRtti = RED4ext::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

    auto* pType = aResult.type;

    if (pType == nullptr)
        return sol::nil;
    if (pType == pStringType)
        return make_object(aState, std::string(static_cast<RED4ext::CString*>(aResult.value)->c_str()));
    if (pType->GetType() == RED4ext::ERTTIType::Handle)
    {
         return make_object(aState, StrongReference(aState, *static_cast<RED4ext::Handle<RED4ext::IScriptable>*>(aResult.value)));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::WeakHandle)
    {
         const auto handle = *static_cast<RED4ext::WeakHandle<RED4ext::IScriptable>*>(aResult.value);
         if (!handle.Expired())
             return make_object(aState, WeakReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::Array)
    {
        auto* pArrayType = static_cast<RED4ext::CArray*>(pType);
        const uint32_t cLength = pArrayType->GetLength(aResult.value);
        sol::table result(aState, sol::create);
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

RED4ext::CStackType Scripting::ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    static auto* pRtti = RED4ext::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

    RED4ext::CStackType result;

    const bool hasData = aObject != sol::nil;

    if (apRtti)
    {
        result.type = apRtti;

        if (apRtti == pStringType)
        {
            std::string str;
            if (hasData)
            {
                sol::state_view v(aObject.lua_state());
                str = v["tostring"](aObject);
            }
            result.value = apAllocator->New<RED4ext::CString>(str.c_str());
        }
        else if (apRtti->GetType() == RED4ext::ERTTIType::Handle)
        {
            if (aObject.is<StrongReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRtti)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(aObject.as<StrongReference>().m_strongHandle);
                    else
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>();
                }
            }
            else if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRtti)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>(aObject.as<WeakReference>().m_weakHandle);
                    else
                        result.value = apAllocator->New<RED4ext::Handle<RED4ext::IScriptable>>();
                }
            }
        }
        else if (apRtti->GetType() == RED4ext::ERTTIType::WeakHandle)
        {
            if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRtti)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<WeakReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>(aObject.as<WeakReference>().m_weakHandle);
                    else
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>();
                }
            }
            else if (aObject.is<StrongReference>()) // Handle Implicit Cast
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRtti)->parent;
                auto* pType = static_cast<RED4ext::CClass*>(aObject.as<StrongReference*>()->m_pType);
                if (pType && pType->IsA(pSubType))
                {
                    if (hasData)
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>(aObject.as<StrongReference>().m_strongHandle);
                    else
                        result.value = apAllocator->New<RED4ext::WeakHandle<RED4ext::IScriptable>>();
                }
            }
        }
        else if (apRtti->GetType() == RED4ext::ERTTIType::Array)
        {
            auto* pArrayType = static_cast<RED4ext::CArray*>(apRtti);
            const auto pMemory = static_cast<RED4ext::DynArray<void*>*>(apAllocator->Allocate(apRtti->GetSize()));
            apRtti->Init(pMemory);

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
        else
        {
            return Converter::ToRED(aObject, apRtti, apAllocator);
        }
    }

    return result;
}

sol::object Scripting::Index(const std::string& acName)
{
    if(const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    return InternalIndex(acName);
}

sol::object Scripting::NewIndex(const std::string& acName, sol::object aParam)
{
    auto& property = m_properties[acName];
    property = std::move(aParam);
    return property;
}

sol::object Scripting::GetSingletonHandle(const std::string& acName)
{
    auto itor = m_singletons.find(acName);
    if (itor != std::end(m_singletons))
        return make_object(m_lua, itor->second);

    auto* pRtti = RED4ext::CRTTISystem::Get();
    auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
    if (!pType)
    {
        Console::Get().Log("Type '" + acName + "' not found or is not initialized yet.");
        return sol::nil;
    }

    auto result = m_singletons.emplace(std::make_pair(acName, SingletonReference{ m_lua, pType }));
    return make_object(m_lua, result.first->second);
}

sol::protected_function Scripting::InternalIndex(const std::string& acName)
{
    const sol::state_view state(m_lua);
    auto obj = make_object(state, [this, name = acName](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::string result;
        auto code = this->Execute(name, args, std::move(env), L, result);
        if(!code)
        {
            Console::Get().Log("Error: " + result);
        }
        return code;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage) const
{
    auto* pRtti = RED4ext::CRTTISystem::Get();

    RED4ext::CBaseFunction* pFunc = pRtti->GetFunction(RED4ext::FNV1a(aFuncName.c_str()));

    static const auto hashcpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");
    static const auto hashGameInstance = RED4ext::FNV1a("ScriptGameInstance");
    auto* pGIType = pRtti->GetType(RED4ext::FNV1a("ScriptGameInstance"));

    auto* pPlayerSystem = pRtti->GetClass(hashcpPlayerSystem);
    auto* gameInstanceType = pRtti->GetClass(hashGameInstance);

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
    auto* unk10 = engine->framework->gameInstance;

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
            args[0].value = &unk10;
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

    auto* pScriptable = unk10->GetInstance(pPlayerSystem);
    RED4ext::CStack stack(pScriptable, args.data(), static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);

    const auto success = pFunc->Execute(&stack);
    if (!success)
        return sol::nil;

    if(hasReturnType)
        return ToLua(m_lua, result);

    return make_object(m_lua, true);
}
