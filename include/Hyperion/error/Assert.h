/// @file Assert.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic run-time assert facilities with formatted error messages
/// @version 0.1
/// @date 2022-06-05
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
#include <cassert>

#if HYPERION_HAS_SOURCE_LOCATION
	#include <source_location>
#elif HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
	// if we're using the experimental source location on an older GCC, alias it to the main std
	// namespace
	#include <experimental/source_location>
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#endif // HYPERION_HAS_SOURCE_LOCATION

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Platform.h>
#include <Hyperion/error/Backtrace.h>

namespace hyperion::error {

	IGNORE_UNUSED_MACROS_START

#if HYPERION_PLATFORM_DEBUG

	#if HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION

	/// @brief Triggers an assertion, printing the formatted error message along with detailed
	/// source code location information of where the error occurred
	///
	/// Formats the error message via the given `format_string` and `format_args`, prepended with
	/// the source code location in the format "[file:line:column: function]: message"
	///
	/// @tparam Args - The types of the arguments to format into the error message
	///
	/// @param format_string - The format string for generating the formatted error message
	/// @param location - The source code location the error occurred
	/// @param format_args - The arguments to format into the error message
	/// @ingroup error
	/// @headerfile "Hyperion/error/Assert.h"
	/// @note Only enabled in Debug builds
	template<typename... Args>
	[[noreturn]] inline constexpr auto hyperion_assert(fmt::format_string<Args...>&& format_string,
													   const std::source_location location,
													   Args&&... format_args) noexcept -> void {
		eprintln("Assertion triggered at [{}:{}:{}: {}]: {}\bBacktrace:\n{}",
				 location.file_name(),
				 location.line(),
				 location.column(),
				 location.function_name(),
				 fmt::format(std::move(format_string), std::forward<Args>(format_args)...),
				 hyperion::backtrace());
		assert(false);
		std::terminate();
	}

	IGNORE_RESERVED_IDENTIFIERS_START
		#define __HYPERION_ASSERT(condition, format_string, ...) /** NOLINT(   		   **/    \
			/** cppcoreguidelines-macro-usage,                                         **/ \
			/** bugprone-reserved-identifier, cert-dcl37-c,                            **/ \
			/** cert-dcl51-cpp)                                                        **/ \
			[&]() {                                                                        \
				if(!(condition)) {                                                         \
					hyperion::error::hyperion_assert(format_string,                        \
													 std::source_location::current()       \
														 __VA_OPT__(, ) __VA_ARGS__);      \
				}                                                                          \
			}()

	#else // HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION

	/// @brief Triggers an assertion, printing the formatted error message along with
	/// source code location information of where the error occurred
	///
	/// Formats the error message via the given `format_string` and `format_args`, prepended with
	/// the source code location in the format "[file:line]: message"
	///
	/// @tparam Args - The types of the arguments to format into the error message
	/// @tparam N - The size of the file name, in characters
	///
	/// @param format_string - The format string for generating the formatted error message
	/// @param file - The name of the source code file the error occurred in
	/// @param line - The line in the source code file the error occurred at
	/// @param format_args - The arguments to format into the error message
	/// @ingroup error
	/// @headerfile "Hyperion/error/Assert.h"
	/// @note Only enabled in Debug builds
	template<typename... Args, size_t N>
	[[noreturn]] inline constexpr auto hyperion_assert(fmt::format_string<Args...>&& format_string,
													   const char (&file)[N], // NOLINT
													   i64 line,
													   Args&&... format_args) noexcept -> void {
		eprintln("Assertion triggered at [{}:{}]: {}\nBacktrace:\n{}",
				 file, // NOLINT
				 line,
				 fmt::format(std::move(format_string), std::forward<Args>(format_args)...),
				 hyperion::backtrace());
		assert(false); // NOLINT
		std::terminate();
	}

	IGNORE_RESERVED_IDENTIFIERS_START
		#define __HYPERION_ASSERT(condition, format_string, ...) /** NOLINT(                   **/ \
			/** cppcoreguidelines-macro-usage,                                                 **/ \
			/** bugprone-reserved-identifier, cert-dcl37-c,                                    **/ \
			/** cert-dcl51-cpp)                                                                **/ \
			[&]() {                                                                                \
				if(!(condition)) {                                                                 \
					hyperion::error::hyperion_assert(format_string,                                \
													 __FILE__,                                     \
													 __LINE__ __VA_OPT__(, ) __VA_ARGS__);         \
				}                                                                                  \
			}()
	IGNORE_RESERVED_IDENTIFIERS_STOP

	#endif // HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION

#else														 // HYPERION_PLATFORM_DEBUG

	IGNORE_RESERVED_IDENTIFIERS_START
	#define __HYPERION_ASSERT(condition, format_string, ...) /** NOLINT(                       **/ \
															 /** cppcoreguidelines-macro-usage,**/ \
															 /** bugprone-reserved-identifier, **/ \
															 /** cert-dcl37-c,		           **/     \
															 /** cert-dcl51-cpp) 			   **/

	IGNORE_RESERVED_IDENTIFIERS_STOP

#endif // HYPERION_PLATFORM_DEBUG

/// @def HYPERION_ASSERT(condition, format_string, ...)
/// @brief Conditionally triggers an assertion, printing the formatted error message along
/// with source code location information of where the error occurred
///
/// Formats the error message via the given `format_string` and `format_args`, prepended
/// with the source code location information.
///
/// If `std::source_location` is available, the assertion message will be in the format
/// "[file:line:column: function]: message".
/// Otherwise, it will be in the format "[file:line]: message"
///
/// @param condition - The assertion condition. Fires the assertion if `false`
/// @param format_string - The format string for generating the formatted error message
/// @param ... - The arguments to format into the error message
/// @ingroup error
/// @headerfile "Hyperion/error/Assert.h"
/// @note Only enabled in Debug builds
/// @note Please don't pass arguments with side-effects to this, at best you'll cause build
/// errors in release builds, and at worst you'll cause really hard to find bugs
#define HYPERION_ASSERT(condition, /** NOLINT(cppcoreguidelines-macro-usage)**/ \
						format_string,                                          \
						...)                                                    \
	__HYPERION_ASSERT(condition, format_string __VA_OPT__(, ) __VA_ARGS__)

	IGNORE_UNUSED_MACROS_STOP
} // namespace hyperion::error
