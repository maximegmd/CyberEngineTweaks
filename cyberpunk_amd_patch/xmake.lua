set_languages("cxx17")

add_requires("zlib", "spdlog", "nlohmann_json")

add_rules("mode.debug", "mode.release")
add_cxflags("-flto")
add_ldflags("-flto")

set_optimize("fastest")


target("performance_overhaul")
    set_kind("shared")
    add_files("src/**.cpp")
    add_includedirs("src/")
    add_syslinks("User32")
    add_packages("zlib", "spdlog", "nlohmann_json")
