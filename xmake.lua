---@diagnostic disable: undefined-global
set_project("hyperion-utils")
set_version("0.1.0")

set_languages("cxx20")

set_xmakever("2.6.9")

add_requires("doctest", {
    alias = "doctest",
    system = false,
    external = true,
    configs = {
        languages = "cxx20"
    }
})
add_requires("fmt", {
    alias = "fmt",
    system = false,
    external = true,
    configs = {
        languages = "cxx20"
    }
})
add_requires("gsl", {
    alias = "gsl",
    system = false,
    external = true,
    configs = {
        languages = "cxx20"
    }
})
add_requires("boost", {
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
add_requires("vcpkg::tracy", {
    alias = "tracy",
    system = false,
    external = true,
    configs = {
        languages = "cxx20"
    }
})

add_rules("mode.debug", "mode.release")

if is_mode("release") then
	add_defines("NDEBUG")
	set_optimize("aggressive")
	add_vectorexts("sse2", "sse3", "sse4", "avx", "avx2")
	set_strip("all")
elseif is_mode("debug") then
	add_defines("DEBUG")
	set_optimize("none")
end

if is_plat("windows") then
	add_defines(_CRT_SECURE_NO_WARNINGS)
end

if not is_plat("windows") then
	add_links("pthread", "atomic", { public = true })
end

option("hyperion_tracy_enable")
    add_defines(TRACY_ENABLE)
option_end()

local setup_compile_flags = function(target)
	if target:has_tool("cxx", "cl", "clang_cl") then
		target:add("cxflags", "/Zc:preprocessor", { public = true })
		target:add("cxflags", "/permissive-", { public = true })
		target:add("cxflags", "/Zc:rvalueCast", { public = true })
		target:add("cxflags", "/wd5104", { public = true })
		target:add("cxflags", "/WX", { public = false })
		target:add("cxflags", "/W4", { public = false })
		target:add("cxflags", "/MP", { public = false })
		target:add("cxflags", "/sdl", { public = false })
	else
		if target:has_tool("cxx", "clang", "clang++") then
			target:add("cxflags", "-Wno-c++98-compat", { public = true })
			target:add("cxflags", "-Wno-c++98-compat-pedantic", { public = true })
			target:add("cxflags", "-Wno-c++98-c++11-c++14-compat-pedantic", { public = true })
			target:add("cxflags", "-Wno-c++20-compat", { public = true })
			target:add("cxflags", "-Wno-gnu-zero-variadic-macro-arguments", { public = true })
			target:add("cxflags", "-Wno-undefined-func-template", { public = true })
			target:add("cxflags", "-Wno-ctad-maybe-unsupported", { public = true })
			target:add("cxflags", "-Wno-global-constructors", { public = true })
			target:add("cxflags", "-Wno-exit-time-destructors", { public = true })
			target:add("cxflags", "-Wno-extra-semi", { public = true })
			target:add("cxflags", "-Wno-extra-semi-stmt", { public = true })
			target:add("cxflags", "-Wno-unused-local-typedef", { public = true })
			target:add("cxflags", "-Weverything", { public = false })
		else
			target:add("cxflags", "-Wno-c++20-compat", { public = false })
			target:add("cxflags", "-Wno-terminate", { public = false })
			target:add("cxflags", "-Wno-extra-semi", { public = false })
		end
		if is_mode("release") then
			target:add("cxflags", "-Ofast")
			target:add("cxflags", "-ffast-math")
			target:add("cxflags", "-fomit-frame-pointer")
		end
		target:add("cxflags", "-Wall", { public = false })
		target:add("cxflags", "-Wextra", { public = false })
		target:add("cxflags", "-Wpedantic", { public = false })
		target:add("cxflags", "-Werror", { public = false })
	end
end

local setup_link_libs = function(target)
	if target:is_plat("windows") then
		target:add("links", "ole32", "dbgeng", { public = true })
		if target:has_tool("cxx", "gcc", "clang") then
			target:add("defines", "BOOST_STACKTRACE_USE_WINDBG")
		else
			target:add("defines", "BOOST_STACKTRACE_USE_WINDBG_CACHED")
		end
	else
		target:add("links", "dl", "backtrace", { public = true })
		target:add("defines", "BOOST_STACKTRACE_USE_BACKTRACE")
	end
end

local hyperion_utils_error_headers = {
	"$(projectdir)/include/Hyperion/error/Assert.h",
	"$(projectdir)/include/Hyperion/error/Backtrace.h",
	"$(projectdir)/include/Hyperion/error/GenericDomain.h",
	"$(projectdir)/include/Hyperion/error/NTDomain.h",
	"$(projectdir)/include/Hyperion/error/Panic.h",
	"$(projectdir)/include/Hyperion/error/PosixDomain.h",
	"$(projectdir)/include/Hyperion/error/StatusCode.h",
	"$(projectdir)/include/Hyperion/error/StatusCodeDomain.h",
	"$(projectdir)/include/Hyperion/error/SystemDomain.h",
	"$(projectdir)/include/Hyperion/error/Win32Domain.h",
}
local hyperion_utils_fs_headers = {
	"$(projectdir)/include/Hyperion/filesystem/File.h",
	"$(projectdir)/include/Hyperion/Filesystem.h",
}
local hyperion_utils_logging_headers = {
	"$(projectdir)/include/Hyperion/logging/Queue.h",
	"$(projectdir)/include/Hyperion/logging/Config.h",
	"$(projectdir)/include/Hyperion/logging/Entry.h",
	"$(projectdir)/include/Hyperion/logging/Sink.h",
}
local hyperion_utils_memory_headers = {
	"$(projectdir)/include/Hyperion/memory/CompressedPair.h",
	"$(projectdir)/include/Hyperion/memory/EmptyBaseClass.h",
	"$(projectdir)/include/Hyperion/memory/UniquePtr.h",
}
local hyperion_utils_mpl_headers = {
	"$(projectdir)/include/Hyperion/mpl/CallWithIndex.h",
	"$(projectdir)/include/Hyperion/mpl/Functions.h",
	"$(projectdir)/include/Hyperion/mpl/HasValue.h",
	"$(projectdir)/include/Hyperion/mpl/Identity.h",
	"$(projectdir)/include/Hyperion/mpl/Index.h",
	"$(projectdir)/include/Hyperion/mpl/List.h",
}
local hyperion_utils_enum_headers = {
	"$(projectdir)/include/Hyperion/enum/detail.h",
}
local hyperion_utils_option_headers = {
	"$(projectdir)/include/Hyperion/option/None.h",
	"$(projectdir)/include/Hyperion/option/SomeFWD.h",
	"$(projectdir)/include/Hyperion/option/OptionData.h",
}
local hyperion_utils_result_headers = {
	"$(projectdir)/include/Hyperion/result/Ok.h",
	"$(projectdir)/include/Hyperion/result/Err.h",
	"$(projectdir)/include/Hyperion/result/ResultData.h",
}
local hyperion_utils_sync_headers = {
	"$(projectdir)/include/Hyperion/synchronization/ReadWriteLock.h",
	"$(projectdir)/include/Hyperion/synchronization/ScopedLockGuard.h",
}
local hyperion_utils_main_headers = {
	"$(projectdir)/include/Hyperion/BasicTypes.h",
	"$(projectdir)/include/Hyperion/ChangeDetector.h",
	"$(projectdir)/include/Hyperion/Concepts.h",
	"$(projectdir)/include/Hyperion/Enum.h",
	"$(projectdir)/include/Hyperion/Error.h",
	"$(projectdir)/include/Hyperion/Filesystem.h",
	"$(projectdir)/include/Hyperion/Fmt.h",
	"$(projectdir)/include/Hyperion/FmtIO.h",
	"$(projectdir)/include/Hyperion/HyperionDef.h",
	"$(projectdir)/include/Hyperion/Ignore.h",
	"$(projectdir)/include/Hyperion/LockFreeQueue.h",
	"$(projectdir)/include/Hyperion/Logger.h",
	"$(projectdir)/include/Hyperion/Memory.h",
	"$(projectdir)/include/Hyperion/MPL.h",
	"$(projectdir)/include/Hyperion/Option.h",
	"$(projectdir)/include/Hyperion/Platform.h",
	"$(projectdir)/include/Hyperion/Result.h",
	"$(projectdir)/include/Hyperion/RingBuffer.h",
	"$(projectdir)/include/Hyperion/Span.h",
	"$(projectdir)/include/Hyperion/Synchronization.h",
	"$(projectdir)/include/Hyperion/TypeTraits.h",
	"$(projectdir)/include/Hyperion/Utils.h",
}

local hyperion_utils_sources = {
	"$(projectdir)/src/error/Panic.cpp",
	"$(projectdir)/src/filesystem/File.cpp",
	"$(projectdir)/src/Logger.cpp",
	"$(projectdir)/src/logging/Sink.cpp",
}

target("hyperion-utils")
	set_kind("static")
	add_includedirs("$(projectdir)/include")
	add_headerfiles(hyperion_utils_main_headers, { prefixdir = "Hyperion" })
	add_headerfiles(hyperion_utils_error_headers, { prefixdir = "Hyperion/error" })
	add_headerfiles(hyperion_utils_fs_headers, { prefixdir = "Hyperion/filesystem" })
	add_headerfiles(hyperion_utils_logging_headers, { prefixdir = "Hyperion/logging" })
	add_headerfiles(hyperion_utils_memory_headers, { prefixdir = "Hyperion/memory" })
	add_headerfiles(hyperion_utils_mpl_headers, { prefixdir = "Hyperion/mpl" })
	add_headerfiles(hyperion_utils_enum_headers, { prefixdir = "Hyperion/enum" })
	add_headerfiles(hyperion_utils_option_headers, { prefixdir = "Hyperion/option" })
	add_headerfiles(hyperion_utils_result_headers, { prefixdir = "Hyperion/result" })
	add_headerfiles(hyperion_utils_sync_headers, { prefixdir = "Hyperion/synchronization" })
	add_files(hyperion_utils_sources)
	add_packages("boost", "doctest", "fmt", "gsl", "tracy", {public = true})
	add_defines("DOCTEST_CONFIG_DISABLE")
	set_default(true)
	add_options("hyperion_tracy_enable")
	add_options("hyperion_disable_testing")
	on_config(function(target)
		setup_compile_flags(target)
		setup_link_libs(target)
	end)
    -- restore the package file after exporting,
    -- so that we don't have to manually reset the package dependencies
    after_package(function()
        local file_exists = function(name)
            local f = io.open(name, "r")
            if f ~= nil then
                f:close()
                return true
            else
                return false
            end
        end

        if file_exists("./export/packages/h/hyperion-utils/xmake.lua") then
            os.run("git restore export/packages/h/hyperion-utils/xmake.lua")
        end
    end)
target_end()

target("hyperion-utils-test")
	set_kind("binary")
	add_includedirs("$(projectdir)/include", { public = false })
	add_files(hyperion_utils_sources)
	add_files("src/doctest_main.cpp")
	add_packages("boost", "doctest", "fmt", "gsl", "tracy", {public = true})
	set_default(false)
	on_config(function(target)
		setup_compile_flags(target)
		setup_link_libs(target)
	end)
target_end()

target("hyperion-utils-benchmark")
	set_kind("binary")
	add_includedirs("$(projectdir)/include", { public = false })
	add_files(hyperion_utils_sources)
	add_files("src/benchmark.cpp")
	add_packages("boost", "doctest", "fmt", "gsl", "tracy", {public = true})
	add_defines("DOCTEST_CONFIG_DISABLE")
	set_default(false)
	on_config(function(target)
		setup_compile_flags(target)
		setup_link_libs(target)
	end)
target_end()

rule("clang-tidy-source")
	set_extensions(".c", ".cc", ".cpp", ".h", ".hpp", "")
	before_build_file(function(_, _, _)
	end)
	on_build_file(function(_, _, _)
	end)
	after_build_file(function(_, _, _)
	end)

target("clang-tidy")
	local total_files = #hyperion_utils_main_headers
					  + #hyperion_utils_error_headers
					  + #hyperion_utils_fs_headers
					  + #hyperion_utils_logging_headers
					  + #hyperion_utils_memory_headers
					  + #hyperion_utils_mpl_headers
					  + #hyperion_utils_enum_headers
					  + #hyperion_utils_option_headers
					  + #hyperion_utils_result_headers
					  + #hyperion_utils_sync_headers
					  + #hyperion_utils_sources
	local num_finished = 0
	set_default(false)
	add_includedirs("$(projectdir)/include", { public = false })
	add_files(hyperion_utils_sources, { rule = "clang-tidy-source" })
	add_files(hyperion_utils_main_headers, { prefixdir = "Hyperion", rule = "clang-tidy-source" })
	add_files(hyperion_utils_error_headers, { prefixdir = "Hyperion/error", rule = "clang-tidy-source" })
	add_files(hyperion_utils_fs_headers, { prefixdir = "Hyperion/filesystem", rule = "clang-tidy-source" })
	add_files(hyperion_utils_logging_headers, { prefixdir = "Hyperion/logging", rule = "clang-tidy-source" })
	add_files(hyperion_utils_memory_headers, { prefixdir = "Hyperion/memory", rule = "clang-tidy-source" })
	add_files(hyperion_utils_mpl_headers, { prefixdir = "Hyperion/mpl", rule = "clang-tidy-source" })
	add_files(hyperion_utils_enum_headers, { prefixdir = "Hyperion/enum", rule = "clang-tidy-source" })
	add_files(hyperion_utils_option_headers, { prefixdir = "Hyperion/option", rule = "clang-tidy-source" })
	add_files(hyperion_utils_result_headers, { prefixdir = "Hyperion/result", rule = "clang-tidy-source" })
	add_files(hyperion_utils_sync_headers, { prefixdir = "Hyperion/synchronization", rule = "clang-tidy-source" })
	add_packages("boost", "doctest", "fmt", "gsl", "tracy", {public = true})
	before_build_file(function(_, source_file, _)
		print("[" .. math.floor(100 * ((num_finished + .0) / total_files)) .. "%] running clang-tidy on " .. source_file)
	end)
	on_build_file(function(_, source_file, _)
		local command = "clang-tidy --config-file=$(projectdir)/.clang-tidy " .. source_file
		os.run(command)
	end)
	after_build_file(function(_, source_file, _)
		num_finished = num_finished + 1
		print("[" .. math.floor(100 * ((num_finished + .0) / total_files)) .. "%] finished clang-tidy on " .. source_file)
	end)
	on_link(function(_)
	end)
target_end()
