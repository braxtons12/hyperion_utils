package("hyperion-utils")
    set_description("Collection of C++20 error handling, I/O, and logging utilities. Part of Hyperion Engine.")

    add_deps("doctest", "fmt", "gsl", { system = false })
    add_deps("boost", {
        alias = "boost",
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
    
    on_load(function (package)
        package:set("installdir", path.join(os.scriptdir(), package:plat(), package:arch(), package:mode()))
    end)

    on_fetch(function (package)
        local result = {}
        local libfiledir = (package:config("shared") and package:is_plat("windows", "mingw")) and "bin" or "lib"
        result.links = "hyperion-utils"
        result.linkdirs = package:installdir("lib")
        result.includedirs = package:installdir("include")
        result.libfiles = path.join(package:installdir(libfiledir), "libhyperion-utils.a")
        return result
    end)
