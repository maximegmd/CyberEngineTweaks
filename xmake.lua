set_xmakever("2.7.2")

set_languages("cxx20")
set_arch("x64")

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("c.unity_build")

add_cxflags("/bigobj", "/MP")
add_defines("RED4EXT_STATIC_LIB", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS")

local vsRuntime = "MD"

if is_mode("debug") then
    add_defines("CET_DEBUG")
    set_symbols("debug")
    set_optimize("none")
    set_warnings("all")
    set_policy("build.optimization.lto", false)

    vsRuntime = vsRuntime.."d"
elseif is_mode("releasedbg") then
    add_defines("CET_DEBUG")
    set_symbols("debug")
    set_optimize("fastest")
    set_warnings("all")
    set_policy("build.optimization.lto", true)

    vsRuntime = vsRuntime.."d"
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_symbols("hidden")
    set_strip("all")
    set_optimize("fastest")
    set_runtimes("MD")
    set_warnings("all", "error")
    set_policy("build.optimization.lto", true)
end

set_runtimes(vsRuntime);

add_requireconfs("*", { configs = { debug = is_mode("debug"), lto = not is_mode("debug"), shared = false, vs_runtime = vsRuntime } })

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

local imguiUserConfig = path.absolute("src/imgui_impl/imgui_user_config.h")
add_requires("imgui v1.88-docking", { configs = { wchar32 = true, freetype = true, user_config = imguiUserConfig } })
add_requires("libiconv 1.17")

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })
    on_install(function() end)

target("tinygettext")
    add_defines("WIN32")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/tinygettext/src/**.cpp")
    add_headerfiles("vendor/tinygettext/include/**.hpp", "vendor/tinygettext/include/**.h")
    add_includedirs("vendor/tinygettext/include/", { public = true })
    add_packages("libiconv")
    on_install(function() end)

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "WINVER=0x0601", "SOL_ALL_SAFETIES_ON", "SOL_LUAJIT=1", "SOL_EXCEPTIONS_SAFE_PROPAGATION", "SPDLOG_WCHAR_TO_UTF8_SUPPORT", "SPDLOG_WCHAR_FILENAMES", "SPDLOG_WCHAR_SUPPORT", "IMGUI_USER_CONFIG=\""..imguiUserConfig.."\"", "SOL_IMGUI_ENABLE_FONT_MANIPULATORS") -- WINVER=0x0601 == Windows 7xmake
    set_pcxxheader("src/stdafx.h")
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h", "build/CETVersion.h")
    add_includedirs("src/", "build/")
    add_syslinks("User32", "Version", "d3d11", "Dwrite")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore", "sqlite3", "openrestry-luajit", "xbyak", "stb", "libiconv")
    add_deps("RED4ext.SDK", "tinygettext")
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

        os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/languages")
        os.cp("languages/*.po", "package/bin/x64/plugins/cyber_engine_tweaks/languages")

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

task("i18n-pot")
    set_menu {
        usage = "xmake i18n-pot",
        description = "Generate update existing po file.",
        options = {}
    }
    on_run(function()
        local potPath = "./languages/template.pot"
        -- get a list of cpp files under src/
        local fileList = ""
        for _, filePath in ipairs(os.files("src/**.cpp")) do
            fileList = fileList .. filePath .. "\n"
        end
        -- write the list to a tmp file
        local fileListPath = "$(tmpdir)/cet_cpp_files.txt"
        local file = io.open(fileListPath, "w")
        if file then
            file:write(fileList)
            file:close()
        end

        -- execute xgettext
        os.execv("vendor/gettext-tools/xgettext.exe", {
            vformat("--files-from=%s", fileListPath),
            vformat("--output=%s", potPath),
            "--c++",
            "--from-code=UTF-8",
            "--add-comments",
            "--no-wrap",
            "--keyword",
            "--keyword=_t:1,1t", "--keyword=Translate:1,1t", "--keyword=translate:1,1t",
            "--keyword=_t:1c,2,2t", "--keyword=Translate:1c,2,2t", "--keyword=translate_ctxt:1,1t",
            "--keyword=_t:1,2,3t", "--keyword=Translate:1,2,3t", "--keyword=translate_plural:1,1t",
            "--keyword=_t:1c,2,3,4t", "--keyword=Translate:1c,2,3,4t", "--keyword=translate_ctxt_plural:1,1t",
            "--copyright-holder=yamashi",
            "--package-name=CyberEngineTweaks",
            "--package-version=",
            "--msgid-bugs-address=https://github.com/maximegmd/CyberEngineTweaks/issues"
        })
        -- remove the tmp file
        os.rm(fileListPath)

        -- auto generate and update en.po
        local enpoPath = "./languages/en.po"
        os.execv("vendor/gettext-tools/msginit.exe", {
            format("--input=%s", potPath),
            format("--output-file=%s.tmp", enpoPath),
            format("--locale=%s", "en"),
            "--no-translator",
            "--no-wrap",
        })
        os.execv("vendor/gettext-tools/msgmerge.exe", {
            "--update",
            "--backup=off",
            "--no-fuzzy-matching",
            "--no-wrap",
            enpoPath,
            enpoPath..".tmp"
        })
        os.rm(enpoPath..".tmp")

    end)

task("i18n-po")
    set_menu {
        usage = "xmake i18n-po [options]",
        description = "Generate new / update existing po file.",
        options = {
            {'m', "mode", "kv", "update", "Create new or update existing po file.", " - create", " - update"},
            {'l', "locale", "kv", nil, "Set locale to create. Use ll_CC or ll format (e.g. en_US, or en)."},
        }
    }
    on_run(function()
        import("core.base.option")
        local mode = option.get("mode")
        local locale = option.get("locale")
        
        local langDir = "./languages/"
        local potPath = langDir .. "template.pot"

        if mode ~= "create" and mode ~= "update" then
            raise("Unkown value %s for --mode.", mode)
        end
        if mode == "create" and (locale == "" or locale == nil) then
            raise("Locale can't be empty.")
        end
        if (not os.exists(potPath)) then
            raise("template.pot doesn't exist.")
        end

        -- create new po file
        if mode == "create" then
            os.execv("vendor/gettext-tools/msginit.exe", {
                format("--input=%s", potPath),
                format("--output-file=%s", langDir..locale..".po"),
                format("--locale=%s", locale),
                "--no-wrap",
            })
        end

        -- update all po files
        if mode == "update" then
            for _, poPath in ipairs(os.files(langDir.."*.po")) do
                cprint("${green bright}Updating${clear} %s", poPath)

                if path.basename(poPath) ~= "en" then
                    os.execv("vendor/gettext-tools/msgmerge.exe", {
                        "--update",
                        "--backup=off",
                        "--no-wrap",
                        poPath,
                        potPath
                    })
                end
            end
        end
    end)

    task("i18n-install")
        set_menu {
        usage = "xmake i18n-install",
        description = "Install po files to CET install path.",
        options = {}
        }
        on_run(function()
            import("core.project.config")
            config.load()
            local installPath = config.get("installpath")
            local langPath = path.join(installPath, "cyber_engine_tweaks", "languages")

            cprint("${green bright}Installing language files ..")
            assert(os.isdir(installPath), format("The path in your configuration doesn't exist or isn't a directory.\n\tUse the follow command to set install path:\n\txmake f --installpath=%s", [["C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\bin\x64\plugins"]]))
            
            os.mkdir(langPath)
            os.cp("./languages/*.po", langPath)

            cprint("po files installed at: ${underline}%s", langPath)
        end)