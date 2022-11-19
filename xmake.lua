set_xmakever("2.7.2")

set_languages("cxx20")
set_arch("x64")

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("c.unity_build")

add_cxflags("/bigobj", "/MP")
add_defines("RED4EXT_STATIC_LIB", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS")

if is_mode("debug") then
    add_defines("CET_DEBUG")
    set_symbols("debug")
    set_optimize("none")
    set_runtimes("MDd")
    set_warnings("all")
    set_policy("build.optimization.lto", false)
elseif is_mode("releasedbg") then
    add_defines("CET_DEBUG")
    set_symbols("debug")
    set_optimize("fastest")
    set_runtimes("MD")
    set_warnings("all")
    set_policy("build.optimization.lto", true)
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_symbols("hidden")
    set_strip("all")
    set_optimize("fastest")
    set_runtimes("MD")
    set_warnings("all", "error")
    set_policy("build.optimization.lto", true)
end

local imguiUserConfig = path.absolute("src/imgui/imgui_user_config.h")

add_requireconfs("*", { debug = is_mode("debug"), lto = not is_mode("debug"), configs = { shared = false } })
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
add_requires("imgui v1.89-docking", { configs = { user_config = imguiUserConfig } })

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })
    on_install(function() end)

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "WINVER=0x0601", "SOL_ALL_SAFETIES_ON", "SOL_LUAJIT=1", "SOL_EXCEPTIONS_SAFE_PROPAGATION", "SPDLOG_WCHAR_TO_UTF8_SUPPORT", "SPDLOG_WCHAR_FILENAMES", "SPDLOG_WCHAR_SUPPORT", "IMGUI_USER_CONFIG=\""..imguiUserConfig.."\"") -- WINVER=0x0601 == Windows 7xmake
    set_pcxxheader("src/stdafx.h")
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h", "build/CETVersion.h")
    add_includedirs("src/", "build/")
    add_syslinks("User32", "Version", "d3d11", "dxgi")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore", "sqlite3", "openrestry-luajit", "xbyak", "stb")
    add_deps("RED4ext.SDK")
    add_configfiles("src/CETVersion.h.in")

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

        os.cp(target:targetfile(), "package/bin/x64/plugins/")
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
