#include <stdafx.h>

#include "Scripting.h"

#include "Options.h"
#include "GameOptions.h"

#include "overlay/Overlay.h"

#include "reverse/Type.h"
#include "reverse/Array.h"
#include "reverse/BasicTypes.h"
#include "reverse/SingletonReference.h"
#include "reverse/StrongReference.h"
#include "reverse/Converter.h"
#include "reverse/WeakReference.h"

Scripting::Scripting()
{
    Initialize();

    m_store.LoadAll(m_lua);
}

Scripting& Scripting::Get()
{
    static Scripting s_instance;
    return s_instance;
}

const ScriptStore& Scripting::GetStore() const
{
    return m_store;
}

bool Scripting::ExecuteLua(const std::string& aCommand)
{
    try
    {
        m_lua.script(aCommand);
    }
    catch(std::exception& e)
    {
        Overlay::Get().Log( e.what());
        return false;
    }

    return true;
}

size_t Scripting::Size(RED4ext::IRTTIType* apRtti)
{
    static auto* pRtti = RED4ext::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

    if (apRtti == pStringType)
        return sizeof(RED4ext::CString);
    if (apRtti->GetType() == RED4ext::ERTTIType::Handle)
        return sizeof(StrongHandle);
    if (apRtti->GetType() == RED4ext::ERTTIType::WeakHandle)
        return sizeof(WeakHandle);

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
        const auto handle = *static_cast<StrongHandle*>(aResult.value);
        if (handle.handle)
            return make_object(aState, StrongReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::WeakHandle)
    {
        const auto handle = *static_cast<WeakHandle*>(aResult.value);
        if (handle.handle)
            return make_object(aState, WeakReference(aState, handle));
    }
    else if (pType->GetType() == RED4ext::ERTTIType::Array)
    {
        auto* pArrayType = static_cast<RED4ext::CArray*>(pType);
        const auto arrayHandle = *static_cast<Array<uint8_t>*>(aResult.value);
        sol::table result(aState, sol::create);
        for(auto i = 0u; i < arrayHandle.count; ++i)
        {
            RED4ext::CStackType el;
            el.value = arrayHandle.entries + i * Size(pArrayType);
            el.type = pArrayType->GetInnerType();

            RED4ext::CName hash;
            pArrayType->GetInnerType()->GetName(hash);

            result[i + 1] = ToLua(aState, el);
        }

        return result;
    }
    else
    {
        auto result = Converter::ToLua(aResult, aState);
        if (result != sol::nil)
            return result;

        RED4ext::CName hash;
        pType->GetName(hash);
        if (hash)
        {
            const std::string typeName = hash.ToString();
            Overlay::Get().Log("Unhandled return type: " + typeName + " type : " + std::to_string((uint32_t)pType->GetType()));
        }
    }

    return sol::nil;
}

RED4ext::CStackType Scripting::ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator)
{
    static auto* pRtti = RED4ext::CRTTISystem::Get();
    static auto* pStringType = pRtti->GetType(RED4ext::FNV1a("String"));

    RED4ext::CStackType result;

    bool hasData = aObject != sol::nil;

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
                RED4ext::IRTTIType* pType = aObject.as<StrongReference*>()->m_pType;
                while (pType != nullptr && pType != pSubType)
                {
                    pType = static_cast<RED4ext::CClass*>(pType)->parent;
                }

                if (pType != nullptr)
                {
                    if (hasData)
                        result.value = apAllocator->New<StrongHandle>(aObject.as<StrongReference>().m_strongHandle);
                    else
                        result.value = apAllocator->New<StrongHandle>();
                }
            }
        }
        else if (apRtti->GetType() == RED4ext::ERTTIType::WeakHandle)
        {
            if (aObject.is<WeakReference>())
            {
                auto* pSubType = static_cast<RED4ext::CClass*>(apRtti)->parent;
                RED4ext::IRTTIType* pType = aObject.as<WeakReference*>()->m_pType;
                while (pType != nullptr && pType != pSubType)
                {
                    pType = static_cast<RED4ext::CClass*>(pType)->parent;
                }

                if (pType != nullptr)
                {
                    if (hasData)
                        result.value = apAllocator->New<WeakHandle>(aObject.as<WeakReference>().m_weakHandle);
                    else
                        result.value = apAllocator->New<WeakHandle>();
                }
            }
        }
        else if (apRtti->GetType() == RED4ext::ERTTIType::Array)
        {
            if (!hasData)
                result.value = apAllocator->New<Array<void*>>();
        }
        else
        {
            return Converter::ToRED(aObject, apRtti, apAllocator);
        }
    }

    return result;
}

