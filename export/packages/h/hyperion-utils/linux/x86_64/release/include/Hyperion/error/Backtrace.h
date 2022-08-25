/// @file Backtrace.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Wrap Boost::stacktrace and provide fmtlib formatter for it
/// @version 0.1
/// @date 2022-06-15
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

#pragma once

#include <Hyperion/HyperionDef.h>
#include <Hyperion/Fmt.h>

#if HYPERION_PLATFORM_COMPILER_CLANG
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wshadow\"")
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wzero-as-null-pointer-constant\"")
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")
#endif

IGNORE_RESERVED_IDENTIFIERS_START
IGNORE_RESERVED_MACRO_IDENTIFIERS_START
IGNORE_COMMA_MISUSE_START
#include <boost/stacktrace.hpp>
IGNORE_COMMA_MISUSE_STOP
IGNORE_RESERVED_MACRO_IDENTIFIERS_STOP
IGNORE_RESERVED_IDENTIFIERS_STOP

#if HYPERION_PLATFORM_COMPILER_CLANG
_Pragma("GCC diagnostic pop")
_Pragma("GCC diagnostic pop")
_Pragma("GCC diagnostic pop")
#endif

namespace hyperion {
	/// @brief `backtrace` provides a full backtrace of the current call stack it its point of
	/// construction
	///
	/// Example:
	/// @code {.cpp}
	/// #include <Hyperion/FmtIO.h>
	/// #include <Hyperion/error/Backtrace.h>
	///
	/// auto function_that_may_terminate() -> void {
	/// 	if(get_my_special_value() != 42) {
	/// 		println("Special value was bad!\n Backtrace: {}\nTerminating",
	/// 				hyperion::backtrace());
	/// 	}
	/// }
	/// @endcode
	/// @ingroup hyperion::error
	/// @headerfile "Hyperion/error/Backtrace.h"
	using backtrace = boost::stacktrace::stacktrace;
} // namespace hyperion

/// @brief Specialize `fmt::formatter` for `hyperion::backtrace` so it can be used with fmtlib
/// formatting functions
/// @ingroup hyperion::error
/// @headerfile "Hyperion/error/Backtrace.h"
template<>
struct fmt::formatter<hyperion::backtrace> {
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	[[nodiscard]] inline constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.begin();
	}

	template<typename FormatContext>
	[[nodiscard]] inline auto
	format(const hyperion::backtrace& trace, FormatContext& ctx) -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), "{}", to_string(trace));
	}
};
