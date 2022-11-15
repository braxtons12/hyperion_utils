package("hyperion-utils")
    set_homepage("https://braxtons12.github.io/Hyperion-Utils/")
    set_description("Collection of C++20 error handling, I/O, and logging utilities. Part of Hyperion Engine.")
    set_license("MIT")
    add_urls("../../../../", {alias = "local"})
    add_urls("https://github.com/braxtons12/Hyperion-Utils.git", {alias = "github"})

    add_configs("hyperion_tracy_enable", {description = "Enable Tracy profiling", default = false, values = {false, true}})
    add_deps("doctest", {
        system = false,
        external = true,
        configs = {
            languages = "cxx20"
        }
    })
    add_deps("fmt", {
        system = false,
        external = true,
        configs = {
            languages = "cxx20"
        }
    })
    add_deps("gsl", {
        system = false,
        external = true,
        configs = {
            languages = "cxx20"
        }
    })
    add_deps("boost", {
        system = false,
        external = true,
        configs = {
            container = true,
            exception = true,
            python = false,
            stacktrace = true,
            languages = "cxx20"
        },
    })
    add_deps("vcpkg::tracy", {
        alias = "tracy",
        system = false,
        external = true,
        configs = {
            languages = "cxx20"
        }
    })

    on_install(function(package)
        local configs = {}
        if package:config("hyperion_tracy_enable") then
            configs.hyperion_tracy_enable = true
        end
        import("package.tools.xmake").install(package, configs)
    end)
--add_deps("doctest", "fmt", "gsl", { system = false })
--add_deps("boost", {
--    alias = "boost",
--    system = false,
--    external = true,
--    configs = {
--        container = true,
--        exception = true,
--        python = false,
--        stacktrace = true,
--        languages = "cxx20"
--    },
--})
--add_deps("vcpkg::tracy", {
--    alias = "tracy",
--    system = false,
--    configs = {
--        languages = "cxx20"
--    },
--})

--on_load(function (package)
--    package:set("installdir", path.join(os.scriptdir(), package:plat(), package:arch(), package:mode()))
--end)

--on_fetch(function (package)
--    local result = {}
--    local libfiledir = (package:config("shared") and package:is_plat("windows", "mingw")) and "bin" or "lib"
--    result.links = "hyperion-utils"
--    result.linkdirs = package:installdir("lib")
--    result.includedirs = package:installdir("include")
--    result.libfiles = path.join(package:installdir(libfiledir), "libhyperion-utils.a")
--    return result
--end)
