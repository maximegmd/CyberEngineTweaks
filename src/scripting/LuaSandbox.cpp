#include <stdafx.h>

#include "LuaSandbox.h"

#include "Texture.h"

#include "EngineTweaks.h"
#include <Utils.h>

static constexpr const char* s_cVMGlobalObjectsWhitelist[] = {
    "_VERSION",
    "assert",
    "error",
    "getmetatable", //< Used to extend string class
    "ipairs",
    "next",
    "pairs",
    "pcall",

    // Required for implementing classes
    "rawequal",
    "rawget",
    "rawlen",
    "rawset",

    "select",
    "setmetatable", //< Required for implementing classes
    "tonumber",
    "tostring",
    "type",
    "unpack",
    "xpcall",

    "collectgarbage", //< Good for testing memory leaks and ref counters: `collectgarbage("collect")`, also used for
                      // forcing the release of some refs

    "string",
    "table",
    "math",
    "bit32",
    "json",
    "IconGlyphs",
};

static constexpr const char* s_cGlobalObjectsWhitelist[] = {
    "print",
    "GetVersion",
    "GetDisplayResolution",
    "ModArchiveExists",

    "ImGuiListClipper",
    "ImGuiCond",
    "ImGuiTreeNodeFlags",
    "ImGuiSelectableFlags",
    "ImGuiInputTextFlags",
    "ImGuiColorEditFlags",
    "ImGuiComboFlags",
    "ImGuiHoveredFlags",
    "ImGuiFocusedFlags",
    "ImGuiPopupFlags",
    "ImGuiTabItemFlags",
    "ImGuiWindowFlags",
    "ImGuiStyleVar",
    "ImGuiTabBarFlags",
    "ImGuiSliderFlags",
    "ImGuiTableFlags",
    "ImGuiTableColumnFlags",
    "ImGuiTableRowFlags",
    "ImGuiTableBgTarget",
    "ImGuiMouseButton",
    "ImDrawCornerFlags",
    "ImDrawFlags",
    "ImGuiCol",
    "ImGuiDir",
    "ImVec2",
    "ImVec4"};

static constexpr const char* s_cPostInitializeScriptingProtectedList[] = {
    // initialized by Scripting
    "Descriptor",     "StrongReference",
    "WeakReference",  "SingletonReference",
    "ClassReference", "ResourceAsyncReference",
    "Unknown",        "IsDefined",
    "Enum",           "EnumInt",
    "Vector3",        "ToVector3",
    "Vector4",        "ToVector4",
    "EulerAngles",    "ToEulerAngles",
    "Quaternion",     "ToQuaternion",
    "CName",          "ToCName",
    "TweakDBID",      "ToTweakDBID",
    "ItemID",         "ToItemID",
    "CRUID",          "LocKey",
    "GameOptions",    "Override",
    "ObserveBefore",  "ObserveAfter",
    "Observe",
};

static constexpr const char* s_cPostInitializeTweakDBProtectedList[] = {
    // initialized by Scripting
    "TweakDB"};

static constexpr const char* s_cPostInitializeModsProtectedList[] = {
    // initialized by Scripting
    "NewObject", "GetSingleton", "GetMod", "GameDump", "Dump", "DumpType", "DumpAllTypeNames", "DumpVtables", "DumpReflection", "Game",

    // initialized by RTTIMapper
    "Vector3", "ToVector3", "Vector4", "ToVector4", "EulerAngles", "ToEulerAngles", "Quaternion", "ToQuaternion", "ItemID", "ToItemID"};

static constexpr const char* s_cGlobalExtraLibsWhitelist[] = {
    "ImGui",
};

LuaSandbox::LuaSandbox(Scripting* apScripting, const VKBindings& acVKBindings)
    : m_pScripting(apScripting)
    , m_vkBindings(acVKBindings)
{
}

void LuaSandbox::Initialize()
{
    auto lockedState = m_pScripting->GetLockedState();
    auto& luaState = lockedState.Get();
    auto globals = luaState.globals();

    // initialize state + environment first
    m_globals = {luaState, sol::create};

    // copy whitelisted things from global table
    for (const auto* cKey : s_cVMGlobalObjectsWhitelist)
    {
        m_globals[cKey] = DeepCopySolObject(globals[cKey].get<sol::object>(), luaState);
        MakeSolUsertypeImmutable(m_globals[cKey], luaState);
    }

    // copy whitelisted from our global table
    for (const auto* cKey : s_cGlobalObjectsWhitelist)
        MakeSolUsertypeImmutable(m_globals[cKey], luaState);

    // copy safe os functions
    {
        auto os = globals["os"].get<sol::table>();
        sol::table osCopy(luaState, sol::create);
        osCopy["clock"] = os["clock"];
        osCopy["date"] = os["date"];
        osCopy["difftime"] = os["difftime"];
        osCopy["time"] = os["time"];
        m_globals["os"] = osCopy;
    }

    CreateSandbox("", "", false, false, false, false);
}

