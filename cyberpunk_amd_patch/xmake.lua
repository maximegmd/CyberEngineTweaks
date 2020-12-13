set_languages("cxx17")

add_requires("zlib", "spdlog")

target("performance_overhaul")
    set_kind("shared")
    add_files("src/**.cpp")
    add_includedirs("src/", {public = true})
    add_syslinks("User32")
    add_packages("zlib", "spdlog")