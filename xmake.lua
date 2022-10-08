set_xmakever("2.6.0")

set_languages("c++20")
set_arch("x64")

add_requires("spdlog 1.9", "nlohmann_json", "hopscotch-map", "minhook", "mem", "imgui 1.84.2", "sol2", "tiltedcore 0.2.7", "sqlite3", "xbyak", "stb", "openrestry-luajit")
add_requireconfs("sol2", { configs = { includes_lua = false } })
add_requireconfs("openrestry-luajit", { configs = { gc64 = true } })

local imguiUserConfig = path.absolute("src/imgui_impl/imgui_user_config.h")
add_requireconfs("imgui", { configs = { user_config = imguiUserConfig } })

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("c.unity_build")

if is_mode("debug") then
    add_defines("CET_DEBUG")
    set_optimize("none")
elseif is_mode("releasedbg") then
    add_defines("CET_DEBUG")
    set_optimize("fastest")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
end

add_cxflags("/bigobj", "/MP")
add_defines("RED4EXT_STATIC_LIB", "UNICODE")

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })
  on_install(function() end)

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "WINVER=0x0601", "SOL_ALL_SAFETIES_ON", "SOL_LUAJIT=1", "SPDLOG_WCHAR_TO_UTF8_SUPPORT", "SPDLOG_WCHAR_FILENAMES", "SPDLOG_WCHAR_SUPPORT", "IMGUI_USER_CONFIG=\""..imguiUserConfig.."\"") -- WINVER=0x0601 == Windows 7xmake
    set_pcxxheader("src/stdafx.h")
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h", "build/CETVersion.h")
    add_includedirs("src/", "build/")
    add_syslinks("User32", "Version", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore", "sqlite3", "openrestry-luajit", "xbyak", "stb")
    add_deps("RED4ext.SDK")
	add_configfiles("src/CETVersion.h.in")

	on_package(function(target)
		os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.cp("vendor/asiloader/*", "package/bin/x64/")
		os.cp("LICENSE", "package/bin/x64/")
		os.cp("ThirdParty_LICENSES", "package/bin/x64/plugins/cyber_engine_tweaks/ThirdParty_LICENSES")
		os.cp(target:targetfile(), "package/bin/x64/plugins/")
		os.cp("scripts/*", "package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.cp("fonts/*", "package/bin/x64/plugins/cyber_engine_tweaks/fonts")
		os.rm("package/*.zip")
	end)
  on_install(function (target)
    cprint("${green bright}Installing Cyber Engine Tweaks ..")
    assert(os.isdir("$(installpath)"), format("The path in your configuration doesn't exist or isn't a directory.\n\tUse the follow command to set install path:\n\txmake f --installpath=%s", [["C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\bin\x64\plugins"]]))
    os.cp(target:targetfile(), "$(installpath)")
    cprint("Cyber Engine Tweaks installed at: ${underline}%s", "$(installpath)")
  end)

option("installpath")
  set_default("installpath")
  set_showmenu(true)
  set_description("Set the path to install cyber_engine_tweaks.asi to.", "e.g.", format("\t-xmake f --installpath=%s", [["C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\bin\x64\plugins"]]))

task("dephash")
	on_run(function ()
		import("core.project.project")
		import("private.action.require.impl.package")

		local requires, requires_extra = project.requires_str()

		local key = {}
		for _, instance in irpairs(package.load_packages(requires, {requires_extra = requires_extra})) do
			table.insert(key, instance:name() .. "-" .. instance:version_str() .. "-" .. instance:buildhash())
		end

		table.sort(key)

		key = table.concat(key, ",")
		print(hash.uuid4(key):gsub('-', ''):lower())
	end)

	set_menu {
		usage = "xmake dephash",
		description = "Outputs a hash key of current dependencies version/configuration"
	}
