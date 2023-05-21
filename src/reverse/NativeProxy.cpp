#include <stdafx.h>

#include "NativeProxy.h"
#include <scripting/Scripting.h>

namespace
{
constexpr auto ProxyPrefix = "$LuaProxy$";
constexpr auto NativeHandler = "Handler";
constexpr auto DefaultFunction = "Callback";
constexpr auto CallStaticOp = 36;
constexpr auto ParamEndOp = 38;
constexpr auto OpSize = sizeof(char);
constexpr auto OffsetSize = sizeof(uint16_t);
constexpr auto FlagsSize = sizeof(uint16_t);
constexpr auto PointerSize = sizeof(void*);
constexpr auto FunctionSize = OpSize + OffsetSize * 2 + PointerSize + FlagsSize + OpSize;
constexpr auto ExitOffset = FunctionSize - OpSize - OffsetSize;

TiltedPhoques::Map<RED4ext::CName, TiltedPhoques::UniquePtr<NativeProxyType>> s_proxies;
RED4ext::CClassFunction* s_nativeHandler;
char s_scriptHandlerCode[FunctionSize];
bool s_rebuildRuntimeData;
}

NativeProxy::NativeProxy(NativeProxy::LockableState aLua, sol::environment aEnvironment, const sol::object& aSpec)
    : NativeProxy(std::move(aLua), std::move(aEnvironment), "IScriptable", aSpec)
{
}

NativeProxy::NativeProxy(NativeProxy::LockableState aLua, sol::environment aEnvironment, const std::string& aInterface,
                         const sol::object& aSpec)
    : m_lua(std::move(aLua))
    , m_environment(std::move(aEnvironment))
{
    auto pProxy = NativeProxyType::ResolveProxy(aInterface);

    if (pProxy)
    {
        m_target = pProxy->CreateHandle();
        *reinterpret_cast<NativeProxy**>(m_target->valueHolder) = this;

        if (!AddFunction(pProxy, DefaultFunction, aSpec))
        {
            if (aSpec.get_type() == sol::type::table)
            {
                for (const auto& item : aSpec.as<sol::table>())
                {
                    if (item.first.get_type() == sol::type::string)
                    {
                        AddFunction(pProxy, item.first.as<std::string>(), item.second);
                    }
                }
            }
        }
    }
}

NativeProxy::NativeProxy(NativeProxy&& aOther) noexcept
    : m_lua(std::move(aOther.m_lua))
    , m_target(std::move(aOther.m_target))
    , m_functions(std::move(aOther.m_functions))
    , m_signatures(std::move(aOther.m_signatures))
    , m_environment(std::move(aOther.m_environment))
{
    if (m_target)
    {
        *reinterpret_cast<NativeProxy**>(m_target->valueHolder) = this;
    }
}

bool NativeProxy::AddFunction(NativeProxyType* apProxy, const std::string& aName, const sol::object& aSpec)
{
    switch (aSpec.get_type())
    {
    case sol::type::function:
    {
        const auto signature = NativeProxyType::AddSignature(apProxy, aName, {});

        if (signature)
        {
            m_signatures[aName.c_str()] = signature;
            m_functions[signature] = aSpec.as<sol::protected_function>();
            return true;
        }
        break;
    }
    case sol::type::table:
    {
        const auto& table = aSpec.as<sol::table>();
        const auto& func = table["callback"];

        if (func.get_type() == sol::type::function)
        {
            const auto& args = table["args"];
            const auto signature = NativeProxyType::AddSignature(apProxy, aName, args.get_or<sol::table>({}));

            if (signature)
            {
                m_signatures[aName.c_str()] = signature;
                m_functions[signature] = func.get<sol::protected_function>();
                return true;
            }
        }
        break;
    }
    }

    return false;
}

WeakReference NativeProxy::GetTarget() const
{
    if (s_rebuildRuntimeData)
    {
        RED4ext::CRTTISystem::Get()->InitializeScriptRuntime();
        s_rebuildRuntimeData = false;
    }

    auto lockedState = m_lua.Lock();
    return {m_lua, m_target};
}

CName NativeProxy::GetFunction(const std::string& aName) const
{
    if (s_rebuildRuntimeData)
    {
        RED4ext::CRTTISystem::Get()->InitializeScriptRuntime();
        s_rebuildRuntimeData = false;
    }

    auto lockedState = m_lua.Lock();

    const auto signatureIt = m_signatures.find(aName.c_str());
    if (signatureIt == m_signatures.end())
        return {};

    return signatureIt.value().hash;
}

