#include <stdafx.h>

#include "ScriptStore.h"
#include "Utils.h"

ScriptStore::ScriptStore(LuaSandbox& aLuaSandbox, const Paths& aPaths, VKBindings& aBindings)
    : m_sandbox(aLuaSandbox)
    , m_paths(aPaths)
    , m_bindings(aBindings)
{
}

void ScriptStore::LoadAll()
{
    // set current path for following scripts to our ModsPath
    current_path(m_paths.ModsRoot());

    m_vkBindInfos.clear();
    m_contexts.clear();
    m_sandbox.ResetState();

    auto consoleLogger = spdlog::get("scripting");

    const auto& cModsRoot = m_paths.ModsRoot();
    for (const auto& file : std::filesystem::directory_iterator(cModsRoot))
    {
        if (!file.is_directory())
            continue;

        auto fPath = file.path();

        try
        {
            if (is_symlink(fPath))
                fPath = read_symlink(fPath);
            else if (is_symlink(fPath / "init.lua"))
                fPath = read_symlink(fPath / "init.lua").parent_path();
        }
        catch (std::exception& e)
        {
        }

        fPath = absolute(fPath);
        auto fPathStr = UTF16ToUTF8(fPath.native());

        if (!exists(fPath / "init.lua"))
        {
            consoleLogger->warn("Ignoring directory that does not contain init.lua! ('{}')", fPathStr);
            continue;
        }

        auto name = UTF16ToUTF8(file.path().filename().native());
        if (name.find('.') != std::string::npos)
        {
            consoleLogger->info("Ignoring directory containing '.', as this is reserved character! ('{}')", fPathStr);
            continue;
        }

        auto ctx = ScriptContext{m_sandbox, fPath, name};
        if (ctx.IsValid())
        {
            auto& ctxBinds = ctx.GetBinds();
            m_vkBindInfos.insert(m_vkBindInfos.cend(), ctxBinds.cbegin(), ctxBinds.cend());
            m_contexts.emplace(name, std::move(ctx));
            consoleLogger->info("Mod {} loaded! ('{}')", name, fPathStr);
        }
        else
        {
            consoleLogger->error("Mod {} failed to load! ('{}')", name, fPathStr);
        }
    }

    m_bindings.InitializeMods(m_vkBindInfos);
}

const TiltedPhoques::Vector<VKBindInfo>& ScriptStore::GetBinds() const
{
    return m_vkBindInfos;
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
    const auto itor = m_contexts.find(acName);
    if (itor != std::end(m_contexts))
        return itor->second.GetRootObject();

    return sol::nil;
}
