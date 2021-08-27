#include <stdafx.h>

#include "LuaSandbox.h"

#include "Scripting.h"

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

size_t LuaSandbox::CreateSandbox(const std::filesystem::path& acPath, const std::string& acName, bool aEnableExtraLibs, bool aEnableDB, bool aEnableIO, bool aEnableLogger)
{
    const size_t cResID = m_sandboxes.size();
    assert(!cResID || (!acPath.empty() && !acName.empty()));
    auto& res = m_sandboxes.emplace_back(m_pScripting, m_env, acPath);
    if (!acPath.empty() && !acName.empty())
    {
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

TiltedPhoques::Locked<sol::state, std::recursive_mutex> LuaSandbox::GetState() const
{
    return m_pScripting->GetState();
}

void LuaSandbox::InitializeExtraLibsForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();

    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    // copy extra whitelisted libs from global table
    const auto cGlobals = luaView.globals();
    for (const auto* cKey : s_cGlobalExtraLibsWhitelist)
        sbEnv[cKey] = DeepCopySolObject(cGlobals[cKey].get<sol::object>(), luaView);
}

void LuaSandbox::InitializeDBForSandbox(Sandbox& aSandbox) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    const auto cGlobals = luaView.globals();
    const auto cSQLite3 = cGlobals["sqlite3"].get<sol::table>();
    sol::table sqlite3Copy(luaView, sol::create);
    for (const auto& cKV : cSQLite3)
    {
        const auto cKeyStr = cKV.first.as<std::string>();
        if (cKeyStr.compare(0, 4, "open"))
            sqlite3Copy[cKV.first] = DeepCopySolObject(cKV.second, luaView);
    }
    sbEnv["sqlite3"] = sqlite3Copy;

    sbEnv["db"] = luaView["sqlite3"]["open"]((cSBRootPath / "db.sqlite3").string());
}

void LuaSandbox::InitializeIOForSandbox(Sandbox& aSandbox, const std::string& acName)
{
    sol::state_view sbStateView = GetState();
    auto sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    const auto cLoadString = [sbStateView, sbEnv](const std::string& acStr, const std::string &acChunkName) -> std::tuple<sol::object, sol::object>
    {
        sol::state_view sv = sbStateView;
        const sol::environment cEnv = sbEnv;

        if (!acStr.empty() && (acStr[0] == LUA_SIGNATURE[0]))
            return std::make_tuple(sol::nil, make_object(sbStateView, "Bytecode prohibited!"));

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

    const auto cLoadFile = [cSBRootPath, cLoadString, sbStateView](const std::string& acPath) -> std::tuple<sol::object, sol::object>
    {
        auto absPath = absolute(cSBRootPath / acPath).make_preferred();
        if (!exists(absPath) || !is_regular_file(absPath))
            absPath += ".lua";
        const auto cRelPathStr = relative(absPath, cSBRootPath).string();
        if (!exists(absPath) || !is_regular_file(absPath) || (cRelPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(sbStateView, "Invalid path!"));

        std::ifstream ifs(absPath);
        const std::string cScriptString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return cLoadString(cScriptString, "@" + absPath.string());
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

    sbEnv["require"] = [this, cLoadString, cSBRootPath, sbStateView, sbEnv](const std::string& acPath) -> std::tuple<sol::object, sol::object>
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
                    return std::make_tuple(sol::nil, make_object(sbStateView, "Invalid path!"));
                absPath = absPath3;
            }
        }
        const auto cRelPathStr = relative(absPath, cSBRootPath).string();
        if ((cRelPathStr.find("..") != std::string::npos))
            return std::make_tuple(sol::nil, make_object(sbStateView, "Invalid path!"));

        const auto cKey = absPath.string();
        const auto cExistingModule = this->m_modules.find(cKey);
        if (cExistingModule != this->m_modules.end())
            return std::make_tuple(cExistingModule->second, sol::nil);

        std::ifstream ifs(absPath);
        const std::string cScriptString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        auto res = cLoadString(cScriptString, "@" + absPath.string());
        auto obj = std::get<0>(res);
        if (obj == sol::nil)
            return res;

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
                return std::make_tuple(sol::nil, make_object(sbStateView, e.what()));
            }

            if (result.valid())
            {
                auto obj = result.get<sol::object>();
                this->m_modules[cKey] = obj;
                return std::make_tuple(obj, sol::nil);
            }

            sol::error err = result;
			std::shared_ptr<spdlog::logger> logger = sbEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();
			logger->error("Error: Cannot load module '{}': {}", acPath, err.what());

            return std::make_tuple(sol::nil, make_object(sbStateView, err.what()));
        }
        return res;
    };

    sbEnv["dir"] = [cSBRootPath, sbStateView](const std::string& acPath) -> sol::table
    {
        sol::state_view sv = sbStateView;

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

    auto lock = m_pScripting->GetState();
    auto& luaView = lock.Get();

    const auto cGlobals = luaView.globals();
    // define replacements for io lib
    {
        const auto cIO = cGlobals["io"].get<sol::table>();
        sol::table ioSB(luaView, sol::create);
        ioSB["type"] = DeepCopySolObject(cIO["type"], luaView);
        ioSB["close"] = DeepCopySolObject(cIO["close"], luaView);
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

    // add in rename and remove replacements for os lib
    {
        const auto cOS = cGlobals["os"].get<sol::table>();
        sol::table osSB = sbEnv["os"].get<sol::table>();
        osSB["rename"] = [cOS, cSBRootPath](const std::string& acOldPath, const std::string& acNewPath) -> std::tuple<sol::object, std::string>
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
        osSB["remove"] = [cOS, cSBRootPath](const std::string& acPath) -> std::tuple<sol::object, std::string>
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

    // add support functions for bindings
    sbEnv["IsBound"] = [vkb = &m_vkBindings, name = acName](const std::string& aID) -> bool
    {
        return vkb->IsBound(name + '.' + aID);
    };
    sbEnv["GetBind"] = [vkb = &m_vkBindings, name = acName](const std::string& aID) -> std::string
    {
        return vkb->GetBindString(name + '.' + aID);
    };
}

void LuaSandbox::InitializeLoggerForSandbox(Sandbox& aSandbox, const std::string& acName) const
{
    auto& sbEnv = aSandbox.GetEnvironment();
    const auto cSBRootPath = aSandbox.GetRootPath();

    // initialize logger for this mod
    auto logger = CreateLogger(cSBRootPath / (acName + ".log"), acName);

    auto state = m_pScripting->GetState();

    // assign logger to mod so it can be used from within it too
    sol::table spdlog(state.Get(), sol::create);
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

void LuaSandbox::CloseDBForSandbox(Sandbox& aSandbox) const
{
    aSandbox.ExecuteString(R"(
        if type(db) == 'userdata' and tostring(db):find('^sqlite database') then
            db:close()
        end
    )");
}