void Scripting::Initialize()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);

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

    m_lua["ToVector3"] = [this](sol::table table) -> Vector3
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

    m_lua["ToVector4"] = [this](sol::table table) -> Vector4
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
        return GetStore().Get(acName);
    };

    m_lua.new_usertype<EulerAngles>("EulerAngles",
        sol::constructors<EulerAngles(float, float, float), EulerAngles(float, float), EulerAngles(float), EulerAngles()>(),
        sol::meta_function::to_string, &EulerAngles::ToString,
        "pitch", &EulerAngles::pitch,
        "yaw", &EulerAngles::yaw,
        "roll", &EulerAngles::roll);

    m_lua["ToEulerAngles"] = [this](sol::table table) -> EulerAngles
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

    m_lua["ToQuaternion"] = [this](sol::table table) -> Quaternion
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
        "hash_hi", &CName::hash_hi);

    m_lua["ToCName"] = [this](sol::table table) -> CName
    {
        return CName
        {
            table["hash_lo"].get_or<uint32_t>(0),
            table["hash_hi"].get_or<uint32_t>(0)
        };
    };

    m_lua.new_usertype<TweakDBID>("TweakDBID",
        sol::constructors<TweakDBID(const std::string&), TweakDBID(const TweakDBID&, const std::string&), TweakDBID(uint32_t, uint8_t), TweakDBID()>(),
        sol::meta_function::to_string, &TweakDBID::ToString);

    m_lua["ToTweakDBID"] = [this](sol::table table) -> TweakDBID
    {
        return TweakDBID
        {
            table["hash"].get_or<uint32_t>(0),
            table["length"].get_or<uint8_t>(0)
        };
    };

    m_lua.new_usertype<ItemID>("ItemID",
        sol::constructors<ItemID(const TweakDBID&, uint32_t, uint16_t, uint8_t), ItemID(const TweakDBID&, uint32_t, uint16_t), ItemID(const TweakDBID&, uint32_t), ItemID(const TweakDBID&), ItemID()>(),
        sol::meta_function::to_string, &ItemID::ToString);

    m_lua["ToItemID"] = [this](sol::table table) -> ItemID
    {
        return ItemID
        {
            table["id"].get_or<TweakDBID>(0),
            table["rng_seed"].get_or<uint32_t>(2),
            table["unknown"].get_or<uint16_t>(0),
            table["maybe_type"].get_or<uint8_t>(0),
        };
    };

    m_lua.new_usertype<Type::Descriptor>("Descriptor",
        sol::meta_function::to_string, &Type::Descriptor::ToString);

    m_lua["Game"] = this;
    m_lua["GetSingleton"] = [this](const std::string& acName)
    {
        return this->GetSingletonHandle(acName);
    };

    m_lua["Dump"] = [this](Type* apType)
    {
        return apType != nullptr ? apType->Dump() : Type::Descriptor{};
    };

    m_lua["DumpType"] = [this](const std::string& acName)
    {
        auto* pRtti = RED4ext::CRTTISystem::Get();
        auto* pType = pRtti->GetClass(RED4ext::FNV1a(acName.c_str()));
        if (!pType || pType->GetType() == RED4ext::ERTTIType::Simple)
            return Type::Descriptor();

        Type type(m_lua, pType);
        return type.Dump();
    };

    m_lua["print"] = [](sol::variadic_args args, sol::this_environment env, sol::this_state L)
    {
        std::ostringstream oss;
        sol::state_view s(L);
        for (auto it = args.cbegin(); it != args.cend(); ++it)
        {
            if (it != args.cbegin())
            {
                oss << " ";
            }
            std::string str = s["tostring"]((*it).get<sol::object>());
            oss << str;
        }
        Overlay::Get().Log(oss.str());
    };

    m_lua["GetAsyncKeyState"] = [](int aKeyCode) -> bool
    {
        return GetAsyncKeyState(aKeyCode) & 0x8000 != 0;
    };

    // execute autoexec.lua inside our default script directory
    std::filesystem::current_path(Options::Get().CETPath / "scripts");
    if (std::filesystem::exists("autoexec.lua"))
        m_lua.do_file("autoexec.lua");
    else
        Overlay::Get().Log("WARNING: missing CET autoexec.lua!");
    
    // execute autoexec.lua inside user script directory (NOTE: directory stays set afterwards to this for now)
    std::filesystem::current_path(Options::Get().ScriptsPath);
    if (std::filesystem::exists("autoexec.lua"))
        m_lua.do_file("autoexec.lua");
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
    property = aParam;
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
        Overlay::Get().Log("Type '" + acName + "' not found or is not initialized yet.");
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
        auto code = this->Execute(name, args, env, L, result);
        if(!code)
        {
            Overlay::Get().Log("Error: " + result);
        }
        return code;
    });

    return NewIndex(acName, std::move(obj));
}

