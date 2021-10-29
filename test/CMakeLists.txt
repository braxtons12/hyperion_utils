cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
include(FetchContent)
include(ExternalProject)

project(HyperionUtilsTest VERSION 0.1.0)

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
# Import microTest
#############################################################################
FetchContent_Declare(microTest
	GIT_REPOSITORY "https://github.com/boost-ext/ut"
	GIT_TAG "v1.1.8"
	)

FetchContent_MakeAvailable(microTest)
#############################################################################
#############################################################################

#############################################################################
# Import Hyperion from the parent directory
#############################################################################
FetchContent_Declare(HyperionUtils SOURCE_DIR "${CMAKE_SOURCE_DIR}/../"
	BINARY_DIR "${CMAKE_SOURCE_DIR}/../build")
FetchContent_MakeAvailable(HyperionUtils)
#############################################################################
#############################################################################

add_executable(Test "${CMAKE_SOURCE_DIR}/src/Test.cpp")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang" OR APPLE)
	set_target_properties(Test PROPERTIES CXX_CLANG_TIDY clang-tidy)
endif()

if(MSVC)
	target_compile_options(Test PRIVATE /WX /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "clang")
	target_compile_options(Test PRIVATE
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
		-ferror-limit=100
		)
else()
	target_compile_options(Test PRIVATE
		-Wall
		-Wextra
		-Wpedantic
		-Werror
		-Wno-c++20-compat
		-fconcepts-diagnostics-depth=4
		)
endif()

target_include_directories(Test BEFORE INTERFACE
	"${CMAKE_SOURCE_DIR}/"
	)

if(UNIX AND NOT APPLE)
	target_link_libraries(Test PRIVATE
		curl
		boost::ut
		HyperionUtils
		)
else()
	target_link_libraries(Test PRIVATE
		GSL
		boost::ut
		fmt::fmt
		HyperionUtils)
endif()
