#include <stdafx.h>

#include "LuaSandbox.h"

#include <Utils.h>

static constexpr const char* s_cGlobalObjectsWhitelist[] =
{
    "assert",
    "error",
    "getmetatable", //< Used to extend string class
    "ipairs",
    "next",
    "pairs",
    "pcall",
    "print",
    "load",

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
    "TweakDB"
};

static constexpr const char* s_cGlobalTablesWhitelist[] =
{
    "string",
    "table",
    "math",
};

static constexpr const char* s_cGlobalExtraLibsWhitelist[] =
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
    "ImGuiTableFlags",
    "ImGuiTableColumnFlags",
    "ImGuiTableRowFlags",
    "ImGuiTableBgTarget",
    "ImGuiMouseButton",
    "ImDrawCornerFlags",
    "ImGuiCol",
    "ImGuiDir",
    "json"
};

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
    const auto cGlobals = m_lua.globals();
    for (const auto* cKey : s_cGlobalObjectsWhitelist)
        m_env[cKey].set(cGlobals[cKey].get<sol::object>());

    // copy whitelisted libs from global table
    for (const auto* cKey : s_cGlobalTablesWhitelist)
        m_env[cKey].set(cGlobals[cKey].get<sol::table>());
    
    // copy safe os functions
    {
        auto os = cGlobals["os"].get<sol::table>();
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
    const size_t cResID = m_sandboxes.size();
    auto& res = m_sandboxes.emplace_back(m_lua, m_env, acPath);
    if (aEnableExtraLibs)
        InitializeExtraLibsForSandbox(res);
    if (aEnableDB)
        InitializeDBForSandbox(res);
    if (aEnableIO)
        InitializeIOForSandbox(res);
    return cResID; 
}

std::shared_ptr<spdlog::logger> LuaSandbox::InitializeLoggerForSandbox(Sandbox& aSandbox, const std::string& acName) const 
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    // initialize logger for this mod
    auto logger = CreateLogger(cSBRootPath / (acName + ".log"), acName);

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

    // assign logger to special var so we can access it from our functions
    sbEnv["__logger"] = logger;

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

Sandbox& LuaSandbox::operator[](size_t aID)
{
    assert(aID < m_sandboxes.size());
    return m_sandboxes[aID];
}

const Sandbox& LuaSandbox::operator[](size_t aID) const
{
    assert(aID < m_sandboxes.size());
    return m_sandboxes[aID];
}

void LuaSandbox::InitializeExtraLibsForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();

    // copy extra whitelisted libs from global table
    const auto cGlobals = m_lua.globals();
    for (const auto* cKey : s_cGlobalExtraLibsWhitelist)
        sbEnv[cKey].set(cGlobals[cKey].get<sol::table>());
}

void LuaSandbox::InitializeDBForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    const auto cGlobals = m_lua.globals();
    const auto cSQLite3 = cGlobals["sqlite3"].get<sol::table>();
    sol::table sqlite3Copy(m_lua, sol::create);
    for (const auto& cKV : cSQLite3)
    {
        const auto cKeyStr = cKV.first.as<std::string>();
        if (cKeyStr.compare(0, 4, "open"))
            sqlite3Copy[cKV.first] = cKV.second;
    }
    sbEnv["sqlite3"] = sqlite3Copy;

    sbEnv["db"] = m_lua["sqlite3"]["open"]((cSBRootPath / "db.sqlite3").string());
}

