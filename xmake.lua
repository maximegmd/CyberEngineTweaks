set_xmakever("2.7.2")

set_languages("cxx20")
set_arch("x64")

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("c.unity_build")

add_cxflags("/bigobj", "/MP", "/EHsc")
add_defines("UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS")

if is_mode("debug") then
    set_optimize("none")
    set_warnings("all")
    set_policy("build.optimization.lto", false)
elseif is_mode("releasedbg") then
    set_optimize("fastest")
    set_warnings("all")
    set_policy("build.optimization.lto", true)
elseif is_mode("release") then
    set_strip("all")
    set_optimize("fastest")
    set_warnings("all", "error")
    set_policy("build.optimization.lto", true)
end

set_symbols("debug")
set_runtimes(is_mode("release") and "MD" or "MDd");

add_requireconfs("**", { configs = {
    --debug = is_mode("debug"), -- This seems to cause compilation issues recently and probably has little benefit anyway
    lto = not is_mode("debug"),
    shared = false,
    vs_runtime = is_mode("release") and "MD" or "MDd" } })

add_requires("spdlog 1.11.0")
add_requires("nlohmann_json")
add_requires("hopscotch-map")
add_requires("minhook")
add_requires("mem")
add_requires("tiltedcore 0.2.7")
add_requires("sqlite3")
add_requires("xbyak")
add_requires("stb")
add_requires("sol2", { configs = { includes_lua = false } })
add_requires("openrestry-luajit", { configs = { gc64 = true } })

local imguiUserConfig = string.gsub(path.absolute("src/imgui_impl/imgui_user_config.h"), "\\", "/")
add_requires("imgui v1.91.1-docking", { configs = { wchar32 = true, freetype = true, user_config = imguiUserConfig } })

