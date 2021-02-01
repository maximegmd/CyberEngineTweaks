#include <stdafx.h>

#include "LuaSandbox.h"

#include <Utils.h>

LuaSandbox::LuaSandbox(sol::state_view aStateView)
    : m_lua(aStateView)
{
}

void LuaSandbox::Initialize(sol::state_view aStateView)
{
    // initialize state + environment first
    m_lua = aStateView;
    m_env = { aStateView, sol::create };

    // copy whitelisted things from global table
    constexpr std::array<std::string_view, 55> whitelistedGlobals =
    {
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
        "rawset",

        "select",
        "setmetatable", //< Required for implementing classes
        "tonumber",
        "tostring",
        "type",
        "unpack",
        "_VERSION",
        "xpcall",

        // CET specific
        "NewObject",
        "DumpReflection",
        "DumpVtables",
        "GetVersion",
        "DumpAllTypeNames",
        "ClassReference",
        "GetDisplayResolution",
        "Dump",
        "ToVector3",
        "Vector4",
        "Game",
        "DumpType",
        "Enum",
        "ToEulerAngles",
        "GameOptions",
        "GameDump",
        "GetSingleton",
        "Descriptor",
        "ItemID",
        "ToItemID",
        "TweakDBID",
        "ToCName",
        "CName",
        "Vector3",
        "Quaternion",
        "EulerAngles",
        "ToVector4",
        "Unknown",
        "ToQuaternion",
        "SingletonReference",
        "StrongReference",
        "ToTweakDBID",
        "WeakReference",
        "GetMod",
        "__Game",
        "__Type",
    };
    const auto globals = m_lua.globals();
    for (const auto& key : whitelistedGlobals)
        m_env[key].set(globals[key].get<sol::object>());

    // copy whitelisted libs from global table
    constexpr std::array<std::string_view, 3> whitelistedTables =
    {
        "string",
        "table",
        "math",
    };
    for (const auto &tableKey : whitelistedTables)
        m_env[tableKey].set(globals[tableKey].get<sol::table>());
    
    // copy safe os functions
    {
        sol::table os = globals["os"].get<sol::table>();
        sol::table osCopy(m_lua, sol::create);
        osCopy["clock"] = os["clock"];
        osCopy["date"] = os["date"];
        osCopy["difftime"] = os["difftime"];
        osCopy["time"] = os["time"];
        m_env["os"] = osCopy;
    }

    CreateSandbox("", false, false, false);
}

void LuaSandbox::ResetState()
{
    m_modules.clear();
    if (m_sandboxes.size() > 1) // first one is always present, meant for console
        m_sandboxes.erase(m_sandboxes.cbegin()+1, m_sandboxes.cend());
}

size_t LuaSandbox::CreateSandbox(const std::filesystem::path& acPath, bool aEnableExtraLibs, bool aEnableDB, bool aEnableIO)
{
    size_t resID = m_sandboxes.size();
    auto& res = m_sandboxes.emplace_back(m_lua, m_env, acPath);
    if (aEnableExtraLibs)
        InitializeExtraLibsForSandbox(res);
    if (aEnableDB)
        InitializeDBForSandbox(res);
    if (aEnableIO)
        InitializeIOForSandbox(res);
    return resID; 
}

std::shared_ptr<spdlog::logger> LuaSandbox::InitializeLoggerForSandbox(Sandbox& aSandbox, const std::string& acName) const 
{
    auto& sbEnv = aSandbox.GetEnvironment();
    auto sbRootPath = aSandbox.GetRootPath();

    // initialize logger for this mod
    auto logger = CreateLogger(sbRootPath / (acName + ".log"), acName);

    // assign logger to mod so it can be used from within it too
    sol::table spdlog(m_lua, sol::create);
    spdlog["trace"] = [logger](const std::string& message)
    {
        logger->trace(message);
    };
    spdlog["debug"] = [logger](const std::string& message)
    {
        logger->debug(message);
    };
    spdlog["info"] = [logger](const std::string& message)
    {
        logger->info(message);
    };
    spdlog["warning"] = [logger](const std::string& message)
    {
        logger->warn(message);
    };
    spdlog["error"] = [logger](const std::string& message)
    {
        logger->error(message);
    };
    spdlog["critical"] = [logger](const std::string& message)
    {
        logger->critical(message);
    };
    sbEnv["spdlog"] = spdlog;

    return logger;
}

sol::protected_function_result LuaSandbox::ExecuteFile(const std::string& acPath)
{
    return m_sandboxes[0].ExecuteFile(acPath);
}

sol::protected_function_result LuaSandbox::ExecuteString(const std::string& acString)
{
    return m_sandboxes[0].ExecuteString(acString);
}

