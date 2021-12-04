#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
    ScriptStore(LuaSandbox& aLuaSandbox, const Paths& aPaths, VKBindings& aBindings);
    ~ScriptStore() = default;

    void LoadAll();

    const TiltedPhoques::Vector<VKBindInfo>& GetBinds() const;
    
    void TriggerOnTweak() const;
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetMod(const std::string& acName) const;

private:
    
    TiltedPhoques::Map<std::string, ScriptContext> m_contexts{ };
    TiltedPhoques::Vector<VKBindInfo> m_vkBindInfos{ };
    LuaSandbox& m_sandbox;
    const Paths& m_paths;
    VKBindings& m_bindings;
};