target("RED4ext.SDK")
    set_kind("headeronly")
    set_group("vendor")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })
    on_install(function() end)

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "WINVER=0x0601", "SOL_ALL_SAFETIES_ON", "SOL_LUAJIT=1", "SOL_EXCEPTIONS_SAFE_PROPAGATION", "SPDLOG_WCHAR_TO_UTF8_SUPPORT", "SPDLOG_WCHAR_FILENAMES", "SPDLOG_WCHAR_SUPPORT", "IMGUI_USER_CONFIG=\""..imguiUserConfig.."\"") -- WINVER=0x0601 == Windows 7xmake
    set_pcxxheader("src/stdafx.h")
    set_kind("shared")

    set_configdir(path.join("src", "config"))
    add_configfiles("src/xmake/**.in")

    add_files("src/**.cpp", "src/**.rc")
    add_headerfiles("src/**.h")
    add_includedirs("src/", "build/")
    add_syslinks("User32", "Version", "d3d11", "dxgi")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore", "sqlite3", "openrestry-luajit", "xbyak", "stb")
    add_deps("RED4ext.SDK")

    -- Set up basic config variables.
    -- Required for us to set up something here first to be able to access and modify "configvars"
    -- in `on_load`, there seems to be no way to do this otherwise at the moment.
    set_configvar("CET_PRODUCT_NAME", "Cyber Engine Tweaks")

    on_load(function(target)
        -- Set filename based on project name
        target:set("basename", target:name())
        target:set("extension", ".asi")

        -- Get a list of existing target config variables.
        local configVars = target:get("configvar")

        -- Set up other basic config variables.
        configVars.CET_CURRENT_YEAR = tonumber(os.date("%Y"))
        configVars.CET_GROUP_NAME = configVars.CET_PRODUCT_NAME .. " Team"
        configVars.CET_FILE_NAME = target:filename();

        -- Set up build type.
        if is_mode("release") then
          configVars.CET_DEBUG_BUILD = 0
        else
          configVars.CET_DEBUG_BUILD = 1
          target:add("defines", "CET_DEBUG")
        end

        -- Emulate GIT_<XXX> XMake Git builtins so we can parse them ourselves.
        -- Unable to use and access builtins unfortunately. This is taken directly from XMake source.
        configVars.CET_GIT_TAG = os.iorun("git describe --tags"):gsub("%s", "")
        configVars.CET_GIT_TAG_LONG = os.iorun("git describe --tags --long"):gsub("%s", "")
        configVars.CET_GIT_BRANCH = os.iorun("git rev-parse --abbrev-ref HEAD"):gsub("%s", "")
        configVars.CET_GIT_COMMIT = os.iorun("git rev-parse --short HEAD"):gsub("%s", "")
        configVars.CET_GIT_COMMIT_LONG = os.iorun("git rev-parse HEAD"):gsub("%s", "")
        configVars.CET_GIT_COMMIT_DATE = os.iorun("git log -1 --date=format:%Y%m%d%H%M%S --format=%ad"):gsub("%s", "")

        -- Split tag so we can easily extract version from it.
        local splitGitTag = configVars.CET_GIT_TAG:split("%.")
        assert(#splitGitTag == 3)

        -- Setup version variables.
        configVars.CET_VERSION_MAJOR = tonumber(splitGitTag[1]:match("%d+"))
        configVars.CET_VERSION_MINOR = tonumber(splitGitTag[2]:match("%d+"))
        configVars.CET_VERSION_PATCH = tonumber(splitGitTag[3]:match("%d+"))
        configVars.CET_VERSION_SIMPLE = format(
            "%d.%d.%d",
            configVars.CET_VERSION_MAJOR,
            configVars.CET_VERSION_MINOR,
            configVars.CET_VERSION_PATCH)
        configVars.CET_VERSION_FULL = format(
            "%s [%s]",
            configVars.CET_GIT_TAG,
            configVars.CET_GIT_BRANCH)
        configVars.CET_VERSION = configVars.CET_VERSION_FULL

        -- Push new list to target.
        target:add("configvar", configVars)
    end)

    on_package(function(target)
        import("net.http")

        os.rm("package/*")

        os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/tweakdb")
        http.download("https://github.com/WolvenKit/WolvenKit/raw/main/WolvenKit.Common/Resources/usedhashes.kark", "package/bin/x64/plugins/cyber_engine_tweaks/tweakdb/usedhashes.kark")
        http.download("https://github.com/WolvenKit/WolvenKit/raw/main/WolvenKit.Common/Resources/tweakdbstr.kark", "package/bin/x64/plugins/cyber_engine_tweaks/tweakdb/tweakdbstr.kark")

        os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/scripts")
        os.cp("scripts/*", "package/bin/x64/plugins/cyber_engine_tweaks/scripts")

        os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/fonts")
        os.cp("fonts/*", "package/bin/x64/plugins/cyber_engine_tweaks/fonts")

        os.cp("vendor/asiloader/*", "package/bin/x64/")

        os.cp("LICENSE", "package/bin/x64/")
        os.cp("ThirdParty_LICENSES", "package/bin/x64/plugins/cyber_engine_tweaks/ThirdParty_LICENSES")

        local target_file = target:targetfile()

        os.cp(target_file, "package/bin/x64/plugins/")

        os.cp(path.join(
            path.directory(target_file),
            path.basename(target_file)..".pdb"
        ), "package/bin/x64/plugins/")
    end)
    on_install(function(target)
        cprint("${green bright}Installing Cyber Engine Tweaks ..")
        assert(os.isdir("$(installpath)"), format("The path in your configuration doesn't exist or isn't a directory.\n\tUse the follow command to set install path:\n\txmake f --installpath=%s", [["C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\bin\x64\plugins"]]))
        os.cp(target:targetfile(), "$(installpath)")
        cprint("Cyber Engine Tweaks installed at: ${underline}%s", "$(installpath)")
    end)

option("installpath")
    set_default("installpath")
    set_showmenu(true)
    set_description("Set the path to install cyber_engine_tweaks.asi to.", "e.g.", format("\t-xmake f --installpath=%s", [["C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\bin\x64\plugins"]]))