CName NativeProxy::GetDefaultFunction() const
{
    return GetFunction(DefaultFunction);
}

void NativeProxy::Callback(RED4ext::IScriptable* apSelf, RED4ext::CStackFrame* apFrame, void* apOut, int64_t)
{
    ++apFrame->code;

    if (!apFrame->func || !apSelf->valueHolder)
        return;

    auto self = *reinterpret_cast<NativeProxy**>(apSelf->valueHolder);

    const auto signature = apFrame->func->fullName;
    const auto functionIt = self->m_functions.find(signature);

    if (functionIt == self->m_functions.end())
        return;

    TiltedPhoques::StackAllocator<1 << 13> allocator;
    const auto pAllocator = TiltedPhoques::Allocator::Get();
    TiltedPhoques::Allocator::Set(&allocator);
    TiltedPhoques::Vector<sol::object> args(0);
    TiltedPhoques::Allocator::Set(pAllocator);

    auto lockedState = self->m_lua.Lock();
    auto& luaState = lockedState.Get();

    for (const auto& param : apFrame->func->params)
    {
        RED4ext::CStackType arg;
        arg.type = param->type;
        arg.value = param->GetValuePtr<void>(apFrame->params);

        args.emplace_back(Scripting::ToLua(lockedState, arg));
    }

    const auto result = functionIt.value()(sol::as_args(args));

    if (!result.valid())
    {
        const auto logger = self->m_environment["__logger"].get<std::shared_ptr<spdlog::logger>>();
        logger->error(result.get<sol::error>().what());
    }
}

NativeProxyType::NativeProxyType(RED4ext::CClass* apBase)
    : CClass("", sizeof(NativeProxy), {.isNative = true})
{
    std::string proxyName = ProxyPrefix;
    proxyName += apBase->name.ToString();

    name = RED4ext::CNamePool::Add(proxyName.c_str());
    parent = apBase;
}

NativeProxyType* NativeProxyType::ResolveProxy(const std::string& aBase)
{
    auto pRtti = RED4ext::CRTTISystem::Get();
    auto pBase = pRtti->GetClassByScriptName(aBase.c_str());

    if (!pBase)
        return nullptr;

    const auto typeIt = s_proxies.find(pBase->name);

    if (typeIt != s_proxies.end())
        return typeIt.value().get();

    auto pScriptable = pRtti->GetClass("IScriptable");

    if (!pBase->IsA(pScriptable))
        return nullptr;

    auto pProxy = TiltedPhoques::MakeUnique<NativeProxyType>(pBase);
    pRtti->RegisterType(pProxy.get());

    auto pUint64 = pRtti->GetType("Uint64");
    auto pSelfProp = RED4ext::CProperty::Create(pUint64, "self", nullptr, 0, nullptr, {.isScripted = true});
    pProxy->props.PushBack(pSelfProp);

    if (!s_nativeHandler)
    {
        s_nativeHandler = RED4ext::CClassFunction::Create(pProxy.get(), NativeHandler, NativeHandler,
                                                          &NativeProxy::Callback);
        s_nativeHandler->flags.isEvent = true;

        auto* pCode = s_scriptHandlerCode;

        *pCode = CallStaticOp;
        pCode += OpSize;

        *reinterpret_cast<uint16_t*>(pCode) = ExitOffset;
        pCode += OffsetSize;

        *reinterpret_cast<uint16_t*>(pCode) = 0;
        pCode += OffsetSize;

        *reinterpret_cast<void**>(pCode) = s_nativeHandler;
        pCode += PointerSize;

        *reinterpret_cast<uint16_t*>(pCode) = 0;
        pCode += FlagsSize;

        *pCode = ParamEndOp;
        pCode += OpSize;
    }

    pProxy->RegisterFunction(s_nativeHandler);

    return s_proxies.emplace(pBase->name, std::move(pProxy)).first.value().get();
}

