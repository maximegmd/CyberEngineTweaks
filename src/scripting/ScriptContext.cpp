#include <stdafx.h>

#include "ScriptContext.h"

#include <Utils.h>

#include <lua.h>

// TODO: refactor this, prolly put sandboxing stuff into separate class, so we could reuse most of this for console later (also is just messy rn, refactor is in place in general :P )
ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create)
    , m_path(acPath)
    , m_name(relative(m_path, Paths::Get().ModsRoot()).string())
{    
    // initialize logger for this mod
    m_logger = CreateLogger(acPath / (m_name + ".log"), "mods." + m_name);

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

	    // TODO: replace these
	    //"package",
	    //"require",

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
	constexpr std::array<std::string_view, 20> whitelistedTables =
    {
	    "string",
	    "table",
	    "math",
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
        "ImGuiCol",
        "ImGuiDir",
        "json"
	};
    for (const auto &tableKey : whitelistedTables)
    {
        sol::table table = globals[tableKey].get<sol::table>();
        sol::table tableCopy(m_lua, sol::create);
	    for (auto kv : table)
		    tableCopy[kv.first] = kv.second;
		m_env[tableKey] = tableCopy;
	}
    
    // copy sqlite3 without open functions
    {
        sol::table sqlite3 = globals["sqlite3"].get<sol::table>();
        sol::table sqlite3Copy(m_lua, sol::create);
	    for (auto kv : sqlite3)
	    {
            std::string keyStr = kv.first.as<std::string>();
            if (keyStr.compare(0, 4, "open"))
		        sqlite3Copy[kv.first] = kv.second;
	    }
		m_env["sqlite3"] = sqlite3Copy;
	}
    
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
    
    // copy safe io functions + replace selected unsafe ones
    {
        std::filesystem::path rootPath = m_path; // copy of path we can pass to lambda, as 'this' can be invalid afterwards
        sol::table io = globals["io"].get<sol::table>();
        sol::table ioCopy(m_lua, sol::create);
        ioCopy["close"] = io["close"];
        ioCopy["lines"] = [io, rootPath](std::string path)
        {
            auto absPath = absolute(rootPath / path).make_preferred();
            auto relPath = relative(absPath, path);
            auto relPathStr =  relPath.string();
            if (relPathStr.find("..") == std::string::npos)
                return io["lines"](absPath.string());
            return io["lines"](""); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
        ioCopy["open"] = [io, rootPath](std::string path, std::string mode)
        {
            auto absPath = absolute(rootPath / path).make_preferred();
            auto relPath = relative(absPath, path);
            auto relPathStr =  relPath.string();
            if (relPathStr.find("..") == std::string::npos)
                return io["open"](absPath.string(), mode);
            return io["open"]("", mode); // simulate invalid input even though it may be valid - we dont want mod access outside!
        };
		m_env["io"] = ioCopy;
	}

    // TODO - finish these + replacements for require/package
    /*{
        std::filesystem::path rootPath = m_path; // copy of path we can pass to lambda, as 'this' can be invalid afterwards
        sol::state_view lua = m_lua;
        sol::environment env = m_env;

        auto loadstring = [env, lua](const std::string& aStr, const std::string &aChunkName) -> std::tuple<sol::object, sol::object>
        {
	        if (!aStr.empty() && (aStr[0] == LUA_SIGNATURE[0]))
		        return std::make_tuple(sol::nil, sol::make_object(lua, "Bytecode prohibited!"));

	        auto result = lua.load(aStr, aChunkName, sol::load_mode::text);
	        if (result.valid())
            {
		        sol::function func = result.get<sol::function>();
		        env.set_on(func);
		        return std::make_tuple(func, sol::nil);
	        }

	        return std::make_tuple(sol::nil, sol::make_object(lua, result.get<sol::error>().what()));
        };

        auto loadfile = [lua, rootPath, loadstring](const std::string& acPath) -> std::tuple<sol::object, sol::object>
        {
            std::filesystem::path prefixedPath = absolute(rootPath / acPath);
	        if (!exists(prefixedPath))
		        return std::make_tuple(sol::nil, sol::make_object(lua, "Invalid path!"));

	        std::ifstream t(prefixedPath);
	        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	        return loadstring(str, "@" + prefixedPath.string());
        };
        m_env["loadfile"] = loadfile;

        auto dofile = [loadfile](const std::string& acPath) -> sol::object
        {
	        std::tuple<sol::object, sol::object> ret = loadfile(acPath);
	        if (std::get<0>(ret) == sol::nil)
		        throw sol::error(std::get<1>(ret).as<std::string>());

	        sol::function func = std::get<0>(ret);
	        return func().get<sol::object>();
        };
        m_env["dofile"] = dofile;
    }*/
    
    m_env["db"] = m_lua["sqlite3"]["open"]((m_path / "db").string()); // preassign database if mod requires it

    m_env["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
    {
        if(acName == "onInit")
            m_onInit = aCallback;
        else if(acName == "onShutdown")
            m_onShutdown = aCallback;
        else if(acName == "onUpdate")
            m_onUpdate = aCallback;
        else if(acName == "onDraw")
            m_onDraw = aCallback;
        else if(acName == "onOverlayOpen")
            m_onOverlayOpen = aCallback;
        else if(acName == "onOverlayClose")
            m_onOverlayClose = aCallback;
        else
            m_logger->error("Tried to register unknown handler '{}'!", acName);
    };

    m_env["registerHotkey"] = [this](const std::string& acID, const std::string& acDescription, sol::function aCallback)
    {
        if (acID.empty() ||
            (std::find_if(acID.cbegin(), acID.cend(), [](char c){ return !(isalpha(c) || isdigit(c) || c == '_'); }) != acID.cend()))
        {
            m_logger->error("Tried to register hotkey with incorrect ID format '{}'! ID needs to be alphanumeric without any whitespace or special characters (exception being '_' which is allowed in ID)!", acID);
            return;
        }

        if (acDescription.empty())
        {
            m_logger->error("Tried to register hotkey with empty description! (ID of hotkey handler: {})", acID);
            return;
        }

        auto loggerRef = m_logger;
        std::string vkBindID = m_name + '.' + acID;
        VKBind vkBind = { vkBindID, acDescription, [loggerRef, aCallback]()
        {
            // TODO: proper exception handling!
            try
            {
                if (aCallback)
                    aCallback();
            }
            catch(std::exception& e)
            {
                loggerRef->error(e.what());
            }
        }};
        m_vkBindInfos.emplace_back(VKBindInfo{vkBind});
    };

    // assign logger to mod so it can be used from within it too
    {
        auto loggerRef = m_logger; // ref for lambdas
        sol::table spdlog(m_lua, sol::create);
        spdlog["trace"] = [loggerRef](const std::string& message)
        {
            loggerRef->trace(message);
        };
        spdlog["debug"] = [loggerRef](const std::string& message)
        {
            loggerRef->debug(message);
        };
        spdlog["warning"] = [loggerRef](const std::string& message)
        {
            loggerRef->warn(message);
        };
        spdlog["error"] = [loggerRef](const std::string& message)
        {
            loggerRef->error(message);
        };
        spdlog["critical"] = [loggerRef](const std::string& message)
        {
            loggerRef->critical(message);
        };
        m_env["spdlog"] = spdlog;
    }

    // TODO: proper exception handling!
    try
    {
        const auto path = acPath / "init.lua";
        const auto result = m_lua.script_file(path.string(), m_env);

        if (result.valid())
        {
            m_initialized = true;
            m_object = result;
        }
        else
        {
            sol::error err = result;
            m_logger->error(err.what());
        }
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}

ScriptContext::ScriptContext(ScriptContext&& other) noexcept : ScriptContext(other)
{
    other.m_initialized = false;
}

ScriptContext::~ScriptContext()
{
    if (m_initialized)
        TriggerOnShutdown();

    m_logger->flush();
}

bool ScriptContext::IsValid() const
{
    return m_initialized;
}

const std::vector<VKBindInfo>& ScriptContext::GetBinds() const
{
    return m_vkBindInfos;
}

void ScriptContext::TriggerOnInit() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onInit)
            m_onInit();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}

void ScriptContext::TriggerOnUpdate(float aDeltaTime) const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onUpdate)
            m_onUpdate(aDeltaTime);
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}

void ScriptContext::TriggerOnDraw() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onDraw)
            m_onDraw();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}
    
void ScriptContext::TriggerOnOverlayOpen() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayOpen)
            m_onOverlayOpen();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}
void ScriptContext::TriggerOnOverlayClose() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onOverlayClose)
            m_onOverlayClose();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}

sol::object ScriptContext::GetRootObject() const
{
    return m_object;
}

void ScriptContext::TriggerOnShutdown() const
{
    // TODO: proper exception handling!
    try
    {
        if (m_onShutdown)
            m_onShutdown();
    }
    catch(std::exception& e)
    {
        m_logger->error(e.what());
    }
}
