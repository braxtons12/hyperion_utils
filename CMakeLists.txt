cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(FetchContent)

project(HyperionUtils VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_C_EXTENSIONS OFF)

#############################################################################
# Import GoogleTest
#############################################################################
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
	message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
	message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
	${CMAKE_CURRENT_BINARY_DIR}/googletest-build
	EXCLUDE_FROM_ALL)
#############################################################################
#############################################################################


#############################################################################
# Import Microsoft GSL Implementation
#############################################################################
FetchContent_Declare(GSL
	GIT_REPOSITORY "https://github.com/microsoft/GSL"
	GIT_TAG "v3.1.0"
	)

FetchContent_MakeAvailable(GSL)

add_library(HyperionUtils INTERFACE)
add_library(HyperionUtilsLib STATIC)

if(MSVC)
	target_compile_options(HyperionUtils INTERFACE /WX /W4 /std:c++20)
	target_compile_options(HyperionUtilsLib PRIVATE /WX /W4 /std:c++20)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	if(NOT WIN32)
		target_compile_options(HyperionUtils INTERFACE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			)
		target_compile_options(HyperionUtilsLib PRIVATE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			)
	else()
		target_compile_options(HyperionUtils INTERFACE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			)
		target_compile_options(HyperionUtilsLib PRIVATE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			)
	endif()
else()
	target_compile_options(HyperionUtils INTERFACE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat-pedantic
		-Wno-c++20-compat
		)
	target_compile_options(HyperionUtilsLib PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat-pedantic
		-Wno-c++20-compat
		)
endif()

target_include_directories(HyperionUtils BEFORE INTERFACE
	"${CMAKE_SOURCE_DIR}/include"
	)
target_include_directories(HyperionUtilsLib BEFORE INTERFACE
	"${CMAKE_SOURCE_DIR}/include"
	)

###### We add headers to sources sets because it helps with `#include` lookup for some tooling #####

set(IMPLS
	"${CMAKE_SOURCE_DIR}/src/Logger.cpp"
	)

set(EXPORTS
	"${CMAKE_SOURCE_DIR}/include/ChangeDetector.h"
	"${CMAKE_SOURCE_DIR}/include/Concepts.h"
	"${CMAKE_SOURCE_DIR}/include/Error.h"
	"${CMAKE_SOURCE_DIR}/include/Ignore.h"
	"${CMAKE_SOURCE_DIR}/include/LockFreeQueue.h"
	"${CMAKE_SOURCE_DIR}/include/Logger.h"
	"${CMAKE_SOURCE_DIR}/include/Macros.h"
	"${CMAKE_SOURCE_DIR}/include/OptionAndResult.h"
	"${CMAKE_SOURCE_DIR}/include/RingBuffer.h"
	"${CMAKE_SOURCE_DIR}/include/Span.h"
	"${CMAKE_SOURCE_DIR}/include/TypeTraits.h"
	"${CMAKE_SOURCE_DIR}/include/synchronization/ReadWriteLock.h"
	"${CMAKE_SOURCE_DIR}/include/synchronization/ScopedLockGuard.h"
	"${CMAKE_SOURCE_DIR}/include/HyperionUtils.h"
	)

target_sources(HyperionUtils INTERFACE
	${EXPORTS}
	${IMPLS}
	)

target_sources(HyperionUtilsLib PUBLIC
	${EXPORTS}
	${IMPLS}
	)

target_link_libraries(HyperionUtils INTERFACE
	GSL)

target_link_libraries(HyperionUtilsLib PUBLIC
	GSL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang" OR APPLE)
	set_target_properties(HyperionUtilsLib PROPERTIES CXX_CLANG_TIDY clang-tidy)
endif()


add_executable(Test "${CMAKE_SOURCE_DIR}/src/test/Test.cpp")

if(MSVC)
	target_compile_options(Test PRIVATE /WX /W4 /std:c++20)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	if(NOT WIN32)
		target_compile_options(Test PRIVATE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			-Wno-global-constructors
			-Wno-used-but-marked-unused
			-Wno-covered-switch-default
			-Wno-missing-prototypes
			)
	else()
		target_compile_options(Test PRIVATE
			-std=c++20
			-Wall
			-Wextra
			-Wpedantic
			-Weverything
			-Werror
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-c++98-c++11-c++14-compat-pedantic
			-Wno-c++20-compat
			-Wno-global-constructors
			-Wno-used-but-marked-unused
			-Wno-covered-switch-default
			-Wno-missing-prototypes
			)
	endif()
else()
	target_compile_options(Test PRIVATE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat-pedantic
		-Wno-c++20-compat
		-Wno-global-constructors
		-Wno-used-but-marked-unused
		-Wno-covered-switch-default
		-Wno-missing-prototypes
		)
endif()

target_include_directories(Test BEFORE INTERFACE
	"${CMAKE_SOURCE_DIR}/src/test"
	)

if(UNIX AND NOT APPLE)
	target_link_libraries(Test PRIVATE
		curl
		GSL
		gtest)
else()
	target_link_libraries(Test PRIVATE
		GSL
		gtest)
endif()