Sandbox& LuaSandbox::operator[](size_t id)
{
    assert(id < m_sandboxes.size());
    return m_sandboxes[id];
}

const Sandbox& LuaSandbox::operator[](size_t id) const
{
    assert(id < m_sandboxes.size());
    return m_sandboxes[id];
}

void LuaSandbox::InitializeExtraLibsForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();

    // copy extra whitelisted libs from global table
    constexpr std::array<std::string_view, 18> whitelistedTables =
    {
        "ImGui",
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
        "ImGuiCol",
        "ImGuiDir",
        "json"
    };
    auto globals = m_lua.globals();
    for (const auto &tableKey : whitelistedTables)
        sbEnv[tableKey].set(globals[tableKey].get<sol::table>());
}

void LuaSandbox::InitializeDBForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    auto sbRootPath = aSandbox.GetRootPath();

    auto globals = m_lua.globals();
    sol::table sqlite3 = globals["sqlite3"].get<sol::table>();
    sol::table sqlite3Copy(m_lua, sol::create);
    for (auto kv : sqlite3)
    {
        std::string keyStr = kv.first.as<std::string>();
        if (keyStr.compare(0, 4, "open"))
            sqlite3Copy[kv.first] = kv.second;
    }
    sbEnv["sqlite3"] = sqlite3Copy;

    sbEnv["db"] = m_lua["sqlite3"]["open"]((sbRootPath / "db.sqlite3").string());
}

