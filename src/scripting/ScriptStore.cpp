#include <stdafx.h>

#include "ScriptStore.h"

#include <CET.h>

#include "Utils.h"

ScriptStore::ScriptStore(LuaSandbox& aLuaSandbox, const Paths& aPaths, VKBindings& aBindings)
    : m_sandbox(aLuaSandbox)
    , m_paths(aPaths)
    , m_bindings(aBindings)
{
}

void ScriptStore::LoadAll()
{
    m_vkBinds.clear();
    m_contexts.clear();
    m_sandbox.ResetState();

    const auto consoleLogger = spdlog::get("scripting");

    const auto& cModsRoot = m_paths.ModsRoot();
    for (const auto& file : std::filesystem::directory_iterator(cModsRoot))
    {
        if (!file.is_directory())
            continue;

        const auto name = UTF16ToUTF8(file.path().filename().native());
        const auto pathStr = UTF16ToUTF8((cModsRoot / file.path()).native());

        if (file.path().native().starts_with(L"cet\\"))
        {
            consoleLogger->warn("Ignoring mod using reserved name! ('{}')", pathStr);
            continue;
        }

        const auto path = GetAbsolutePath(file.path(), cModsRoot, false);
        if (path.empty())
        {
            consoleLogger->error("Tried to access invalid directory! ('{}')", pathStr);
            continue;
        }

        if (!exists(path / L"init.lua"))
        {
            consoleLogger->warn("Ignoring mod which does not contain init.lua! ('{}')", pathStr);
            continue;
        }

        auto ctx = ScriptContext{m_sandbox, path, name};
        if (ctx.IsValid())
        {
            m_contexts.emplace(name, std::move(ctx));
            consoleLogger->info("Mod {} loaded! ('{}')", name, pathStr);
        }
        else
            consoleLogger->error("Mod {} failed to load! ('{}')", name, pathStr);
    }

    for (auto& contextIt : m_contexts)
        m_vkBinds.insert({contextIt.first, contextIt.second.GetBinds()});

    m_bindings.InitializeMods(m_vkBinds);
}

const VKBind* ScriptStore::GetBind(const VKModBind& acModBind) const
{
    if (acModBind == Bindings::GetOverlayToggleModBind())
        return &Bindings::GetOverlayToggleBind();

    const auto it = m_contexts.find(acModBind.ModName);
    if (it != m_contexts.cend())
        return it->second.GetBind(acModBind.ID);

    return nullptr;
}

const TiltedPhoques::Vector<VKBind>* ScriptStore::GetBinds(const std::string& acModName) const
{
    const auto it = m_contexts.find(acModName);
    if (it != m_contexts.cend())
    {
        return &it->second.GetBinds();
    }

    return nullptr;
}

const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& ScriptStore::GetAllBinds() const
{
    return m_vkBinds;
}

void ScriptStore::TriggerOnTweak() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnTweak();
}

void ScriptStore::TriggerOnInit() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnInit();
}

void ScriptStore::TriggerOnUpdate(float aDeltaTime) const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnUpdate(aDeltaTime);
}

void ScriptStore::TriggerOnDraw() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnDraw();
}

void ScriptStore::TriggerOnOverlayOpen() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnOverlayOpen();
}

void ScriptStore::TriggerOnOverlayClose() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnOverlayClose();
}

sol::object ScriptStore::GetMod(const std::string& acName) const
{
    const auto it = m_contexts.find(acName);
    if (it != m_contexts.cend())
        return it->second.GetRootObject();

    return sol::nil;
}
