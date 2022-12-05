/// @file Assert.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic run-time assert facilities with formatted error messages
/// @version 0.1
/// @date 2022-12-04
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
#pragma once

#include <Hyperion/HyperionDef.h>

#if HYPERION_HAS_SOURCE_LOCATION
#if HYPERION_PLATFORM_COMPILER_CLANG && HYPERION_PLATFORM_WINDOWS && !defined(__cpp_consteval)
IGNORE_RESERVED_IDENTIFIERS_START
IGNORE_RESERVED_MACRO_IDENTIFIERS_START
		// NOLINTNEXTLINE
		#define __cpp_consteval 201811L
IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP
IGNORE_RESERVED_IDENTIFIERS_STOP
#endif

	#include <source_location>
#elif HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
	// if we're using the experimental source location on an older GCC, alias it to the main std
	// namespace
	#include <experimental/source_location>
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#else
	#error "Hyperion requires std::source_location"
#endif // HYPERION_HAS_SOURCE_LOCATION

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/Platform.h>
#include <Hyperion/error/Backtrace.h>
#include <cassert>

namespace hyperion::error {

	IGNORE_UNUSED_MACROS_START

#if HYPERION_PLATFORM_DEBUG
	#define hyperion_assert(condition, format_string, ...) /** NOLINT(   		   **/          \
		/** cppcoreguidelines-macro-usage,                                         **/          \
		/** bugprone-reserved-identifier, cert-dcl37-c,                            **/          \
		/** cert-dcl51-cpp)                                                        **/          \
		[&]() {                                                                                 \
			if(!(condition)) {                                                                  \
				const auto location = std::source_location::current();                          \
				hyperion::eprintln("Assertion triggered at [{}:{}:{}: {}]: {}\bBacktrace:\n{}", \
						 location.file_name(),                                                  \
						 location.line(),                                                       \
						 location.column(),                                                     \
						 location.function_name(),                                              \
						 fmt::format(std::move(format_string) __VA_OPT__(, ) __VA_ARGS__),      \
						 hyperion::backtrace());                                                \
				assert(condition);                                                              \
			}                                                                                   \
		}()
#else // HYPERION_PLATFORM_DEBUG

	#define hyperion_assert(condition, format_string, ...) /** NOLINT(                       **/ \
														   /** cppcoreguidelines-macro-usage,**/ \
														   /** bugprone-reserved-identifier, **/ \
														   /** cert-dcl37-c,		         **/ \
														   /** cert-dcl51-cpp) 			     **/ \
		hyperion::ignore(condition, format_string __VA_OPT__(, ) __VA_ARGS__)
#endif // HYPERION_PLATFORM_DEBUG
// clang-format off

/// @def HYPERION_ASSERT(condition, format_string, ...)
/// @brief Conditionally triggers an assertion, printing the formatted error message along with
/// detailed source code location information of where the error occurred and a backtrace
///
/// Formats the error message via the given `format_string` and `format_args`, prepended with
/// the source code location and followed by a backtrace in the format:
/// @code 
/// Assertion triggered at [{file_name}:{line}:{column}: {function_name}]: {assertion_message}
/// Backtrace:
/// {Backtrace}
/// @endcode
///
/// @param condition - The assertion condition. Fires the assertion if `false`
/// @param format_string - The format string for generating the formatted error message
/// @param ... - The (possible/optional) arguments to format into the error message
/// @note This is only enabled in Debug builds, in non-debug builds this will resolve to a no-op
/// @note Please don't pass arguments with side-effects to this, at best you'll cause build
/// errors in release builds, and at worst you'll cause really hard to find bugs
///
/// @ingroup error
/// @headerfile "Hyperion/error/Assert.h"
#define HYPERION_ASSERT(condition, format_string, ...)/** NOLINT(cppcoreguidelines-macro-usage)**/ \
	hyperion_assert(condition, format_string __VA_OPT__(, ) __VA_ARGS__)

	IGNORE_UNUSED_MACROS_STOP
// clang-format on
} // namespace hyperion::error
