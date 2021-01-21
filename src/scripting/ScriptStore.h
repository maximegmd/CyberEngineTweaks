#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
    ScriptStore(const Paths& aPaths, VKBindings& aBindings);
    ~ScriptStore() = default;

    void LoadAll(sol::state_view aStateView);

    const std::vector<VKBindInfo>& GetBinds() const;
    
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetMod(const std::string& acName) const;

private:
    
    std::unordered_map<std::string, ScriptContext> m_contexts{ };
    std::vector<VKBindInfo> m_vkBindInfos{ };
    const Paths& m_paths;
    VKBindings& m_bindings;
};