void LuaSandbox::PostInitializeScripting()
{
    auto lockedState = m_pScripting->GetLockedState();
    auto& luaState = lockedState.Get();
    auto globals = luaState.globals();

    // copy whitelisted from global table
    for (const auto* cKey : s_cPostInitializeScriptingProtectedList)
        MakeSolUsertypeImmutable(m_globals[cKey], luaState);
}

void LuaSandbox::PostInitializeTweakDB()
{
    auto lockedState = m_pScripting->GetLockedState();
    auto& luaState = lockedState.Get();
    auto globals = luaState.globals();

    // copy whitelisted from global table
    for (const auto* cKey : s_cPostInitializeTweakDBProtectedList)
        MakeSolUsertypeImmutable(m_globals[cKey], luaState);
}

void LuaSandbox::PostInitializeMods()
{
    auto lockedState = m_pScripting->GetLockedState();
    auto& luaState = lockedState.Get();
    auto globals = luaState.globals();

    // copy whitelisted from global table
    for (const auto* cKey : s_cPostInitializeModsProtectedList)
        MakeSolUsertypeImmutable(m_globals[cKey], luaState);
}

void LuaSandbox::ResetState()
{
    if (m_modules.empty())
        return;

    auto lockedState = m_pScripting->GetLockedState();
    auto& luaState = lockedState.Get();

    for (size_t i = 1; i < m_sandboxes.size(); ++i)
        CloseDBForSandbox(m_sandboxes[i]);

    m_modules.clear();

    m_sandboxes.erase(m_sandboxes.cbegin() + 1, m_sandboxes.cend());

    luaState.collect_garbage();
}

uint64_t LuaSandbox::CreateSandbox(const std::filesystem::path& acPath, const std::string& acName, bool aEnableExtraLibs, bool aEnableDB, bool aEnableIO, bool aEnableLogger)
{
    const uint64_t cResID = m_sandboxes.size();
    assert(!cResID || (!acPath.empty() && !acName.empty()));

    auto lockedState = m_pScripting->GetLockedState();
    const auto& luaState = lockedState.Get();

    auto& res = m_sandboxes.emplace_back(cResID, m_pScripting, m_globals, acPath);
    if (!acPath.empty() && !acName.empty())
    {
        if (aEnableExtraLibs)
            InitializeExtraLibsForSandbox(res, luaState);
        if (aEnableDB)
            InitializeDBForSandbox(res, luaState);
        if (aEnableIO)
            InitializeIOForSandbox(res, luaState, acName);
        if (aEnableLogger)
            InitializeLoggerForSandbox(res, luaState, acName);
    }
    return cResID;
}

Sandbox& LuaSandbox::operator[](uint64_t aID)
{
    assert(aID < m_sandboxes.size());
    return m_sandboxes[aID];
}

const Sandbox& LuaSandbox::operator[](uint64_t aID) const
{
    assert(aID < m_sandboxes.size());
    return m_sandboxes[aID];
}

TiltedPhoques::Locked<sol::state, std::recursive_mutex> LuaSandbox::GetLockedState() const
{
    return m_pScripting->GetLockedState();
}

void LuaSandbox::SetImGuiAvailable(bool aAvailable)
{
    auto lockedState = m_pScripting->GetLockedState();
    m_imguiAvailable = aAvailable;
}

bool LuaSandbox::GetImGuiAvailable() const
{
    auto lockedState = m_pScripting->GetLockedState();
    return m_imguiAvailable;
}

sol::table& LuaSandbox::GetGlobals()
{
    return m_globals;
}

