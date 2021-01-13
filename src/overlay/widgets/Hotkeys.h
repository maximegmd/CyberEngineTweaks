#pragma once

#include "Widget.h"

struct Hotkeys : public Widget
{
    Hotkeys() = default;
    ~Hotkeys() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Load();
    void Save();

private:
    std::vector<VKBindInfo> m_vkBindInfos{ };
};
