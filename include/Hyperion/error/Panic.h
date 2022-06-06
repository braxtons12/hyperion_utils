/// @file Panic.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Runtime termination facilities
/// @version 0.1
/// @date 2022-06-06
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
#include <cassert>

#if HYPERION_HAS_SOURCE_LOCATION
	#include <source_location>
#elif HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
	#include <experimental/source_location>
// if we're using the experimental source location on an older GCC, alias it to the main std
// namespace
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#endif // HYPERION_HAS_SOURCE_LOCATION

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/error/Backtrace.h>
#include <atomic>

namespace hyperion::error {
	/// @brief Panic manages the active panic handler called when a panic occurs
	///
	/// # Example
	/// @code {.cpp}
	/// 	#include <Hyperion/Utils.h>
	/// static auto my_panic_handler(const std::string& panic_message) noexcept -> void {
	/// 	// do something w/ `panic_message`
	///		// we leave it up to the handler to do something appropriate after handling the message
	/// 	// this should usually be to terminate the process
	///		std::terminate();
	/// }
	///
	/// int main(int argc, char** argv) {
	///		hyperion::Panic::set_handler(&my_panic_handler);
	///
	/// 	// the full panic message w/ source location and backtrace will be passed to
	///		// `my_panic_handler`
	///		panic("My Panic Message!");
	/// }
	/// @endcode
	/// @ingroup error
	/// @headerfile "Hyperion/error/Panic.h"
	class Panic {
	  public:
		using handler_type = void (*)(const std::string& panic_message) noexcept;
		static inline auto set_handler(handler_type panic_handler) noexcept -> void {
			handler.store(panic_handler, std::memory_order_seq_cst);
		}

		[[nodiscard]] static inline auto get_handler() noexcept -> handler_type {
			return handler.load(std::memory_order_seq_cst);
		}

		[[nodiscard]] static inline auto get_default_handler() noexcept -> handler_type {
			return &default_handler;
		}

	  private:
		/// @brief The custom panic handler to use, if any.
		///
		/// This should be defined in __**exactly ONE (1)**__ translation unit, and can only be
		/// defined when `HYPERION_USES_CUSTOM_PANIC_HANDLER` has been defined to `true` prior to
		/// including any Hyperion headers
		/// @ingroup error
		/// @headerfile "Hyperion/error/Panic.h"
		static std::atomic<handler_type> handler; // NOLINT

		[[noreturn]] static inline auto
		default_handler(const std::string& message) noexcept -> void {
			eprintln("{}", message);
#if HYPERION_PLATFORM_DEBUG
			assert(false); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,
						   // hicpp-no-array-decay)
#else					   // HYPERION_PLATFORM_DEBUG
			std::terminate();
#endif					   // HYPERION_PLATFORM_DEBUG
		}
	};

	std::atomic<Panic::handler_type> Panic::handler = get_default_handler(); // NOLINT

	IGNORE_UNUSED_MACROS_START

#if HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION

	IGNORE_INVALID_NORETURN_START

	/// @brief Invokes a panic with the given message
	///
	/// A panic is a forced termination due to a detected irrecoverable error
	///
	/// @tparam Args - The types of the arguments to pass to the format string
	///
	/// @param format_string - The format string for formatting the error message to print before
	/// aborting
	/// @param location - The source code location the panic occurred
	/// @param format_args - The arguments to format into the format string
	/// @ingroup error
	/// @headerfile "Hyperion/error/Panic.h"
	template<typename... Args>
	[[noreturn]] inline constexpr auto(panic)(fmt::format_string<Args...>&& format_string,
											  std::source_location location,
											  Args&&... format_args) noexcept -> void {

		const auto message
			= fmt::format("Panic occurred at [{}:{}:{}:{}]: {}\nBacktrace:\n {}",
						  location.file_name(),
						  location.line(),
						  location.column(),
						  location.function_name(),
						  fmt::format(std::move(format_string), std::forward<Args>(format_args)...),
						  hyperion::backtrace());

		auto handler = Panic::get_handler();
		if(handler == nullptr) {
			Panic::get_default_handler()(message);
		}
		else {
			handler(message);
		}
	}

	IGNORE_INVALID_NORETURN_STOP

	IGNORE_RESERVED_IDENTIFIERS_START
	#define __hyperion_panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage, 	   **/ \
												 /** bugprone-reserved-identifier,  		   **/       \
												 /** cert-dcl37-c,						   	   **/                 \
												 /** cert-dcl51-cpp) 					   	   **/               \
		(hyperion::error::panic)(format_string,                                                    \
								 std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
	IGNORE_RESERVED_IDENTIFIERS_STOP

#else // HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION
	IGNORE_INVALID_NORETURN_START

	/// @brief Invokes a panic with the given message
	///
	/// A panic is a forced termination due to a detected irrecoverable error
	///
	/// @tparam Args - The types of the arguments to pass to the format string
	///
	/// @param format_string - The format string for formatting the error message to print before
	/// aborting
	/// @param file - The source code file the panic occurred in
	/// @param line - The source code line the panic occurred at
	/// @param format_args - The arguments to format into the format string
	/// @ingroup error
	template<typename... Args, size_t N>
	[[noreturn]] inline constexpr auto(panic)(fmt::format_string<Args...>&& format_string,
											  const char (&file)[N], // NOLINT
											  i64 line,
											  Args&&... format_args) noexcept -> void {

		const auto message
			= fmt::format("panic occurred at [{}:{}]: {}\nBacktrace:\n{}",
						  file, // nolint
						  line,
						  fmt::format(std::move(format_string), std::forward<args>(format_args)...),
						  hyperion::backtrace());
		auto handler = Panic::get_handler();
		if(handler == nullptr) {
			Panic::get_default_handler()(message);
		}
		else {
			handler(message);
		}
	}

	IGNORE_INVALID_NORETURN_STOP

	IGNORE_RESERVED_IDENTIFIERS_START
	#define __hyperion_panic(format_string, ...) /** NOLINT( 				                   **/            \
												 /** cppcoreguidelines-macro-usage,            **/ \
												 /** bugprone-reserved-identifier,             **/ \
												 /** cert-dcl37-c,					           **/              \
												 /** cert-dcl51-cpp) 				           **/            \
		(hyperion::error::panic)(format_string, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
	IGNORE_RESERVED_IDENTIFIERS_STOP

#endif // HYPERION_HAS_SOURCE_LOCATION || HYPERION_HAS_EXPERIMENTAL_SOURCE_LOCATION

/// @brief Invokes a panic with the formatted error message
///
/// A panic is a forced termination due to a detected irrecoverable error.
/// This will invoke a panic with the formatted error message, prepended with the source code
/// location at which the panic occurred.
///
/// If `std::source_location` is available, the panic message will be in the format
/// "[file:line:column: function]: message".
/// Otherwise, it will be in the format "[file:line]: message"
///
/// @param format_string - The format string for formatting the error message to print before
/// aborting
/// @ingroup error
/// @headerfile "Hyperion/error/Panic.h"
#define panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	__hyperion_panic(format_string __VA_OPT__(, ) __VA_ARGS__)

	IGNORE_UNUSED_MACROS_STOP
} // namespace hyperion::error
