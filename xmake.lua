set_languages("cxx20")

add_requires("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore", { external = false })

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")

    set_optimize("fastest")
end

add_cxflags("/std:c++latest")
add_defines("RED4EXT_STATIC_LIB")

target("RED4ext.SDK")
    set_kind("static")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("cyber_engine_tweaks")
    add_defines("KIERO_USE_MINHOOK=1", "KIERO_INCLUDE_D3D12=1", "IMGUI_IMPL_WIN32_DISABLE_GAMEPAD", "WIN32_LEAN_AND_MEAN", "NOMINMAX")
    set_pcxxheader("src/stdafx.h") -- see: https://github.com/xmake-io/xmake/issues/1171#issuecomment-751421178
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/")
    add_syslinks("User32", "Version", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore")
    add_deps("RED4ext.SDK")
