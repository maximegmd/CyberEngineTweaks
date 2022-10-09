#include <stdafx.h>

#include "LuaSandbox.h"

#include "Scripting.h"
#include "Texture.h"

#include <Utils.h>

static constexpr const char* s_cGlobalObjectsWhitelist[] =
{
    "_VERSION",
    "assert",
    "error",
    "getmetatable", //< Used to extend string class
    "ipairs",
    "next",
    "pairs",
    "pcall",
    "print",

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

    "collectgarbage", //< Good for testing memory leaks and ref counters: `collectgarbage("collect")`, also used for forcing the release of some refs
};

static constexpr const char* s_cGlobalTablesWhitelist[] =
{
    "string",
    "table",
    "math",
    "bit32"
};

static constexpr const char* s_cGlobalImmutablesList[] =
{
    "__Game",
    "__Type",
    "ClassReference",
    "CName",
    "Descriptor",
    "Enum",
    "EulerAngles",
    "Game",
    "GameOptions",
    "ItemID",
    "Quaternion",
    "SingletonReference",
    "StrongReference",
    "TweakDBID",
    "Unknown",
    "Vector3",
    "Vector4",
    "WeakReference"
};

static constexpr const char* s_cGlobalImGuiList[] =
{
    "ImGui"
};

static constexpr const char* s_cGlobalExtraLibsWhitelist[] =
{
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
    "ImVec4",
    "json"
};

LuaSandbox::LuaSandbox(Scripting* apScripting, const VKBindings& acVKBindings)
    : m_pScripting(apScripting)
    , m_vkBindings(acVKBindings)
{
}

void LuaSandbox::Initialize()
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    // initialize state + environment first
    m_env = {luaView, sol::create};

    // copy whitelisted things from global table
    const auto cGlobals = luaView.globals();
    for (const auto* cKey : s_cGlobalObjectsWhitelist)
        m_env[cKey].set(cGlobals[cKey].get<sol::object>());

    // copy whitelisted libs from global table
    for (const auto* cKey : s_cGlobalTablesWhitelist)
        m_env[cKey].set(cGlobals[cKey].get<sol::table>());

    // copy safe os functions
    {
        auto os = cGlobals["os"].get<sol::table>();
        sol::table osCopy(luaView, sol::create);
        osCopy["clock"] = os["clock"];
        osCopy["date"] = os["date"];
        osCopy["difftime"] = os["difftime"];
        osCopy["time"] = os["time"];
        m_env["os"] = osCopy;
    }

    CreateSandbox();
}

void LuaSandbox::PostInitialize()
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    // make shared things immutable
    for (const auto* cKey : s_cGlobalImmutablesList)
        MakeSolUsertypeImmutable(luaView[cKey], luaView);
}

void LuaSandbox::ResetState()
{
    for (auto& cSandbox : m_sandboxes)
        CloseDBForSandbox(cSandbox);

    m_modules.clear();

    if (m_sandboxes.size() > 1) // first one is always present, meant for console
        m_sandboxes.erase(m_sandboxes.cbegin()+1, m_sandboxes.cend());

    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();
    luaView.collect_garbage();
}

uint64_t LuaSandbox::CreateSandbox(const std::filesystem::path& acPath, const std::string& acName, bool aEnableImGui, bool aEnableExtraLibs, bool aEnableDB, bool aEnableIO, bool aEnableLogger)
{
    const uint64_t cResID = m_sandboxes.size();
    assert(!cResID || (!acPath.empty() && !acName.empty()));
    auto& res = m_sandboxes.emplace_back(cResID, m_pScripting, m_env, acPath);
    if (!acPath.empty() && !acName.empty())
    {
        if (aEnableImGui)
            InitializeImGuiForSandbox(res);
        if (aEnableExtraLibs)
            InitializeExtraLibsForSandbox(res);
        if (aEnableDB)
            InitializeDBForSandbox(res);
        if (aEnableIO)
            InitializeIOForSandbox(res, acName);
        if (aEnableLogger)
            InitializeLoggerForSandbox(res, acName);
    }
    return cResID;
}

sol::protected_function_result LuaSandbox::ExecuteFile(const std::string& acPath)
{
    return m_sandboxes[0].ExecuteFile(acPath);
}

