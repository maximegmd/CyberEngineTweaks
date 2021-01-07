#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
    ScriptStore() = default;
    ~ScriptStore() = default;

    void LoadAll(sol::state_view aStateView);
    
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnConsoleOpen() const;
    void TriggerOnConsoleClose() const;

    sol::object GetMod(const std::string& acName) const;

private:
    
    std::unordered_map<std::string, ScriptContext> m_contexts{ };
};