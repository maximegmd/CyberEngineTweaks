package("scnlib")

    set_homepage("https://scnlib.readthedocs.io/")
    set_description("scnlib is a modern C++ library for replacing scanf and std::istream")

    set_urls("https://github.com/eliaskosunen/scnlib/archive/refs/tags/v$(version).zip")
    add_versions("0.4", "49a84f1439e52666532fbd5da3fa1d652622fc7ac376070e330e15c528d38190")
    add_versions("1.1.2", "72BF304662B03E00DE5B438B9D4697A081E786D589E067817C356174FB2CB06C")

    add_configs("header_only", {description = "Use header only version.", default = false, type = "boolean"})

    on_load(function (package)
        if package:config("header_only") then
            package:add("defines", "SCN_HEADER_ONLY=1")
        else
            package:add("deps", "cmake")
        end
    end)

    on_install(function (package)
        if package:config("header_only") then
            os.cp("include/scn", package:installdir("include"))
            return
        end
        local configs = {"-DSCN_TESTS=OFF", "-DSCN_DOCS=OFF", "-DSCN_EXAMPLES=OFF", "-DSCN_BENCHMARKS=OFF", "-DSCN_PENDANTIC=OFF", "-DSCN_BUILD_FUZZING=OFF"}
        import("package.tools.cmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <scn/scn.h>
            #include <cstdio>

            static void test() {
                int i;
                scn::prompt("What's your favorite number? ", "{}", i);
                printf("Oh, cool, %d!", i);
            }
        ]]}, {configs = {languages = "c++17"}, includes = "scn/scn.h"}))
    end)