sol::protected_function_result LuaSandbox::ExecuteString(const std::string& acString)
{
    return m_sandboxes[0].ExecuteString(acString);
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

TiltedPhoques::Locked<sol::state, std::recursive_mutex> LuaSandbox::GetState() const
{
    return m_pScripting->GetState();
}

void LuaSandbox::InitializeImGuiForSandbox(Sandbox& aSandbox) const
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    sol::state_view sbStateView = luaView;
    const auto& cSBRootPath = aSandbox.GetRootPath();

    sol::table imgui{luaView, sol::create};

    // copy ImGui from global table
    const auto cGlobals = luaView.globals();
    for (const auto* cKey : s_cGlobalImGuiList)
        imgui[cKey] = DeepCopySolObject(cGlobals[cKey].get<sol::object>(), luaView);

    Texture::BindTexture(imgui);

    const auto cLoadTexture = [cSBRootPath, sbStateView](const std::string& acPath) -> std::tuple<std::shared_ptr<Texture>, sol::object> {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        const auto path = GetLuaPath(acPath, cSBRootPath, false);
        auto texture = Texture::Load(UTF16ToUTF8(path.native()));
        if (!texture)
            return std::make_tuple(nullptr, make_object(sbStateView, "Failed to load '" + acPath + "'"));

        current_path(previousCurrentPath);

        return std::make_tuple(texture, sol::nil);
    };
    imgui.set_function("LoadTexture", cLoadTexture);

    aSandbox.GetImGui() = imgui;
}

void LuaSandbox::InitializeExtraLibsForSandbox(Sandbox& aSandbox) const
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    auto& sbEnv = aSandbox.GetEnvironment();

    // copy extra whitelisted libs from global table
    const auto cGlobals = luaView.globals();
    for (const auto* cKey : s_cGlobalExtraLibsWhitelist)
        sbEnv[cKey] = DeepCopySolObject(cGlobals[cKey].get<sol::object>(), luaView);
}

void LuaSandbox::InitializeDBForSandbox(Sandbox& aSandbox)
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();
    const auto sbId = aSandbox.GetId();

    const auto cGlobals = luaView.globals();
    const auto cSQLite3 = cGlobals["sqlite3"].get<sol::table>();
    sol::table sqlite3Copy(luaView, sol::create);
    for (const auto& cKV : cSQLite3)
    {
        const auto cKeyStr = cKV.first.as<std::string>();
        if (cKeyStr.compare(0, 4, "open"))
            sqlite3Copy[cKV.first] = DeepCopySolObject(cKV.second, luaView);
    }
    const auto dbOpen = luaView["sqlite3"]["open"].get<sol::function>();
    const auto dbPath = UTF16ToUTF8(GetLuaPath(L"db.sqlite3", cSBRootPath, false).native());
    sqlite3Copy["reopen"] = [this, sbId, dbPath, dbOpen]{
        auto& sandbox = m_sandboxes[sbId];

        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(sandbox.GetRootPath());

        CloseDBForSandbox(sandbox);
        sandbox.GetEnvironment()["db"] = dbOpen(dbPath);

        current_path(previousCurrentPath);
    };

    sbEnv["sqlite3"] = sqlite3Copy;

    const auto previousCurrentPath = std::filesystem::current_path();
    current_path(cSBRootPath);

    sbEnv["db"] = dbOpen(dbPath);

    current_path(previousCurrentPath);
}