sol::object Scripting::Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment env, sol::this_state L, std::string& aReturnMessage)
{
    auto* pRtti = RED4ext::CRTTISystem::Get();

    RED4ext::CBaseFunction* pFunc = pRtti->GetFunction(RED4ext::FNV1a(aFuncName.c_str()));

    static const auto hashcpPlayerSystem = RED4ext::FNV1a("cpPlayerSystem");
    static const auto hashGameInstance = RED4ext::FNV1a("ScriptGameInstance");

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

    if (pFunc->params.size - 1 != aArgs.size())
    {
        aReturnMessage = "Function '" + aFuncName + "' expects " +
            std::to_string(pFunc->params.size - 1) + " parameters, not " +
            std::to_string(aArgs.size()) + ".";

        return sol::nil;
    }

    const auto* engine = RED4ext::CGameEngine::Get();
    auto* unk10 = engine->framework->gameInstance;

    using CStackType = RED4ext::CStackType;
    std::vector<CStackType> args(aArgs.size() + 1);

    args[0].type = pFunc->params[0]->type;
    args[0].value = &unk10;

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

    for (auto i = 0ull; i < aArgs.size(); ++i)
    {
        args[i + 1ull] = ToRED(aArgs[i].get<sol::object>(), pFunc->params[i + 1ull]->type, &s_scratchMemory);

        if(!args[i + 1ull].value)
        {
            auto* pType = pFunc->params[i + 1]->type;

            RED4ext::CName name;
            pType->GetName(name);
            if (name)
            {
                const auto typeName = name.ToString();
                aReturnMessage = "Function '" + aFuncName + "' parameter " + std::to_string(i) + " must be " + typeName + ".";
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

    auto* pScriptable = static_cast<RED4ext::IScriptable*>(unk10->GetInstance(pPlayerSystem));

    RED4ext::CStack stack(pScriptable, args.data(), static_cast<uint32_t>(args.size()), hasReturnType ? &result : nullptr, 0);

    const auto success = pFunc->Execute(&stack);
    if (!success)
        return sol::nil;

    if(hasReturnType)
        return ToLua(m_lua, result);

    return make_object(m_lua, true);
}