void LuaSandbox::InitializeExtraLibsForSandbox(Sandbox& aSandbox, const sol::state& acpState) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();
    auto globals = acpState.globals();
    sol::state_view stateView = acpState;

    // copy extra whitelisted libs from global table
    for (const auto* cKey : s_cGlobalExtraLibsWhitelist)
    {
        sbEnv[cKey] = DeepCopySolObject(globals[cKey].get<sol::object>(), acpState);
        MakeSolUsertypeImmutable(sbEnv[cKey], acpState);
    }

    sol::table imgui = sbEnv["ImGui"];

    imgui["LoadTexture"] = [this, cSBRootPath, stateView](const std::string& acPath) -> std::tuple<std::shared_ptr<Texture>, sol::object>
    {
        if (!GetImGuiAvailable())
            return std::make_tuple(nullptr, make_object(stateView, "Tried to use ImGui outside of allowed events!"));

        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        const auto path = GetLuaPath(acPath, cSBRootPath, false);
        auto texture = Texture::Load(UTF16ToUTF8(path.native()));
        if (!texture)
            return std::make_tuple(nullptr, make_object(stateView, fmt::format("Failed to load '{}'", acPath)));

        current_path(previousCurrentPath);

        return std::make_tuple(texture, sol::nil);
    };
}

void LuaSandbox::InitializeDBForSandbox(Sandbox& aSandbox, const sol::state& acpState)
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();
    const auto sbId = aSandbox.GetId();
    auto globals = acpState.globals();

    const auto cSQLite3 = globals["sqlite3"].get<sol::table>();
    sol::table sqlite3Copy(acpState, sol::create);
    for (const auto& cKV : cSQLite3)
    {
        const auto cKeyStr = cKV.first.as<std::string>();
        if (cKeyStr.compare(0, 4, "open"))
            sqlite3Copy[cKV.first] = DeepCopySolObject(cKV.second, acpState);
    }
    const auto dbOpen = acpState["sqlite3"]["open"].get<sol::function>();
    sqlite3Copy["reopen"] = [this, sbId, dbOpen]
    {
        auto& sandbox = m_sandboxes[sbId];

        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(sandbox.GetRootPath());

        CloseDBForSandbox(sandbox);
        sandbox.GetEnvironment()["db"] = dbOpen(UTF16ToUTF8(GetLuaPath(L"db.sqlite3", sandbox.GetRootPath(), true).native()));

        current_path(previousCurrentPath);
    };

    sbEnv["sqlite3"] = sqlite3Copy;

    const auto previousCurrentPath = std::filesystem::current_path();
    current_path(cSBRootPath);

    sbEnv["db"] = dbOpen(UTF16ToUTF8(GetLuaPath(L"db.sqlite3", cSBRootPath, true).native()));

    current_path(previousCurrentPath);
}