void LuaSandbox::InitializeIOForSandbox(Sandbox& aSandbox, const std::string& acName)
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    sol::state_view sbStateView = luaView;
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();

    const auto cLoadString = [sbStateView, sbEnv](const std::string& acStr, const std::string &acChunkName) -> std::tuple<sol::object, sol::object>
    {
        if (!acStr.empty() && (acStr[0] == LUA_SIGNATURE[0]))
            return std::make_tuple(sol::nil, make_object(sbStateView, "Bytecode prohibited!"));

        sol::state_view sv = sbStateView;
        const auto& acKey = (acChunkName.empty()) ? (acStr) : (acChunkName);
        const auto cResult = sv.load(acStr, acKey, sol::load_mode::text);
        if (cResult.valid())
        {
            const auto cFunc = cResult.get<sol::function>();
            sbEnv.set_on(cFunc);
            return std::make_tuple(cFunc, sol::nil);
        }

        return std::make_tuple(sol::nil, make_object(sbStateView, cResult.get<sol::error>().what()));
    };
    sbEnv["loadstring"] = cLoadString;

    const auto cLoadFile = [cSBRootPath, cLoadString, sbStateView](const std::string& acPath) -> std::tuple<sol::object, sol::object>
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        auto path = GetLuaPath(acPath, cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
            path = GetLuaPath(acPath + ".lua", cSBRootPath, false);

        if (path.empty() || !is_regular_file(path))
        {
            current_path(previousCurrentPath);

            return std::make_tuple(sol::nil, make_object(sbStateView, "Tried to access invalid path '" + acPath + "'!"));
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

    // TODO - add _LOADED table and fill in when module loads some value, react in these functions when the key is sol::nil
    sbEnv["require"] = [this, cLoadString, cSBRootPath, sbStateView, sbEnv](const std::string& acPath) -> std::tuple<sol::object, sol::object>
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

            return std::make_tuple(sol::nil, make_object(sbStateView, "Tried to access invalid path '" + acPath + "'!"));
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
            sol::protected_function_result result{ };
            try
            {
                result = func();
            }
            catch(std::exception& e)
            {
                current_path(previousCurrentPath);

                return std::make_tuple(sol::nil, make_object(sbStateView, e.what()));
            }

            if (result.valid())
            {
                auto resultObj = result.get<sol::object>();
                m_modules[cKey] = resultObj;

                current_path(previousCurrentPath);

                return std::make_tuple(resultObj, sol::nil);
            }

            sol::error err = result;
            std::shared_ptr<spdlog::logger> logger = sbEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
            logger->error("Error: Cannot load module '{}': {}", acPath, err.what());

            current_path(previousCurrentPath);

            return std::make_tuple(sol::nil, make_object(sbStateView, err.what()));
        }

        current_path(previousCurrentPath);

        return res;
    };

    sbEnv["dir"] = [cSBRootPath, sbStateView](const std::string& acPath) -> sol::table
    {
        const auto previousCurrentPath = std::filesystem::current_path();
        current_path(cSBRootPath);

        const auto path = GetLuaPath(acPath, cSBRootPath, false);

        if (path.empty() || !is_directory(path))
        {
            current_path(previousCurrentPath);

            return sol::nil;
        }

        sol::table res(sbStateView, sol::create);
        int index = 1;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            sol::table item(sbStateView, sol::create);
            item["name"] = UTF16ToUTF8(relative(entry.path(), path).native());
            item["type"] = entry.is_directory() ? ("directory") : ("file");
            res[index++] = item;
        }

        current_path(previousCurrentPath);

        return res;
    };

    const auto cGlobals = luaView.globals();
    // define replacements for io lib
    {
        const auto cIO = cGlobals["io"].get<sol::table>();
        sol::table ioSB(luaView, sol::create);
        ioSB["read"] = DeepCopySolObject(cIO["read"], luaView);
        ioSB["write"] = DeepCopySolObject(cIO["write"], luaView);
        ioSB["input"] = DeepCopySolObject(cIO["input"], luaView);
        ioSB["output"] = DeepCopySolObject(cIO["output"], luaView);
        ioSB["type"] = DeepCopySolObject(cIO["type"], luaView);
        ioSB["close"] = DeepCopySolObject(cIO["close"], luaView);
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
        const auto cOS = cGlobals["os"].get<sol::table>();
        sol::table osSB = sbEnv["os"].get<sol::table>();
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
            if (newPath.empty() || acNewPath == "db.sqlite3")
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

void LuaSandbox::InitializeLoggerForSandbox(Sandbox& aSandbox, const std::string& acName) const
{
    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    auto& sbEnv = aSandbox.GetEnvironment();
    const auto& cSBRootPath = aSandbox.GetRootPath();

    // initialize logger for this mod
    auto logger = CreateLogger(GetAbsolutePath((acName + ".log"), cSBRootPath, true), acName);

    // assign logger to mod so it can be used from within it too
    sol::table spdlog(luaView, sol::create);
    spdlog["trace"]    = [logger](const std::string& message) { logger->trace(message);    };
    spdlog["debug"]    = [logger](const std::string& message) { logger->debug(message);    };
    spdlog["info"]     = [logger](const std::string& message) { logger->info(message);     };
    spdlog["warning"]  = [logger](const std::string& message) { logger->warn(message);     };
    spdlog["error"]    = [logger](const std::string& message) { logger->error(message);    };
    spdlog["critical"] = [logger](const std::string& message) { logger->critical(message); };
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