RED4ext::CName NativeProxyType::AddSignature(NativeProxyType* apProxy, const std::string& aName,
                                             const sol::table& aArgs)
{
    auto pRtti = RED4ext::CRTTISystem::Get();

    std::string fullName = aName + ';';
    std::vector<RED4ext::CName> argTypes;
    argTypes.reserve(aArgs.size());

    for (const auto& item : aArgs)
    {
        if (item.second.get_type() != sol::type::string)
            return {};

        auto argTypeName = item.second.as<std::string>();

        if (argTypeName.empty())
            return {};

        auto argTypeHash = RED4ext::CNamePool::Add(argTypeName.c_str());
        auto pArgType = pRtti->GetType(argTypeHash);

        if (pArgType)
        {
            argTypeName = FormatTypeName(pRtti, pArgType);
        }
        else
        {
            pArgType = pRtti->GetClassByScriptName(argTypeHash);

            if (!pArgType)
                return {};
        }

        fullName += argTypeName;
        argTypes.emplace_back(pArgType->GetName());
    }

    auto pFunc = apProxy->GetFunction(fullName.c_str());

    if (pFunc)
        return pFunc->fullName;

    pFunc = RED4ext::CClassFunction::Create(apProxy, fullName.c_str(), aName.c_str(),
        reinterpret_cast<RED4ext::ScriptingFunction_t<void*>>(&NativeProxy::Callback));
    pFunc->bytecode.bytecode.buffer.data = s_scriptHandlerCode;
    pFunc->bytecode.bytecode.buffer.size = FunctionSize;
    pFunc->flags.isNative = false;
    pFunc->flags.isEvent = true;

    for (const auto& argType : argTypes)
    {
        pFunc->AddParam(argType, "arg");
    }

    apProxy->RegisterFunction(pFunc);

    using FlagsIntType = uint32_t;
    constexpr auto CustomFlag = 1 << (8 * sizeof(FlagsIntType) - 1);
    static_assert(sizeof(RED4ext::CBaseFunction::Flags) == sizeof(FlagsIntType));

    // Tell Hot Reload to not touch this quasi-scripted function
    *reinterpret_cast<FlagsIntType*>(&pFunc->flags) |= CustomFlag;

    s_rebuildRuntimeData = true;

    return pFunc->fullName;
}

std::string NativeProxyType::FormatTypeName(RED4ext::CRTTISystem* apRtti, RED4ext::CBaseRTTIType* apType)
{
    switch (apType->GetType())
    {
    case RED4ext::ERTTIType::Class:
    {
        return apRtti->ConvertNativeToScriptName(apType->GetName()).ToString();
    }
    case RED4ext::ERTTIType::Handle:
    case RED4ext::ERTTIType::WeakHandle:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIHandleType*>(apType)->innerType;
        return FormatTypeName(apRtti, pInnerType);
    }
    case RED4ext::ERTTIType::Array:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIArrayType*>(apType)->innerType;
        return "array<" + FormatTypeName(apRtti, pInnerType) + ">";
    }
    case RED4ext::ERTTIType::ScriptReference:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIScriptReferenceType*>(apType)->innerType;
        return "script_ref<" + FormatTypeName(apRtti, pInnerType) + ">";
    }
    default: return apType->GetName().ToString();
    }
}

RED4ext::Handle<RED4ext::IScriptable> NativeProxyType::CreateHandle()
{
    auto pInstace = CreateInstance(true);
    auto pScriptable = reinterpret_cast<RED4ext::IScriptable*>(pInstace);

    return RED4ext::Handle<RED4ext::IScriptable>(pScriptable);
}

const bool NativeProxyType::IsEqual(const RED4ext::ScriptInstance aLhs, const RED4ext::ScriptInstance aRhs, uint32_t a3)
{
    return !parent->flags.isAbstract && parent->IsEqual(aLhs, aRhs);
}

void NativeProxyType::Assign(RED4ext::ScriptInstance aLhs, const RED4ext::ScriptInstance aRhs) const
{
    if (!parent->flags.isAbstract)
    {
        parent->Assign(aLhs, aRhs);
    }
}

void NativeProxyType::ConstructCls(RED4ext::ScriptInstance aMemory) const
{
    if (!parent->flags.isAbstract)
    {
        parent->ConstructCls(aMemory);
    }
}

void NativeProxyType::DestructCls(RED4ext::ScriptInstance aMemory) const
{
    if (!parent->flags.isAbstract)
    {
        parent->DestructCls(aMemory);
    }
}

void* NativeProxyType::AllocMemory() const
{
    auto classAlignment = GetAlignment();
    auto alignedSize = RED4ext::AlignUp(GetSize(), classAlignment);

    auto allocator = GetAllocator();
    auto allocResult = allocator->AllocAligned(alignedSize, classAlignment);

    std::memset(allocResult.memory, 0, allocResult.size);
    return allocResult.memory;
}
