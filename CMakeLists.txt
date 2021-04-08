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
	GIT_TAG "7.1.3"
	)

FetchContent_MakeAvailable(fmt)
#############################################################################
#############################################################################

SET(CMAKE_CXX_CLANG_TIDY clang-tidy)

add_library(HyperionUtils INTERFACE)
# Dummy library. A cheap hack to make tooling find fmt and gsl headers
add_library(HyperionUtils.headers STATIC EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/src/empty_cpp_for_tooling.cpp")

if(MSVC)
	target_compile_options(HyperionUtils INTERFACE /WX /W4 /std:c++20)
	target_compile_options(HyperionUtils.headers INTERFACE /WX /W4 /std:c++20)
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
		target_compile_options(HyperionUtils.headers INTERFACE
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
		target_compile_options(HyperionUtils.headers INTERFACE
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
		-Wno-c++20-compat
		)
	target_compile_options(HyperionUtils.headers INTERFACE
		-std=c++20
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++20-compat
		)
endif()

target_include_directories(HyperionUtils BEFORE INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	)

target_include_directories(HyperionUtils.headers BEFORE INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/synchronization"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/detail"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/logging"
	)

###### We add headers to sources sets because it helps with `#include` lookup for some tooling #####

set(EXPORTS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/ChangeDetector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Concepts.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Error.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Ignore.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/LockFreeQueue.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Logger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Macros.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/OptionAndResult.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/RingBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Span.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/TypeTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/synchronization/ReadWriteLock.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/synchronization/ScopedLockGuard.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/detail/AllocateUnique.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/Config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/Entry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/HyperionUtils.h"
	)

target_sources(HyperionUtils INTERFACE
	${EXPORTS}
	)

target_link_libraries(HyperionUtils INTERFACE
	GSL
	)

target_link_libraries(HyperionUtils.headers PUBLIC
	fmt::fmt
	GSL
	)
