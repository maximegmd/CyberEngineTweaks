#include <stdafx.h>

#include "Bindings.h"
#include "overlay/Overlay.h"

#include <scripting/LuaVM.h>

Bindings::Bindings(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm)
    : m_bindings(aBindings)
    , m_overlay(aOverlay)
    , m_vm(aVm)
    , m_overlayKeyID(m_overlay.GetBind().ID)
{
}

bool Bindings::OnEnable()
{
    if (!m_enabled)
    {
        m_bindings.StopRecordingBind();
        ResetChanges();
        m_enabled = true;
    }
    return m_enabled;
}

bool Bindings::OnDisable()
{
    if (m_enabled)
    {
        m_bindings.StopRecordingBind();
        m_vm.BlockDraw(m_madeChanges);
        m_madeChanges = (HelperWidgets::UnsavedChangesPopup(m_openChangesModal, m_madeChanges, m_saveCB, m_loadCB) == 0);
        m_vm.BlockDraw(m_madeChanges);
        m_enabled = m_madeChanges;
    }
    if (!m_enabled)
    {
        m_madeChanges = false;
    }
    return !m_enabled;
}

void Bindings::Update()
{
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Reset changes"))
        ResetChanges();

    ImGui::Spacing();

    if (!m_luaVMReady && m_vm.IsInitialized())
        Initialize();

    if (!m_luaVMReady)
        ImGui::TextUnformatted("LuaVM is not yet initialized!");
    else
    {
        m_madeChanges = false;
        for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
        {
            // transform mod name to nicer format until modinfo is in
            std::string activeModName = modBindingsIt.key() == "cet" ? "Cyber Engine Tweaks" : modBindingsIt.key();
            bool capitalize = true;
            std::ranges::transform(std::as_const(activeModName), activeModName.begin(), [&capitalize](char c) {
                if (!std::isalnum(c))
                {
                    capitalize = true;
                    return ' ';
                }
                if (capitalize)
                {
                    capitalize = false;
                    return static_cast<char>(std::toupper(c));
                }
                return c;
            });

            ImGui::TextUnformatted(activeModName.c_str());
            ImGui::Spacing();

            for (auto& binding : modBindingsIt.value())
            {
                if (binding.IsBinding && !m_bindings.IsRecordingBind())
                {
                    auto codeBind = m_bindings.GetLastRecordingResult();
                    if (codeBind != binding.CodeBind && m_bindings.IsFirstKeyUsed(codeBind))
                    {
                        // check for previous binding of this key/combo and unbind it (unless it is overlay key!)
                        auto modBindOpt = m_bindings.GetModBindForBindCode(codeBind);
                        if (modBindOpt)
                        {
                            const auto& modBind = (*modBindOpt).get();
                            if (modBind != m_overlay.GetModBind())
                            {
                                // unbind former
                                const auto& modName = modBind.ModName;
                                auto& bindInfos = m_vkBindInfos[modName];
                                const auto bindIt = std::ranges::find_if(bindInfos, [&id = modBind.ID](const auto& bindInfo) {
                                    return bindInfo.Bind.get().ID == id;
                                });
                                if (bindIt != bindInfos.cend())
                                {
                                    if (bindIt->Bind.get().IsHotkey() == binding.Bind.get().IsHotkey())
                                    {
                                        m_bindings.UnBind({modName, bindIt->Bind.get().ID});
                                        bindIt->CodeBind = 0;
                                    }
                                    else
                                    {
                                        // forbid rebind!
                                        codeBind = binding.CodeBind;
                                    }
                                }
                            }
                            else
                            {
                                // forbid rebind!
                                codeBind = binding.CodeBind;
                            }
                        }
                        else
                        {
                            modBindOpt = m_bindings.GetModBindStartingWithBindCode(binding.CodeBind);
                            if (modBindOpt)
                            {
                                // check if binding starting with same key is of the same type
                                const auto& modBind = (*modBindOpt).get();
                                const auto& modName = modBind.ModName;
                                auto& bindInfos = m_vkBindInfos[modName];
                                const auto bindIt = std::ranges::find_if(bindInfos, [&id = modBind.ID](const auto& bindInfo) {
                                    return bindInfo.Bind.get().ID == id;
                                });
                                if (bindIt != bindInfos.cend())
                                {
                                    if (bindIt->Bind.get().IsHotkey() != binding.Bind.get().IsHotkey())
                                    {
                                        // forbid rebind!
                                        codeBind = binding.CodeBind;
                                    }
                                }
                            }
                        }
                    }

                    if (codeBind != binding.CodeBind)
                    {
                        m_bindings.Bind(codeBind, {modBindingsIt.key(), binding.Bind.get().ID});
                        binding.CodeBind = codeBind;
                    }

                    binding.IsBinding = false;
                }

                const auto drawState = HelperWidgets::BindWidget(binding, 10.0f);
                if (drawState > 0)
                {
                    m_bindings.StartRecordingBind({modBindingsIt.key(), binding.Bind.get().ID});
                    binding.IsBinding = true;
                }
                else if (drawState < 0)
                {
                    if (binding.IsBinding)
                    {
                        m_bindings.StopRecordingBind();
                        binding.IsBinding = false;
                    }
                    m_bindings.UnBind({modBindingsIt.key(), binding.Bind.get().ID});
                    binding.CodeBind = 0;
                }

                m_madeChanges |= binding.CodeBind != binding.SavedCodeBind;
            }
        }
    }
}

void Bindings::Save()
{
    if (!m_vm.IsInitialized())
    {
        m_luaVMReady = false;
        return;
    }

    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (auto& binding : modBindingsIt.value())
        {
            if (binding.SavedCodeBind != binding.CodeBind)
                binding.SavedCodeBind = binding.CodeBind;
        }
    }

    m_bindings.Save();

    m_luaVMReady = true;
}

void Bindings::ResetChanges()
{
    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (auto& binding : modBindingsIt.value())
        {
            if (binding.SavedCodeBind != binding.CodeBind)
                binding.CodeBind = binding.SavedCodeBind;
        }
    }
}

void Bindings::Initialize()
{
    if (!m_vm.IsInitialized())
        return;

    const auto& allModsBinds = m_vm.GetAllBinds();

    m_vkBindInfos.clear();

    // emplace CET internal settings
    {
        auto& vkBindInfo =m_vkBindInfos[m_overlay.GetModBind().ModName].emplace_back(m_overlay.GetBindInfo());
        vkBindInfo.SavedCodeBind = m_bindings.GetBindCodeForModBind(m_overlay.GetModBind(), false);
        vkBindInfo.CodeBind = vkBindInfo.SavedCodeBind;
    }

    // emplace mod bindings
    for (const auto& modBindsIt : allModsBinds)
    {
        for (const auto& vkBind : modBindsIt.second.get())
        {
            auto& vkBindInfo = m_vkBindInfos[modBindsIt.first].emplace_back(vkBind);
            vkBindInfo.SavedCodeBind = m_bindings.GetBindCodeForModBind({modBindsIt.first, vkBind.ID}, false);
            vkBindInfo.CodeBind = vkBindInfo.SavedCodeBind;
        }
    }

    m_luaVMReady = true;
}