void LuaSandbox::InitializeIOForSandbox(Sandbox& aSandbox)
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    const auto cLoadString = [](const std::string& acStr, const std::string &acChunkName, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        sol::state_view sv = aThisState;
        const sol::environment cEnv = aThisEnv;

        if (!acStr.empty() && (acStr[0] == LUA_SIGNATURE[0]))
            return std::make_tuple(sol::nil, make_object(sv, "Bytecode prohibited!"));

        const auto& acKey = (acChunkName.empty()) ? (acStr) : (acChunkName);
        const auto cResult = sv.load(acStr, acKey, sol::load_mode::text);
        if (cResult.valid())
        {
            const auto cFunc = cResult.get<sol::function>();
            cEnv.set_on(cFunc);
            return std::make_tuple(cFunc, sol::nil);
        }

        return std::make_tuple(sol::nil, make_object(sv, cResult.get<sol::error>().what()));
    };
    sbEnv["loadstring"] = cLoadString;

    const auto cLoadFile = [cSBRootPath, cLoadString](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        sol::state_view sv = aThisState;

        auto absPath = absolute(cSBRootPath / acPath).make_preferred();
        if (!exists(absPath) || !is_regular_file(absPath))
            absPath += ".lua";
        const auto cRelPathStr = relative(absPath, cSBRootPath).string();
        if (!exists(absPath) || !is_regular_file(absPath) || (cRelPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(sv, "Invalid path!"));

        std::ifstream ifs(absPath);
        const std::string cScriptString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return cLoadString(cScriptString, "@" + absPath.string(), aThisState, aThisEnv);
    };
    sbEnv["loadfile"] = cLoadFile;

    sbEnv["dofile"] = [cLoadFile](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> sol::object
    {
        const auto ret = cLoadFile(acPath, aThisState, aThisEnv);
        if (std::get<0>(ret) == sol::nil)
            throw sol::error(std::get<1>(ret).as<std::string>());

        const sol::function func = std::get<0>(ret);
        return func().get<sol::object>(); // is OK, dofile should throw if there is an error, we try to copy it...
    };

    sbEnv["require"] = [this, cLoadString, cSBRootPath](const std::string& acPath, sol::this_state aThisState, sol::this_environment aThisEnv) -> std::tuple<sol::object, sol::object>
    {
        auto absPath = absolute(cSBRootPath / acPath).make_preferred();
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
        const auto cRelPathStr = relative(absPath, cSBRootPath).string();
        if ((cRelPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(this->m_lua, "Invalid path!"));

        const auto cKey = absPath.string();
        const auto cExistingModule = this->m_modules.find(cKey);
        if (cExistingModule != this->m_modules.end())
            return std::make_tuple(cExistingModule->second, sol::nil);

        std::ifstream ifs(absPath);
        const std::string cScriptString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        auto res = cLoadString(cScriptString, "@" + absPath.string(), aThisState, aThisEnv);
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
                return std::make_tuple(sol::nil, make_object(this->m_lua, e.what()));
            }
            
            if (result.valid())
            {
                auto obj = result.get<sol::object>();
                this->m_modules[cKey] = obj;
                return std::make_tuple(obj, sol::nil);
            }
            sol::error err = result;
            return std::make_tuple(sol::nil, make_object(this->m_lua, err.what()));
        }
        return res;
    };

    sbEnv["dir"] = [cSBRootPath](const std::string& acPath, sol::this_state aThisState) -> sol::table
    {
        sol::state_view sv = aThisState;

        auto absPath = absolute(cSBRootPath / acPath).make_preferred();
        const auto cRelPathStr = relative(absPath, cSBRootPath).string();
        if (!exists(absPath) || !is_directory(absPath) || (cRelPathStr.find("..") != std::string::npos))
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
    
    const auto cGlobals = m_lua.globals();
    // define replacements for io lib
    {
        const auto cIO = cGlobals["io"].get<sol::table>();
        sol::table ioSB(m_lua, sol::create);
        ioSB["type"] = cIO["type"];
        ioSB["close"] = cIO["close"];
        ioSB["lines"] = [cIO, cSBRootPath](const std::string& acPath)
        {
            const auto cAbsPath = absolute(cSBRootPath / acPath).make_preferred();
            const auto cRelPathStr = relative(cAbsPath, cSBRootPath).string();
            if (cRelPathStr.find("..") == std::string::npos)
                return cIO["lines"](cAbsPath.string());
            return cIO["lines"](""); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
        const auto cOpenWithMode = [cIO, cSBRootPath](const std::string& acPath, const std::string& acMode)
        {
            const auto cAbsPath = absolute(cSBRootPath / acPath).make_preferred();
            const auto cRelPathStr = relative(cAbsPath, cSBRootPath).string();
            if (cRelPathStr.find("..") == std::string::npos && (cRelPathStr != "db.sqlite3"))
                return cIO["open"](cAbsPath.string(), acMode);
            return cIO["open"]("", acMode); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
        auto cOpenDefault = [cOpenWithMode](const std::string& acPath)
        {
            return cOpenWithMode(acPath, "r");
        };
        ioSB["open"] = sol::overload(cOpenDefault, cOpenWithMode);
        sbEnv["io"] = ioSB;
    }

    // add in rename and remove repacements for os lib
    {
        const auto cOS = cGlobals["os"].get<sol::table>();
        auto osEnv = sbEnv["os"]; // os should already be in environment at this point, do not overwrite it!
        osEnv["rename"] = [cOS, cSBRootPath](const std::string& acOldPath, const std::string& acNewPath) -> std::tuple<sol::object, std::string>
        {
            const auto cAbsOldPath = absolute(cSBRootPath / acOldPath).make_preferred();
            const auto cRelOldPathStr =  relative(cAbsOldPath, cSBRootPath).string();
            if (!exists(cAbsOldPath) || (cRelOldPathStr.find("..") != std::string::npos) || (cRelOldPathStr == "db.sqlite3"))
                return std::make_tuple(sol::nil, "Argument oldpath is invalid!");

            const auto cAbsNewPath = absolute(cSBRootPath / acNewPath).make_preferred();
            const auto cRelNewPathStr =  relative(cAbsNewPath, cSBRootPath).string();
            if (cRelNewPathStr.find("..") != std::string::npos)
                return std::make_tuple(sol::nil, "Argument newpath is invalid!");

            const auto cResult = cOS["rename"](cAbsOldPath.string(), cAbsNewPath.string());
            if (cResult.valid())
                return std::make_tuple(cResult.get<sol::object>(), "");
            return std::make_tuple(cResult.get<sol::object>(0), cResult.get<std::string>(1));
        };
        osEnv["remove"] = [cOS, cSBRootPath](const std::string& acPath) -> std::tuple<sol::object, std::string>
        {
            const auto cAbsPath = absolute(cSBRootPath / acPath).make_preferred();
            const auto cRelPathStr = relative(cAbsPath, cSBRootPath).string();
            if (!exists(cAbsPath) || (cRelPathStr.find("..") != std::string::npos) || (cRelPathStr == "db.sqlite3"))
                return std::make_tuple(sol::nil, "Argument path is invalid!");

            const auto cResult = cOS["remove"](cAbsPath.string());
            if (cResult.valid())
                return std::make_tuple(cResult.get<sol::object>(), "");
            return std::make_tuple(cResult.get<sol::object>(0), cResult.get<std::string>(1));
        };
    }
}
