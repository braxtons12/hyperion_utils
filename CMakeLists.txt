cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(FetchContent)

project(HyperionUtils VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_CXX_STANDARD 20)

#############################################################################
# Import Microsoft GSL Implementation
#############################################################################
FetchContent_Declare(GSL
	GIT_REPOSITORY "https://github.com/microsoft/GSL"
	GIT_TAG "v4.0.0"
	)

FetchContent_MakeAvailable(GSL)
#############################################################################
#############################################################################

#############################################################################
# Import fmtlib/fmt
#############################################################################
FetchContent_Declare(fmt
	GIT_REPOSITORY "https://github.com/fmtlib/fmt"
	GIT_TAG "9.0.0"
	)

FetchContent_MakeAvailable(fmt)
#############################################################################
#############################################################################

#############################################################################
# Import boost to get boost::stacktrace
#############################################################################
SET(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(boost
	GIT_REPOSITORY "https://github.com/boostorg/boost"
	GIT_TAG "boost-1.79.0"
	GIT_SUBMODULES
	libs/array
	libs/assert
	libs/config
	libs/container_hash
	libs/core
	libs/detail
	libs/function_types
	libs/fusion
	libs/hana
	libs/integer
	libs/io
	libs/mpl
	libs/predef
	libs/preprocessor
	libs/stacktrace
	libs/static_assert
	libs/throw_exception
	libs/tuple
	libs/typeof
	libs/type_traits
	libs/utility
	libs/winapi
	tools/cmake
	)
FetchContent_MakeAvailable(boost)
#FetchContent_GetProperties(boost)
#if(NOT boost_POPULATED)
#    FetchContent_Populate(boost)
#    add_subdirectory(
#            ${boost_SOURCE_DIR}
#            ${boost_BINARY_DIR}
#            EXCLUDE_FROM_ALL
#    )
#endif()

#############################################################################
#############################################################################

#############################################################################
# Import doctest
#############################################################################
FetchContent_Declare(doctest
	GIT_REPOSITORY "https://github.com/doctest/doctest"
	GIT_TAG "origin/dev"
	)

FetchContent_MakeAvailable(doctest)
#############################################################################
#############################################################################

#############################################################################
# Import tracy
#############################################################################
FetchContent_Declare(tracy
	GIT_REPOSITORY "https://github.com/wolfpld/tracy"
    GIT_TAG "v0.8.2"
	)

FetchContent_MakeAvailable(tracy)
#############################################################################
#############################################################################

set(HYPERION_UTILS_EXPORTS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/ChangeDetector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Concepts.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/enum/detail.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Enum.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/Assert.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/error/Backtrace.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Fmt.h"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/Functions.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/HasValue.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/mpl/Identity.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Testing.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/TypeTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Utils.h"
	)
set(HYPERION_UTILS_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/error/Panic.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem/File.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/logging/Sink.cpp"
	)
set(HYPERION_UTILS_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/doctest_main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/ChangeDetector.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/Enum.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/LockFreeQueue.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/Option.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/Result.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/RingBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tests/memory/UniquePtr.cpp"
    )

add_library(HyperionUtils STATIC)
add_library(Hyperion::Utils ALIAS HyperionUtils)

add_executable(HyperionUtilsTest
    ${HYPERION_UTILS_TEST_SOURCES}
	)

add_executable(HyperionUtilsLoggerBenchmark
	"${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark.cpp"
	)

set_target_properties(HyperionUtilsTest PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(HyperionUtilsLoggerBenchmark PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_sources(HyperionUtils PRIVATE ${HYPERION_UTILS_SOURCES})
##### We add includes to sources sets because it helps with `#include` lookup for some tooling #####
target_sources(HyperionUtils PUBLIC ${HYPERION_UTILS_EXPORTS})
target_include_directories(HyperionUtils PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	if (NOT DEFINED CMAKE_CXX_CLANG_TIDY)
		SET(CMAKE_CXX_CLANG_TIDY clang-tidy)
	endif ()
	set_target_properties(HyperionUtils PROPERTIES CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
	set_target_properties(HyperionUtilsTest PROPERTIES CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
	set_target_properties(HyperionUtilsLoggerBenchmark PROPERTIES CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
endif ()

if (MSVC)
	target_compile_options(HyperionUtils
		PUBLIC
		/Zc:preprocessor
		/permissive-
		/Zc:rvalueCast
        /Zc:__cplusplus
		/wd5104
		)
	target_compile_options(HyperionUtilsTest
		PRIVATE
		/WX
		/W4
		/MP
		/sdl
		)
	target_compile_options(HyperionUtilsLoggerBenchmark
		PRIVATE
		/WX
		/W4
		/MP
		/sdl
		)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	target_compile_options(HyperionUtils
		PUBLIC
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat-pedantic
		-Wno-c++20-compat
		-Wno-gnu-zero-variadic-macro-arguments
		-Wno-undefined-func-template
		-Wno-ctad-maybe-unsupported
		-Wno-global-constructors
		-Wno-exit-time-destructors
		-Wno-extra-semi
		-Wno-extra-semi-stmt
		-Wno-unused-local-typedef
		)
	target_compile_options(HyperionUtilsTest
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Weverything
		-Werror
		)
	target_compile_options(HyperionUtilsLoggerBenchmark
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Weverything
		-Werror
		)
else ()
	target_compile_options(HyperionUtils
		PUBLIC
		-Wno-c++20-compat
		-Wno-terminate
		-Wno-extra-semi
		)
	target_compile_options(HyperionUtilsTest
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		)
	target_compile_options(HyperionUtilsLoggerBenchmark
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		)
endif ()

target_compile_features(HyperionUtils PUBLIC cxx_std_20)

target_compile_definitions(HyperionUtils PRIVATE DOCTEST_CONFIG_DISABLE)
target_compile_definitions(HyperionUtilsLoggerBenchmark PUBLIC DOCTEST_CONFIG_DISABLE)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	if (APPLE AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
		target_compile_options(HyperionUtilsTest PRIVATE -mcpu=apple-a14 -mtune=native)
		target_compile_options(HyperionUtilsLoggerBenchmark PRIVATE -mcpu=apple-a14 -mtune=native)
	elseif(NOT MSVC)
		target_compile_options(HyperionUtilsTest PRIVATE -march=native -mtune=native)
		target_compile_options(HyperionUtilsLoggerBenchmark PRIVATE -march=native -mtune=native)
	endif ()
endif()

if (HYPERION_TRACY_ENABLE)
	SET(TRACY Tracy::TracyClient)
endif ()

if (WIN32 OR MSVC)
	SET(HYPERION_UTILS_COMPILE_DEFINITIONS
		_CRT_SECURE_NO_WARNINGS)
	SET(HYPERION_UTILS_LINK_LIBS
		ole32
		dbgeng
		${HYPERION_UTILS_LINK_LIBS}
		)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		SET(HYPERION_UTILS_BACKTRACE
			Boost::stacktrace_windbg
			)
		SET(HYPERION_UTILS_COMPILE_DEFINITIONS
			BOOST_STACKTRACE_USE_WINDBG
			${HYPERION_UTILS_COMPILE_DEFINITIONS}
			)
	else ()
		SET(HYPERION_UTILS_BACKTRACE
			Boost::stacktrace_windbg_cached
			)
		SET(HYPERION_UTILS_COMPILE_DEFINITIONS
			BOOST_STACKTRACE_USE_WINDBG_CACHED
			${HYPERION_UTILS_COMPILE_DEFINITIONS}
			)
	endif ()
else ()
	SET(HYPERION_UTILS_BACKTRACE
		Boost::stacktrace_backtrace
		)
	SET(HYPERION_UTILS_COMPILE_DEFINITIONS
		BOOST_STACKTRACE_USE_BACKTRACE
		${HYPERION_UTILS_COMPILE_DEFINITIONS}
		)
	SET(HYPERION_UTILS_LINK_LIBS
		dl
		backtrace
		${HYPERION_UTILS_LINK_LIBS}
		)
endif ()
SET(HYPERION_UTILS_COMPILE_DEFINITIONS
	BOOST_STACKTRACE_LINK
	${HYPERION_UTILS_COMPILE_DEFINITIONS}
	)

if (UNIX)
	SET(HYPERION_UTILS_LINK_LIBS
		GSL
		fmt::fmt
		pthread
		doctest::doctest
		atomic
        Boost::hana
		${TRACY}
		${HYPERION_UTILS_BACKTRACE}
		${HYPERION_UTILS_LINK_LIBS}
		)
else ()
	SET(HYPERION_UTILS_LINK_LIBS
		GSL
		fmt::fmt
		doctest::doctest
        Boost::hana
		${TRACY}
		${HYPERION_UTILS_BACKTRACE}
		${HYPERION_UTILS_LINK_LIBS}
		)
endif ()

target_compile_definitions(HyperionUtils PUBLIC
	${HYPERION_UTILS_COMPILE_DEFINITIONS}
	)

target_link_libraries(HyperionUtils PUBLIC
	${HYPERION_UTILS_LINK_LIBS}
	)

target_link_libraries(HyperionUtilsTest PRIVATE
	HyperionUtils
	)
include("cmake/doctest_force_link_static_lib_in_target.cmake")
doctest_force_link_static_lib_in_target(HyperionUtilsTest HyperionUtils)

target_link_libraries(HyperionUtilsLoggerBenchmark PRIVATE
	HyperionUtils
	)

#target_compile_options(HyperionUtilsLoggerBenchmark PUBLIC -fsanitize=thread)
#target_link_options(HyperionUtilsLoggerBenchmark PUBLIC -fsanitize=thread)
#target_compile_options(HyperionUtilsTest PUBLIC -fsanitize=thread)
#target_link_options(HyperionUtilsTest PUBLIC -fsanitize=thread)

set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs/_build/html")
set(DOXYGEN_HTML "${DOXYGEN_OUTPUT_DIR}/index.html")

set(DOCS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
set(DOCS_FILES
    "${DOCS_DIR}/index.rst"
    "${DOCS_DIR}/enum.rst"
    "${DOCS_DIR}/quick_start.rst"
    "${DOCS_DIR}/error_handling/index.rst"
    "${DOCS_DIR}/error_handling/Assert.rst"
    "${DOCS_DIR}/error_handling/Error.rst"
    "${DOCS_DIR}/error_handling/Option.rst"
    "${DOCS_DIR}/error_handling/Panic.rst"
    "${DOCS_DIR}/error_handling/Result.rst"
    "${DOCS_DIR}/error_handling/StatusCode.rst"
    "${DOCS_DIR}/error_handling/status_codes/GenericDomain.rst"
    "${DOCS_DIR}/error_handling/status_codes/NTDomain.rst"
    "${DOCS_DIR}/error_handling/status_codes/PosixDomain.rst"
    "${DOCS_DIR}/error_handling/status_codes/StatusCode.rst"
    "${DOCS_DIR}/error_handling/status_codes/StatusCodeDomain.rst"
    "${DOCS_DIR}/error_handling/status_codes/SystemDomain.rst"
    "${DOCS_DIR}/error_handling/status_codes/Win32Domain.rst"
    "${DOCS_DIR}/filesystem/index.rst"
    "${DOCS_DIR}/filesystem/File.rst"
    )

add_custom_command(OUTPUT ${DOXYGEN_HTML}
    DEPENDS ${HYPERION_UTILS_EXPORTS} ${DOCS_FILES}
    COMMAND sphinx-build -M html . _build
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
	MAIN_DEPENDENCY
	COMMENT "Generating documentation with doxygen, sphinx, and breathe"
	VERBATIM)

add_custom_target(HyperionUtilsDoxygen DEPENDS ${DOXYGEN_HTML})
set_target_properties(HyperionUtilsDoxygen PROPERTIES EXCLUDE_FROM_ALL TRUE)

include(GNUInstallDirs)
install(DIRECTORY ${DOXYGEN_OUTPUT_DIR} DESTINATION ${CMAKE_INSTALL_DOCDIR})

enable_testing()

add_test(test HyperionUtilsTest)
