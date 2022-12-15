#include <stdafx.h>

#include "Scripting.h"

#include "FunctionOverride.h"
#include "GameOptions.h"
#include "Texture.h"

#include "EngineTweaks.h"
#include <lsqlite3/lsqlite3.h>
#include <reverse/Type.h>
#include <reverse/RTTIHelper.h>
#include <sol_imgui/sol_imgui.h>
#include <Utils.h>

static constexpr bool s_cThrowLuaErrors = true;

Scripting::Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12)
    : m_sandbox(this, aBindings)
    , m_mapper(m_lua.AsRef(), m_sandbox)
    , m_store(m_sandbox, aPaths, aBindings)
#if GAME_CYBERPUNK
    , m_override(this)
#endif
    , m_paths(aPaths)
    , m_d3d12(aD3D12)
{
    CreateLogger(aPaths.CETRoot() / "scripting.log", "scripting");
    CreateLogger(aPaths.CETRoot() / "gamelog.log", "gamelog");
}

void Scripting::Initialize()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();

    luaVm.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table, sol::lib::bit32);
    luaVm.require("sqlite3", luaopen_lsqlite3);

    // make sure to set package path to current directory scope
    // as this could get overriden by LUA_PATH environment variable
    luaVm["package"]["path"] = "./?.lua";

    // execute autoexec.lua inside our default script directory
    const auto previousCurrentPath = std::filesystem::current_path();
    current_path(m_paths.CETRoot() / "scripts");
    luaVm.script("json = require 'json/json'", sol::detail::default_chunk_name(), sol::load_mode::text);
    luaVm.script("IconGlyphs = require 'IconGlyphs/icons'", sol::detail::default_chunk_name(), sol::load_mode::text);
    current_path(previousCurrentPath);

    // initialize sandbox
    m_sandbox.Initialize();

    auto& globals = m_sandbox.GetGlobals();

    // load in imgui bindings
    sol_ImGui::InitBindings(luaVm, globals);
    sol::table imgui = globals["ImGui"];
    Texture::BindTexture(imgui);
    for (auto [key, value] : imgui)
    {
        if (value.get_type() != sol::type::function)
            continue;

        sol::function original = value;
        imgui[key] = make_object(
            luaVm,
            [this, original](sol::variadic_args aVariadicArgs, sol::this_environment aThisEnv) -> sol::variadic_results
            {
                const sol::environment cEnv = aThisEnv;
                const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
                if (!m_sandbox.GetImGuiAvailable())
                {
                    logger->error("Tried to call ImGui from invalid event!");
                    throw "Tried to call ImGui from invalid event!";
                }

                return original(as_args(aVariadicArgs));
            });
    }

    // setup logger for console sandbox
    auto& consoleSB = m_sandbox[0];
    auto& consoleSBEnv = consoleSB.GetEnvironment();
    consoleSBEnv["__logger"] = spdlog::get("scripting");

    // load in game bindings
    globals["print"] = [](sol::variadic_args aArgs, sol::this_state aState)
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

        spdlog::get("scripting")->info(oss.str());
    };

    globals["GetVersion"] = []() -> std::string
    {
        return CET_BUILD_COMMIT;
    };

    globals["GetDisplayResolution"] = [this]() -> std::tuple<float, float>
    {
        const auto resolution = m_d3d12.GetResolution();
        return {static_cast<float>(resolution.cx), static_cast<float>(resolution.cy)};
    };

    // load mods
    m_store.LoadAll();
}

const VKBind* Scripting::GetBind(const VKModBind& acModBind) const
{
    return m_store.GetBind(acModBind);
}

const TiltedPhoques::Vector<VKBind>* Scripting::GetBinds(const std::string& acModName) const
{
    return m_store.GetBinds(acModName);
}

const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& Scripting::GetAllBinds() const
{
    return m_store.GetAllBinds();
}

void Scripting::TriggerOnHook() const
{
    m_store.TriggerOnHook();
}

void Scripting::TriggerOnTweak() const
{
    m_store.TriggerOnTweak();
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

void Scripting::TriggerOnOverlayOpen() const
{
    m_store.TriggerOnOverlayOpen();
}

void Scripting::TriggerOnOverlayClose() const
{
    m_store.TriggerOnOverlayClose();
}

sol::object Scripting::GetMod(const std::string& acName) const
{
    return m_store.GetMod(acName);
}

void Scripting::UnloadAllMods()
{
#if GAME_CYBERPUNK
    m_override.Clear();
#endif
    RegisterOverrides();
    m_store.DiscardAll();
}

void Scripting::ReloadAllMods()
{
    UnloadAllMods();

    m_store.LoadAll();

    TriggerOnHook();
    TriggerOnTweak();
    TriggerOnInit();

    if (EngineTweaks::Get().GetOverlay().IsEnabled())
        TriggerOnOverlayOpen();

    spdlog::get("scripting")->info("LuaVM: Reloaded all mods!");
}

bool Scripting::ExecuteLua(const std::string& acCommand) const
{
    // TODO: proper exception handling!
    try
    {
        auto lockedState = m_lua.Lock();
        const auto cResult = m_sandbox[0].ExecuteString(acCommand);
        if (cResult.valid())
            return true;
        const sol::error cError = cResult;
        spdlog::get("scripting")->error(cError.what());
    }
    catch (std::exception& e)
    {
        spdlog::get("scripting")->error(e.what());
    }
    return false;
}

void Scripting::CollectGarbage() const
{
    auto lockedState = m_lua.Lock();
    auto& luaState = lockedState.Get();

    luaState.collect_garbage();
}

LuaSandbox& Scripting::GetSandbox()
{
    return m_sandbox;
}

Scripting::LockedState Scripting::GetLockedState() const noexcept
{
    return m_lua.Lock();
}

sol::object Scripting::Index(const std::string& acName, sol::this_state aState, sol::this_environment aEnv)
{
    if (const auto itor = m_properties.find(acName); itor != m_properties.end())
    {
        return itor->second;
    }

    const auto func = RTTIHelper::Get().ResolveFunction(acName);

    if (!func)
    {
        std::string errorMessage = fmt::format("Function {} is not a GameInstance member and is not a global.", acName);

        if constexpr (s_cThrowLuaErrors)
        {
            luaL_error(aState, errorMessage.c_str());
        }
        else
        {
            const sol::environment cEnv = aEnv;
            auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Error: {}", errorMessage);
        }

        return sol::nil;
    }

    auto& property = m_properties[acName];
    property = func;
    return property;
}
