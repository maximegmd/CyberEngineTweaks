set_languages("cxx17")

add_requires("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore", {configs = {cxflags = "/DNDEBUG"}, external = false }) -- configs = {cxflags = "/DNDEBUG"} should not be needed when 'debug' is 'false' (default), but for some reason we still pull in debug packages!!!

add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("CET_DEBUG")
end

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")
    add_defines("NDEBUG")

    set_optimize("fastest")
end

add_cxflags("/std:c++latest", "/bigobj", "/MP")
add_defines("RED4EXT_STATIC_LIB", "UNICODE")

before_build(function (target)
		local host = os.host()
		local subhost = os.subhost()

		local system
		if (host ~= subhost) then
			system = host .. "/" .. subhost
		else
			system = host
		end

		local branch = "unknown-branch"
		local commitHash = "unknown-commit"
		try
		{
			function ()
				import("detect.tools.find_git")
				local git = find_git()
				if (git) then
					branch = os.iorunv(git, {"rev-parse", "--abbrev-ref", "HEAD"}):trim()
					commitHash = os.iorunv(git, {"describe", "--tags"}):trim()
				else
					error("git not found")
				end
			end,

			catch
			{
				function (err)
					print(string.format("Failed to retrieve git data: %s", err))
				end
			}
		}

		io.writefile("src/CETVersion.h", string.format([[
#pragma once

#define CET_BUILD_BRANCH "%s";
#define CET_BUILD_COMMIT "%s";
#define CET_BUILD_TIME "%s";
]], branch, commitHash, os.date("%Y-%m-%d %H:%M:%S")))
	end)

target("RED4ext.SDK")
    set_kind("static")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("cyber_engine_tweaks")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "SOL_ALL_SAFETIES_ON")
    set_pcxxheader("src/stdafx.h") -- see: https://github.com/xmake-io/xmake/issues/1171#issuecomment-751421178
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.c")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/")
    add_syslinks("User32", "Version", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore")
    add_deps("RED4ext.SDK")

--[[ target("luasocket")
    set_kind("static")
    add_defines("LUASOCKET_API=;")
    add_packages("lua")
    add_files("vendor/luasocket/src/*.c")
    del_files("vendor/luasocket/src/unix*.c", "vendor/luasocket/src/usocket.c", "vendor/luasocket/src/serial.c")
    add_headerfiles("vendor/luasocket/src/*.h")
    add_includedirs("vendor/luasocket/src/")
    add_syslinks("ws2_32")
]]--