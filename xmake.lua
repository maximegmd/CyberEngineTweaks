set_languages("cxx17")

add_requires("mimalloc", {configs = {cxflags = "-DMI_USE_RTLGENRANDOM"}})
add_requires("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore", { external = false })

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")

    set_optimize("fastest")
end

add_cxflags("/std:c++latest")

target("RED4ext")
    set_kind("static")
    add_files("vendor/RED4ext/src/red4ext.sdk/**.cpp")
    add_headerfiles("vendor/RED4ext/src/red4ext.sdk/**.hpp")
    add_includedirs("vendor/RED4ext/src/red4ext.sdk/", { public = true })

target("cyber_engine_tweaks")
    add_defines("KIERO_USE_MINHOOK=1", "KIERO_INCLUDE_D3D12=1")
    set_kind("shared")
    set_filename("cyber_engine_tweaks.asi")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/")
    add_syslinks("User32", "d3d11")
    add_packages("spdlog", "nlohmann_json", "minhook", "imgui", "sol2", "tiltedcore")
    add_deps("RED4ext")