void LuaSandbox::InitializeIOForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName)
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();
    auto globals = acpState.globals();
    const auto cSBEnv = sbEnv;
    sol::state_view stateView = acpState;

    const auto cLoadString = [stateView, cSBEnv](const std::string& acStr, const std::string& acChunkName) -> std::tuple<sol::object, sol::object>
    {
        if (!acStr.empty() && acStr[0] == LUA_SIGNATURE[0])
            return std::make_tuple(sol::nil, make_object(stateView, "Bytecode prohibited!"));

        sol::state_view luaView = stateView;
        const auto& acKey = acChunkName.empty() ? acStr : acChunkName;
        const auto cResult = luaView.load(acStr, acKey, sol::load_mode::text);
        if (cResult.valid())
        {
            const auto cFunc = cResult.get<sol::function>();
            cSBEnv.set_on(cFunc);
            return std::make_tuple(cFunc, sol::nil);
        }

        return std::make_tuple(sol::nil, make_object(stateView, cResult.get<sol::error>().what()));
    };
    sbEnv["loadstring"] = cLoadString;

    const auto cLoadFile = [cSBRootPath, cLoadString, stateView](const std::string& acPath) -> std::tuple<sol::object, sol::object>
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        auto path = GetLuaPath(acPath, cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
            path = GetLuaPath(acPath + ".lua", cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
        {
            current_path(previousCurrentPath);

            return std::make_tuple(sol::nil, make_object(stateView, "Tried to access invalid path '" + acPath + "'!"));
        }

        std::ifstream ifs(path);
        const std::string cScriptString((std::istreambuf_iterator(ifs)), std::istreambuf_iterator<char>());
        auto result = cLoadString(cScriptString, "@" + UTF16ToUTF8(path.native()));

        current_path(previousCurrentPath);

        return result;
    };
    sbEnv["loadfile"] = cLoadFile;

    sbEnv["dofile"] = [cLoadFile](const std::string& acPath) -> sol::object
    {
        const auto ret = cLoadFile(acPath);
        if (std::get<0>(ret) == sol::nil)
            throw sol::error(std::get<1>(ret).as<std::string>());

        const sol::function func = std::get<0>(ret);
        return func().get<sol::object>(); // is OK, dofile should throw if there is an error, we try to copy it...
    };

    // TODO - add _LOADED table and fill in when module loads some value, react in these functions when the key is
    // sol::nil
    sbEnv["require"] = [this, cLoadString, cSBRootPath, stateView, cSBEnv](const std::string& acPath) -> std::tuple<sol::object, sol::object>
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        auto path = GetLuaPath(acPath, cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
            path = GetLuaPath(acPath + ".lua", cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
            path = GetLuaPath(acPath + "\\init.lua", cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
        {
            current_path(previousCurrentPath);

            return std::make_tuple(sol::nil, make_object(stateView, "Tried to access invalid path '" + acPath + "'!"));
        }

        const auto cKey = UTF16ToUTF8((cSBRootPath / path).native());
        const auto cExistingModule = m_modules.find(cKey);
        if (cExistingModule != m_modules.end())
        {
            current_path(previousCurrentPath);

            return std::make_tuple(cExistingModule->second, sol::nil);
        }

        std::ifstream ifs(path);
        const std::string cScriptString((std::istreambuf_iterator(ifs)), std::istreambuf_iterator<char>());
        auto res = cLoadString(cScriptString, "@" + cKey);
        auto obj = std::get<0>(res);
        if (obj == sol::nil)
        {
            current_path(previousCurrentPath);

            return res;
        }

        sol::function func = std::get<0>(res);
        if (func != sol::nil)
        {
            // TODO: proper exception handling!
            sol::protected_function_result result{};
            try
            {
                result = func();
            }
            catch (std::exception& e)
            {
                current_path(previousCurrentPath);

                return std::make_tuple(sol::nil, make_object(stateView, e.what()));
            }

            if (result.valid())
            {
                auto resultObj = result.get<sol::object>();
                m_modules[cKey] = resultObj;

                current_path(previousCurrentPath);

                return std::make_tuple(resultObj, sol::nil);
            }

            sol::error err = result;
            auto logger = cSBEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Error: Cannot load module '{}': {}", acPath, err.what());

            current_path(previousCurrentPath);

            return std::make_tuple(sol::nil, make_object(stateView, err.what()));
        }

        current_path(previousCurrentPath);

        return res;
    };

    sbEnv["dir"] = [cSBRootPath, stateView](const std::string& acPath) -> sol::table
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        const auto path = GetLuaPath(acPath, cSBRootPath, false);

        if (path.empty() || !is_directory(path))
        {
            current_path(previousCurrentPath);

            return sol::nil;
        }

        sol::table res(stateView, sol::create);
        int index = 1;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            sol::table item(stateView, sol::create);
            item["name"] = UTF16ToUTF8(relative(entry.path(), path).native());
            item["type"] = entry.is_directory() ? "directory" : "file";
            res[index++] = item;
        }

        current_path(previousCurrentPath);

        return res;
    };

    // define replacements for io lib
    {
        const auto cIO = globals["io"].get<sol::table>();
        sol::table ioSB(acpState, sol::create);
        ioSB["read"] = DeepCopySolObject(cIO["read"], acpState);
        ioSB["write"] = DeepCopySolObject(cIO["write"], acpState);
        ioSB["input"] = DeepCopySolObject(cIO["input"], acpState);
        ioSB["output"] = DeepCopySolObject(cIO["output"], acpState);
        ioSB["type"] = DeepCopySolObject(cIO["type"], acpState);
        ioSB["close"] = DeepCopySolObject(cIO["close"], acpState);
        ioSB["lines"] = [cIO, cSBRootPath](const std::string& acPath)
        {
            const auto previousCurrentPath = std::filesystem::current_path();
            current_path(cSBRootPath);

            const auto path = GetLuaPath(acPath, cSBRootPath, false);

            auto result = cIO["lines"](path.empty() || acPath == "db.sqlite3" ? "" : UTF16ToUTF8(path.native()));

            current_path(previousCurrentPath);

            return result;
        };
        const auto cOpenWithMode = [cIO, cSBRootPath](const std::string& acPath, const std::string& acMode)
        {
            const auto previousCurrentPath = std::filesystem::current_path();
            current_path(cSBRootPath);

            const auto path = GetLuaPath(acPath, cSBRootPath, true);

            auto result = cIO["open"](path.empty() || acPath == "db.sqlite3" ? "" : UTF16ToUTF8(path.native()), acMode);

            current_path(previousCurrentPath);

            return result;
        };
        auto cOpenDefault = [cOpenWithMode](const std::string& acPath)
        {
            return cOpenWithMode(acPath, "r");
        };
        ioSB["open"] = sol::overload(cOpenDefault, cOpenWithMode);
        sbEnv["io"] = ioSB;
    }

    // add in rename and remove replacements for os lib
    {
        const auto cOS = globals["os"].get<sol::table>();
        sol::table osSB = DeepCopySolObject(m_globals["os"].get<sol::object>(), acpState);
        osSB["rename"] = [cOS, cSBRootPath](const std::string& acOldPath, const std::string& acNewPath) -> std::tuple<sol::object, std::string>
        {
            const auto previousCurrentPath = std::filesystem::current_path();
            current_path(cSBRootPath);

            const auto oldPath = GetLuaPath(acOldPath, cSBRootPath, false);
            if (oldPath.empty() || acOldPath == "db.sqlite3")
            {
                current_path(previousCurrentPath);

                return std::make_tuple(sol::nil, "Argument oldPath is invalid! ('" + acOldPath + "')");
            }

            const auto newPath = GetLuaPath(acOldPath, cSBRootPath, true);
            if (newPath.empty() || exists(newPath) || acNewPath == "db.sqlite3")
            {
                current_path(previousCurrentPath);

                return std::make_tuple(sol::nil, "Argument newPath is invalid! ('" + acNewPath + "')");
            }

            const auto cResult = cOS["rename"](UTF16ToUTF8(oldPath.native()), UTF16ToUTF8(newPath.native()));

            current_path(previousCurrentPath);

            return cResult.valid() ? std::make_tuple(cResult.get<sol::object>(), "") : std::make_tuple(cResult.get<sol::object>(0), cResult.get<std::string>(1));
        };
        osSB["remove"] = [cOS, cSBRootPath](const std::string& acPath) -> std::tuple<sol::object, std::string>
        {
            const auto previousCurrentPath = std::filesystem::current_path();
            current_path(cSBRootPath);

            const auto path = GetLuaPath(acPath, cSBRootPath, false);

            const auto cResult = cOS["remove"](UTF16ToUTF8(path.native()));

            current_path(previousCurrentPath);

            return cResult.valid() ? std::make_tuple(cResult.get<sol::object>(), "") : std::make_tuple(cResult.get<sol::object>(0), cResult.get<std::string>(1));
        };
        sbEnv["os"] = osSB;
    }

    // add support functions for bindings
    sbEnv["IsBound"] = [vkb = &m_vkBindings, name = acName](const std::string& aID) -> bool
    {
        return vkb->IsBound({name, aID});
    };
    sbEnv["GetBind"] = [vkb = &m_vkBindings, name = acName](const std::string& aID) -> std::string
    {
        return vkb->GetBindString({name, aID});
    };
}