void LuaSandbox::InitializeIOForSandbox(Sandbox& aSandbox)
{
    auto& sbEnv = aSandbox.GetEnvironment();
    auto sbRootPath = aSandbox.GetRootPath();

    auto loadstring = [](const std::string& aStr, const std::string &aChunkName, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        sol::state_view sv = aThisState;
        sol::environment env = aThisEnv;

        if (!aStr.empty() && (aStr[0] == LUA_SIGNATURE[0]))
            return std::make_tuple(sol::nil, make_object(sv, "Bytecode prohibited!"));

        auto key = (aChunkName.empty()) ? (aStr) : (aChunkName);
        auto result = sv.load(aStr, key, sol::load_mode::text);
        if (result.valid())
        {
            sol::function func = result.get<sol::function>();
            env.set_on(func);
            return std::make_tuple(func, sol::nil);
        }

        return std::make_tuple(sol::nil, make_object(sv, result.get<sol::error>().what()));
    };
    sbEnv["loadstring"] = loadstring;

    auto loadfile = [sbRootPath, loadstring](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        sol::state_view sv = aThisState;

        auto absPath = absolute(sbRootPath / acPath).make_preferred();
        if (!exists(absPath) || !is_regular_file(absPath))
            absPath += ".lua";
        auto relPath = relative(absPath, sbRootPath);
        auto relPathStr =  relPath.string();
        if (!exists(absPath) || !is_regular_file(absPath) || (relPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(sv, "Invalid path!"));

        std::ifstream t(absPath);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        return loadstring(str, "@" + absPath.string(), aThisState, aThisEnv);
    };
    sbEnv["loadfile"] = loadfile;

    auto dofile = [loadfile](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> sol::object
    {
        std::tuple<sol::object, sol::object> ret = loadfile(acPath, aThisState, aThisEnv);
        if (std::get<0>(ret) == sol::nil)
            throw sol::error(std::get<1>(ret).as<std::string>());

        sol::function func = std::get<0>(ret);
        return func().get<sol::object>(); // is OK, dofile should throw if there is an error, we try to copy it...
    };
    sbEnv["dofile"] = dofile;

    auto require = [this, loadstring, sbRootPath](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        auto absPath = absolute(sbRootPath / acPath).make_preferred();
        if (!exists(absPath) || !is_regular_file(absPath))
        {
            auto absPath2 = absPath;
            absPath2 += ".lua";
            if (exists(absPath2) && is_regular_file(absPath2))
                absPath = absPath2;
            else
            {
                auto absPath3 = absPath;
                absPath3 /= "init.lua";
                if (!exists(absPath3) || !is_regular_file(absPath3))
                    return std::make_tuple(sol::nil, make_object(this->m_lua, "Invalid path!"));
                absPath = absPath3;
            }
        }
        auto relPath = relative(absPath, sbRootPath);
        auto relPathStr =  relPath.string();
        if ((relPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(this->m_lua, "Invalid path!"));

        auto key = absPath.string();
        auto existingModule = this->m_modules.find(key);
        if (existingModule != this->m_modules.end())
            return std::make_tuple(existingModule->second, sol::nil);

        std::ifstream t(absPath);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        auto res = loadstring(str, "@" + absPath.string(), aThisState, aThisEnv);
        sol::function func = std::get<0>(res);
        if (func != sol::nil)
        {
            auto res = func();
            if (res.valid())
            {
                auto obj = res.get<sol::object>();
                this->m_modules[key] = obj;
                return std::make_tuple(obj, sol::nil);
            }
            sol::error err = res;
            return std::make_tuple(sol::nil, make_object(this->m_lua, err.what()));
        }
        return res;
    };
    sbEnv["require"] = require;

    auto dir = [sbRootPath](const std::string& acPath, sol::this_state aThisState) -> sol::table
    {
        sol::state_view sv = aThisState;

        auto absPath = absolute(sbRootPath / acPath).make_preferred();
        auto relPath = relative(absPath, sbRootPath);
        auto relPathStr =  relPath.string();
        if (!exists(absPath) || !is_directory(absPath) || (relPathStr.find("..") != std::string::npos))
            return sol::nil;

        sol::table res(sv, sol::create);
        int index = 1;
        for (const auto& file : std::filesystem::directory_iterator(absPath))
        {
            sol::table item(sv, sol::create);
            item["name"] = relative(file.path(), absPath).string();
            item["type"] = (file.is_directory()) ? ("directory") : ("file");
            res[index++] = item;
        }
        return res;
    };
    sbEnv["dir"] = dir;
    
    auto globals = m_lua.globals();
    // define replacements for io lib
    {
        sol::table io = globals["io"].get<sol::table>();
        sol::table ioSB(m_lua, sol::create);
        ioSB["close"] = io["close"];
        ioSB["lines"] = [io, sbRootPath](const std::string& acPath)
        {
            auto absPath = absolute(sbRootPath / acPath).make_preferred();
            auto relPath = relative(absPath, sbRootPath);
            auto relPathStr =  relPath.string();
            if (relPathStr.find("..") == std::string::npos)
                return io["lines"](absPath.string());
            return io["lines"](""); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
        auto openWithMode = [io, sbRootPath](const std::string& acPath, const std::string& acMode)
        {
            auto absPath = absolute(sbRootPath / acPath).make_preferred();
            auto relPath = relative(absPath, sbRootPath);
            auto relPathStr =  relPath.string();
            if (relPathStr.find("..") == std::string::npos)
                return io["open"](absPath.string(), acMode);
            return io["open"]("", acMode); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
        auto openDefault = [openWithMode](const std::string& acPath)
        {
            return openWithMode(acPath, "r");
        };
        ioSB["type"] = io["type"];
        ioSB["open"] = sol::overload(openDefault, openWithMode);
        sbEnv["io"] = ioSB;
    }

    // add in rename and remove repacements for os lib
    {
        sol::table os = globals["os"].get<sol::table>();
        auto osEnv = sbEnv["os"];
        osEnv["rename"] = [os, sbRootPath](const std::string& acOldPath, const std::string& acNewPath) -> std::tuple<sol::object, std::string>
        {
            auto absOldPath = absolute(sbRootPath / acOldPath).make_preferred();
            auto relOldPath = relative(absOldPath, sbRootPath);
            auto relOldPathStr =  relOldPath.string();
            if (!exists(absOldPath) || (relOldPathStr.find("..") != std::string::npos) || (relOldPathStr == "db.sqlite3"))
                return std::make_tuple(sol::nil, "Argument oldpath is invalid!");

            auto absNewPath = absolute(sbRootPath / acNewPath).make_preferred();
            auto relNewPath = relative(absNewPath, sbRootPath);
            auto relNewPathStr =  relNewPath.string();
            if (relNewPathStr.find("..") != std::string::npos)
                return std::make_tuple(sol::nil, "Argument newpath is invalid!");

            auto res = os["rename"](absOldPath.string(), absNewPath.string());
            if (res.valid())
                return std::make_tuple(res.get<sol::object>(), "");
            return std::make_tuple(res.get<sol::object>(0), res.get<std::string>(1));
        };
        osEnv["remove"] = [os, sbRootPath](const std::string& acPath) -> std::tuple<sol::object, std::string>
        {
            auto absPath = absolute(sbRootPath / acPath).make_preferred();
            auto relPath = relative(absPath, sbRootPath);
            auto relPathStr =  relPath.string();
            if (!exists(absPath) || (relPathStr.find("..") != std::string::npos) || (relPathStr == "db.sqlite3"))
                return std::make_tuple(sol::nil, "Argument path is invalid!");

            auto res = os["remove"](absPath.string());
            if (res.valid())
                return std::make_tuple(res.get<sol::object>(), "");
            return std::make_tuple(res.get<sol::object>(0), res.get<std::string>(1));
        };
    }
}
