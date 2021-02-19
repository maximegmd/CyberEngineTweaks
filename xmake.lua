set_xmakever("2.5.1")

set_languages("cxx20")
set_arch("x64")

add_requires("spdlog", "nlohmann_json", "hopscotch-map", "minhook", "mem", "imgui 1.81", "sol2", "tiltedcore 0.2.0", "sqlite3")
add_requireconfs("imgui", { configs = {cxflags = "/DNDEBUG"} })

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") or is_mode("releasedbg") then
    add_defines("CET_DEBUG")

elseif is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")
    add_defines("NDEBUG")

    set_optimize("fastest")
end

add_cxflags("/bigobj", "/MP")
add_defines("RED4EXT_STATIC_LIB", "UNICODE")

before_build(function (target)
	import("modules.version")

	local branch, commitHash = version()

	local cetVersionString = string.format([[
#pragma once

#define CET_BUILD_BRANCH "%s"
#define CET_BUILD_COMMIT "%s"
]], branch, commitHash)
	local cetVersionCurrent = try
	{
		function()
			return io.open("src/CETVersion.h", "r")
		end
	}
	if cetVersionCurrent~=nil then
		local cetVersionCurrentString = cetVersionCurrent:read("*a")
		cetVersionCurrent:close()
		if (cetVersionCurrentString == cetVersionString) then
			return
		end
	end
	io.writefile("src/CETVersion.h", cetVersionString)
end)

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "SOL_ALL_SAFETIES_ON")
    set_pcxxheader("src/stdafx.h")
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.c", "src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/")
    add_syslinks("User32", "Version", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore", "sqlite3")
    add_deps("RED4ext.SDK")

	on_package(function(target)
		os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.cp("vendor/asiloader/*", "package/bin/x64/")
		os.cp("LICENSE", "package/bin/x64/")
		os.cp(target:targetfile(), "package/bin/x64/plugins/")
		os.cp("scripts/*", "package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.rm("package/*.zip")

		import("modules.version")

		local branch, commitHash = version()

		-- This is a hack provided by ruki until find_7z returns the full path
		--[[
		import("core.platform.environment")
		function do_zip()
			environment.enter("toolchains")
			os.cd("package/")
			os.vrunv("7z", {"a", "cet_"..commitHash..".zip", "."})
			environment.leave("toolchains")
		end

		do_zip()
		]]--

	end)
