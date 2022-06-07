cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(FetchContent)

project(HyperionUtils VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

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
	GIT_TAG "8.1.1"
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
	libs/integer
	libs/predef
	libs/preprocessor
	libs/stacktrace
	libs/static_assert
	libs/throw_exception
	libs/type_traits
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
	GIT_TAG "origin/master"
	)

FetchContent_MakeAvailable(tracy)
#############################################################################
#############################################################################


set(EXPORTS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/ChangeDetector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Hyperion/Concepts.h"
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
add_library(Hyperion::Utils ALIAS HyperionUtils)

add_executable(HyperionUtilsTest
	"${CMAKE_CURRENT_SOURCE_DIR}/src/doctest_main.cpp"
	)

add_executable(HyperionUtilsLoggerBenchmark
	"${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark.cpp"
	)

if(MSVC)
	target_sources(HyperionUtils INTERFACE ${EXPORTS})
endif()

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
		INTERFACE
		/Zc:preprocessor
		/permissive-
		/Zc:rvalueCast
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
		-Wno-extra-semi
		-Wno-extra-semi-stmt
		-Wno-unused-local-typedef
		)
	target_compile_options(HyperionUtilsTest
		PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Weverything
		-Werror
		)
	target_compile_options(HyperionUtilsLoggerBenchmark
		PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Weverything
		-Werror
		)
else ()
	target_compile_options(HyperionUtils
		INTERFACE
		-Wno-c++20-compat
		-Wno-terminate
		-Wno-extra-semi
		)
	target_compile_options(HyperionUtilsTest
		PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		)
	target_compile_options(HyperionUtilsLoggerBenchmark
		PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		)
endif ()

target_compile_features(HyperionUtils INTERFACE "cxx_std_20")

target_include_directories(HyperionUtils INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	)

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

##### We add includes to sources sets because it helps with `#include` lookup for some tooling #####

#target_sources(HyperionUtils INTERFACE
#        ${EXPORTS}
#        )

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
		${TRACY}
		${HYPERION_UTILS_BACKTRACE}
		${HYPERION_UTILS_LINK_LIBS}
		)
else ()
	SET(HYPERION_UTILS_LINK_LIBS
		GSL
		fmt::fmt
		doctest::doctest
		${TRACY}
		${HYPERION_UTILS_BACKTRACE}
		${HYPERION_UTILS_LINK_LIBS}
		)
endif ()

target_compile_definitions(HyperionUtils INTERFACE
	${HYPERION_UTILS_COMPILE_DEFINITIONS}
	)

target_link_libraries(HyperionUtils INTERFACE
	${HYPERION_UTILS_LINK_LIBS}
	)
target_link_libraries(HyperionUtilsTest PRIVATE
	HyperionUtils
	)
target_link_libraries(HyperionUtilsLoggerBenchmark PRIVATE
	HyperionUtils
	)
#target_compile_options(HyperionUtilsLoggerBenchmark PUBLIC -fsanitize=thread)
#target_link_options(HyperionUtilsLoggerBenchmark PUBLIC -fsanitize=thread)
#target_compile_options(HyperionUtilsTest PUBLIC -fsanitize=thread)
#target_link_options(HyperionUtilsTest PUBLIC -fsanitize=thread)

find_package(Doxygen REQUIRED)

set(MCSS_DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-mcss.in")
set(MCSS_DOXYFILE_OUT "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-mcss")
set(MCSS_CONF_PY "${CMAKE_CURRENT_SOURCE_DIR}/conf.py")
set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/Cnx")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
set(DOXYGEN_INDEX_FILE "${DOXYGEN_OUTPUT_DIR}/xml/index.xml")
set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
set(DOXYFILE_OUT "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")

set(DOXYGEN_HTML "${DOXYGEN_OUTPUT_DIR}/html/index.html")

set(COMPILATION_DATABASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DOXYGEN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(DOXYGEN_COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/include")

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
configure_file(${MCSS_DOXYFILE_IN} ${MCSS_DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

add_custom_command(OUTPUT ${DOXYGEN_HTML}
	DEPENDS ${EXPORTS} ${CMAKE_CURRENT_SOURCE_DIR}/README.md
	COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/third-party/m.css/documentation/doxygen.py ${MCSS_CONF_PY} --debug
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN} ${MCSS_DOXYFILE_OUT} ${MCSS_DOXYFILE_IN}
	COMMENT "Generating themed docs with m.css"
	VERBATIM)

add_custom_target(HyperionUtilsDoxygen DEPENDS ${DOXYGEN_HTML})
set_target_properties(HyperionUtilsDoxygen PROPERTIES EXCLUDE_FROM_ALL TRUE)

include(GNUInstallDirs)
install(DIRECTORY ${DOXYGEN_OUTPUT_DIR} DESTINATION ${CMAKE_INSTALL_DOCDIR})

enable_testing()

add_test(test HyperionUtilsTest)
