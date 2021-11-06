cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(FetchContent)

project(HyperionUtils VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

if (NOT MSVC)
    SET(CMAKE_CXX_STANDARD 20)
endif ()
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_C_STANDARD 17)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_C_EXTENSIONS OFF)

#############################################################################
# Import Microsoft GSL Implementation
#############################################################################
FetchContent_Declare(GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v3.1.0"
        )

FetchContent_MakeAvailable(GSL)
#############################################################################
#############################################################################

#############################################################################
# Import fmtlib/fmt
#############################################################################
FetchContent_Declare(fmt
        GIT_REPOSITORY "https://github.com/fmtlib/fmt"
        GIT_TAG "8.0.1"
        )

FetchContent_MakeAvailable(fmt)
#############################################################################
#############################################################################

#############################################################################
# Import doctest
#############################################################################
FetchContent_Declare(doctest
        GIT_REPOSITORY "https://github.com/onqtam/doctest"
        GIT_TAG "2.4.6"
        )

FetchContent_MakeAvailable(doctest)
#############################################################################
#############################################################################

if(NOT MSVC)
    SET(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

set(EXPORTS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/ChangeDetector.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Concepts.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/Assert.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/GenericDomain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/NTDomain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/Panic.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/PosixDomain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/StatusCode.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/StatusCodeDomain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/SystemDomain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/Win32Domain.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Error.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/filesystem/File.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Filesystem.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Format.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/HyperionDef.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Ignore.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/logging/Queue.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/logging/Config.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/logging/Entry.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/logging/Sink.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Logger.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/memory/CompressedPair.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/memory/EmptyBaseClass.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/memory/UniquePtr.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Memory.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/CallWithIndex.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/ForAll.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/HasValue.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/Index.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/List.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/MPL.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/option/None.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/option/SomeFWD.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/option/OptionData.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Option.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Platform.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/result/Ok.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/result/Err.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/result/ResultData.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Result.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/RingBuffer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Span.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/synchronization/ReadWriteLock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/synchronization/ScopedLockGuard.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Synchronization.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/TypeTraits.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Utils.h"
        )

add_library(HyperionUtils INTERFACE)
# Dummy library. A cheap hack to make tooling find fmt and gslTest
add_executable(HyperionUtilsTest
        "${CMAKE_CURRENT_SOURCE_DIR}/src/doctest_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/test/OptionAndResultTests.h"
        ${EXPORTS}
        )

add_executable(HyperionUtilsBenchmark
        "${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark.cpp"
        ${EXPORTS}
        )

if (MSVC)
    target_compile_options(HyperionUtilsTest PRIVATE /WX /W4 /std:c++latest /Zc:preprocessor /permissive- /Zc:rvalueCast /MP /sdl)
    target_compile_options(HyperionUtilsBenchmark PRIVATE /WX /W4 /std:c++latest /Zc:preprocessor /permissive- /Zc:rvalueCast /MP /sdl)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
    target_compile_options(HyperionUtils
            INTERFACE
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++98-c++11-c++14-compat-pedantic
            -Wno-c++20-compat
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-undefined-func-template
            -Wno-ctad-maybe-unsupported
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-extra-semi-stmt
            )
    target_compile_options(HyperionUtilsTest PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Weverything
            -Werror
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++98-c++11-c++14-compat-pedantic
            -Wno-c++20-compat
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-undefined-func-template
            -Wno-ctad-maybe-unsupported
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-extra-semi-stmt
            )
    target_compile_options(HyperionUtilsBenchmark PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Weverything
            -Werror
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++98-c++11-c++14-compat-pedantic
            -Wno-c++20-compat
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-undefined-func-template
            -Wno-ctad-maybe-unsupported
            -Wno-global-constructors
            -Wno-exit-time-destructors
            -Wno-extra-semi-stmt
            -Wno-keyword-macro
            )
else ()
    target_compile_options(HyperionUtils
            INTERFACE
            -Wno-c++20-compat
            -Wno-terminate
            )
    target_compile_options(HyperionUtilsTest
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-c++20-compat
            -Wno-terminate
            )
    target_compile_options(HyperionUtilsBenchmark
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-c++20-compat
            -Wno-terminate
            )
endif ()

target_include_directories(HyperionUtils INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_include_directories(HyperionUtilsTest PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_include_directories(HyperionUtilsBenchmark PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_compile_definitions(HyperionUtils INTERFACE DOCTEST_CONFIG_DISABLE)
target_compile_definitions(HyperionUtilsBenchmark PUBLIC DOCTEST_CONFIG_DISABLE)

###### We addTest to sources sets because it helps with `#include` lookup for some tooling #####

target_sources(HyperionUtils INTERFACE
        ${EXPORTS}
        )

if (UNIX)
    target_link_libraries(HyperionUtils INTERFACE
            GSL
            fmt::fmt
            pthread
            doctest::doctest
            )
    target_link_libraries(HyperionUtilsTest PUBLIC
            GSL
            fmt::fmt
            pthread
            doctest::doctest
            )
    target_link_libraries(HyperionUtilsBenchmark PUBLIC
            GSL
            fmt::fmt
            pthread
            doctest::doctest
            )
else ()
    target_link_libraries(HyperionUtils INTERFACE
            GSL
            fmt::fmt
            doctest::doctest
            )
    target_link_libraries(HyperionUtilsTest PUBLIC
            GSL
            fmt::fmt
            doctest::doctest
            )
    target_link_libraries(HyperionUtilsBenchmark PUBLIC
            GSL
            fmt::fmt
            doctest::doctest
            )
endif ()


add_subdirectory("docs")

enable_testing()

add_test(test HyperionUtilsTest)
