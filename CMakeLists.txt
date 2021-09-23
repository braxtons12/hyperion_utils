cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(FetchContent)

project(HyperionUtils VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

SET(CMAKE_CXX_STANDARD 20)
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

SET(CMAKE_CXX_CLANG_TIDY clang-tidy)

add_library(HyperionUtils INTERFACE)
# Dummy library. A cheap hack to make tooling find fmt and gsl headers
add_library(HyperionUtils.headers STATIC EXCLUDE_FROM_ALL)

if(MSVC)
	target_compile_options(HyperionUtils.headers PRIVATE /WX /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	target_compile_options(HyperionUtils
		INTERFACE
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-c++98-c++11-c++14-compat-pedantic
		-Wno-c++20-compat
		-Wno-gnu-zero-variadic-macro-arguments
		-Wno-undefined-func-template
		)
	target_compile_options(HyperionUtils.headers PRIVATE
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
		)
else()
	target_compile_options(HyperionUtils
		INTERFACE
		-Wno-c++20-compat
		-Wno-gnu-zero-variadic-macro-arguments
		)
	target_compile_options(HyperionUtils.headers
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++20-compat
		-Wno-gnu-zero-variadic-macro-arguments
		)
endif()

target_include_directories(HyperionUtils INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	)

target_include_directories(HyperionUtils.headers PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils"
	)

###### We add headers to sources sets because it helps with `#include` lookup for some tooling #####

set(EXPORTS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/ChangeDetector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Concepts.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/Assert.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/GenericDomain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/NTDomain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/Panic.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/PosixDomain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/StatusCode.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/StatusCodeDomain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/SystemDomain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/error/Win32Domain.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Error.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Format.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/HyperionDef.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Ignore.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/LockFreeQueue.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/Config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/Entry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/Sink.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/logging/SinkBase.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Logger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/memory/CompressedPair.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/memory/EmptyBaseClass.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/memory/UniquePtr.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Memory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/mpl/Callable.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/mpl/CallWithIndex.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/mpl/ForAll.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/mpl/IndexType.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/mpl/List.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/MPL.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/option/None.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/option/OptionData.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Option.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Platform.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/result/Ok.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/result/Err.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/result/ResultData.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Result.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/RingBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Span.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/synchronization/ReadWriteLock.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/synchronization/ScopedLockGuard.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/Synchronization.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/TypeTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperionUtils/HyperionUtils.h"
	)

target_sources(HyperionUtils INTERFACE
	${EXPORTS}
	)

target_sources(HyperionUtils.headers PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/empty_cpp_for_tooling.cpp"
	${EXPORTS}
	)

if(UNIX)
	target_link_libraries(HyperionUtils INTERFACE
		GSL
		fmt::fmt
		pthread
		)
	target_link_libraries(HyperionUtils.headers PUBLIC
		GSL
		fmt::fmt
		pthread
		)
else()
	target_link_libraries(HyperionUtils INTERFACE
		GSL
		fmt::fmt
		)
	target_link_libraries(HyperionUtils.headers PUBLIC
		GSL
		fmt::fmt
		)
endif()


add_subdirectory("docs")
