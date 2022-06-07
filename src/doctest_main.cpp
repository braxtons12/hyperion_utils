/// @file doctest_main.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Main file implementation for Hyperion's tests
/// @version 0.1
/// @date 2022-06-04
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#include <Hyperion/Platform.h>

#if HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wunused-macros\"")
#endif // HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC

#if HYPERION_PLATFORM_COMPILER_CLANG
	_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wreserved-identifier\"")
#elif HYPERION_PLATFORM_COMPILER_MSVC
_Pragma("warning( push )") _Pragma("warning( disable : 4405 )")
#endif // HYPERION_PLATFORM_COMPILER_CLANG

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_NO_COMPARISON_WARNING_SUPPRESSION
#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#if HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
		_Pragma("GCC diagnostic pop")
#endif // HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
#if HYPERION_PLATFORM_COMPILER_CLANG
			_Pragma("GCC diagnostic pop")
#elif HYPERION_PLATFORM_COMPILER_MSVC
	_Pragma("warning( pop )")
#endif // HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC

#include <Hyperion/Utils.h>