void LuaSandbox::InitializeLoggerForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();

    // initialize logger for this mod
    auto logger = CreateLogger(GetAbsolutePath(acName + ".log", cSBRootPath, true), acName);

    // assign logger to mod so it can be used from within it too
    sol::table spdlog(acpState, sol::create);
    spdlog["trace"] = [logger](const std::string& message)
    {
        logger->trace("{}", message);
    };
    spdlog["debug"] = [logger](const std::string& message)
    {
        logger->debug("{}", message);
    };
    spdlog["info"] = [logger](const std::string& message)
    {
        logger->info("{}", message);
    };
    spdlog["warning"] = [logger](const std::string& message)
    {
        logger->warn("{}", message);
    };
    spdlog["error"] = [logger](const std::string& message)
    {
        logger->error("{}", message);
    };
    spdlog["critical"] = [logger](const std::string& message)
    {
        logger->critical("{}", message);
    };
    sbEnv["spdlog"] = spdlog;

    // assign logger to special var so we can access it from our functions
    sbEnv["__logger"] = logger;
}

void LuaSandbox::CloseDBForSandbox(const Sandbox& aSandbox) const
{
    aSandbox.ExecuteString(R"(
        if type(db) == 'userdata' and tostring(db):find('^sqlite database') then
            db:close()
        end
    )");
}
