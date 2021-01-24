set_xmakever("2.5.1")

set_languages("cxx20")

add_requires("spdlog", "nlohmann_json", "hopscotch-map", "minhook", "mem", "imgui", "sol2", "tiltedcore")
add_requireconfs("imgui", { configs = {cxflags = "/DNDEBUG"} })

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") or is_mode("releasedbg") then
    add_defines("CET_DEBUG")
end

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")
    add_defines("NDEBUG")

    set_optimize("fastest")
end

add_cxflags("/bigobj", "/MP")
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
    set_pcxxheader("src/stdafx.h") -- see: https://github.com/xmake-io/xmake/issues/1171#issuecomment-751421178
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.c")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/")
    add_syslinks("User32", "Version", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "hopscotch-map", "imgui", "mem", "sol2", "tiltedcore")
    add_deps("RED4ext.SDK")

	on_package(function(target)
		os.mkdir("package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.cp("vendor/asiloader/*", "package/bin/x64/")
		os.cp("LICENSE", "package/bin/x64/")
		os.cp(target:targetfile(), "package/bin/x64/plugins/")
		os.cp("scripts/*", "package/bin/x64/plugins/cyber_engine_tweaks/scripts")
		os.rm("package/*.zip")

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

		os.cd("package/")
		os.runv("zip", {"-r", "cet_"..commitHash..".zip", "."})
	end)
