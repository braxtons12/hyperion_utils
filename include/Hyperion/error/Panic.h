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
	#include <experimental/source_location>
// if we're using the experimental source location on an older GCC, alias it to the main std
// namespace
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#else
	#error "Hyperion requires std::source_location"
#endif // HYPERION_HAS_SOURCE_LOCATION

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/error/Backtrace.h>
#include <atomic>
#include <cassert>

namespace hyperion::error {
	/// @brief Panic manages the active panic handler called when a panic occurs
	///
	/// # Example
	/// @code {.cpp}
	/// 	#include <Hyperion/Utils.h>
	/// static auto my_panic_handler(const std::string& panic_message,
	/// 							 const std::source_location& location,
	/// 							 const hyperion::backtrace& backtrace) noexcept -> void {
	/// 	// do something w/ `panic_message`, `location`, and `backtrace`
	///		// we leave it up to the handler to do something appropriate after handling the message
	/// 	// this should usually be to terminate the process, so we do so here in our example
	///		std::terminate();
	/// }
	///
	/// int main(int argc, char** argv) {
	///		hyperion::Panic::set_handler(&my_panic_handler);
	///
	/// 	// the full panic message, source location, and backtrace will be passed to
	///		// `my_panic_handler`
	///		panic("My Panic Message!");
	/// }
	/// @endcode
	/// @ingroup error
	/// @headerfile "Hyperion/error/Panic.h"
	class Panic {
	  public:
		// clang-format off

		/// @brief Alias of the custom panic handler type
		///
		/// A panic handler should take the signature
		/// 	- `void(const std::string& panic_message, const std::source_location& location, const hyperion::backtrace& backtrace)`
		/// where:
		/// 	- `panic_message` is the formatted panic message given to the panic call
		/// 	- `location` is the location the panic occurred
		/// 	- `backtrace` is the backtrace originating from the panic location
		/// @ingroup error
		/// @headerfile "Hyperion/error/Panic.h"
		using handler_type = void (*)(const std::string& panic_message,
									  const std::source_location& location,
									  const hyperion::backtrace& backtrace) noexcept;
		// clang-format on

		/// @brief Registers a custom panic handler with Hyperion, so that all panics will be
		/// handled by this handler
		///
		/// It's the responsibility of the caller to ensure that the registered handler does
		/// something sensible. Panics are irrecoverable errors, and should be dealt with in a way
		/// appropriate to such an issue (exit gracefully in some manner).
		///
		/// @param panic_handler - The custom panic handler to use
		/// @ingroup error
		/// @headerfile "Hyperion/error/Panic.h"
		static inline auto set_handler(handler_type panic_handler) noexcept -> void {
			handler.store(panic_handler, std::memory_order_seq_cst);
		}

		/// @brief Returns the currently registered panic handler
		///
		/// @return the current panic handler
		/// @ingroup error
		/// @ingroup "Hyperion/error/Panic.h"
		[[nodiscard]] static inline auto get_handler() noexcept -> handler_type {
			return handler.load(std::memory_order_seq_cst);
		}

		/// @brief Returns the default Hyperion panic handler
		///
		/// Hyperion's default panic handler simply prints the panic message, location, and
		/// backtrace to `stderr`, then triggers an assertion (on debug builds) or calls
		/// `std::terminate` (on release builds)
		///
		/// @return the default panic handler
		/// @ingroup error
		/// @ingroup "Hyperion/error/Panic.h"
		[[nodiscard]] static inline auto get_default_handler() noexcept -> handler_type {
			return &default_handler;
		}

	  private:
		static std::atomic<handler_type> handler; // NOLINT

		IGNORE_INVALID_NORETURN_START
		[[noreturn]] static inline auto
		default_handler(const std::string& panic_message,
						const std::source_location& location,
						const hyperion::backtrace& backtrace) noexcept -> void {
			const auto message = fmt::format("Panic occurred at [{}:{}:{}:{}]: {}\nBacktrace:\n {}",
											 location.file_name(),
											 location.line(),
											 location.column(),
											 location.function_name(),
											 panic_message,
											 backtrace);

			eprintln("{}", message);
#if HYPERION_PLATFORM_DEBUG
			assert(false); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,
						   // hicpp-no-array-decay)
#else					   // HYPERION_PLATFORM_DEBUG
			std::terminate();
#endif					   // HYPERION_PLATFORM_DEBUG
		}
		IGNORE_INVALID_NORETURN_STOP
	};

	std::atomic<Panic::handler_type> Panic::handler = get_default_handler(); // NOLINT

	IGNORE_UNUSED_MACROS_START

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
	[[noreturn]] inline constexpr auto panic_impl(fmt::format_string<Args...>&& format_string,
												  std::source_location location,
												  const hyperion::backtrace& backtrace,
												  Args&&... format_args) noexcept -> void {

		const auto message
			= fmt::format(std::move(format_string), std::forward<Args>(format_args)...);
		auto handler = Panic::get_handler();
		if(handler == nullptr) {
			Panic::get_default_handler()(message, location, backtrace);
		}
		else {
			handler(message, location, backtrace);
		}

#if HYPERION_PLATFORM_COMPILER_GCC
		std::terminate();
#endif
	}

	IGNORE_INVALID_NORETURN_STOP

#define hyperion_panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage, 	   **/ \
										   /** bugprone-reserved-identifier,  		   **/       \
										   /** cert-dcl37-c,						   	   **/                 \
										   /** cert-dcl51-cpp) 					   	   **/               \
	hyperion::error::panic_impl(format_string,                                               \
								std::source_location::current(),                             \
								hyperion::backtrace() __VA_OPT__(, ) __VA_ARGS__)

/// @brief Invokes a panic with the formatted error message
///
/// A panic should be invoked when an irrecoverable error has occurred.
/// By default, this will trigger an assertion in debug builds or a forced termination in release
/// builds, along with the formatted error message and detailed source location, followed by a
/// backtrace. This behavior can be customized by registering a custom handler with
/// `hyperion:error::Panic::set_handler`.
///
/// @param format_string - The format string for formatting the error message to print before
/// aborting
/// @param ... - The (possible/optional) arguments to format into the format string
/// @ingroup error
/// @headerfile "Hyperion/error/Panic.h"
#define panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	hyperion_panic(format_string __VA_OPT__(, ) __VA_ARGS__)

	IGNORE_UNUSED_MACROS_STOP
} // namespace hyperion::error
