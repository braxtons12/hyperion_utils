/// @file Panic.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Runtime termination facilities
/// @version 0.1
/// @date 2021-10-19
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
#elif HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION
	#include <experimental/source_location>
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#endif // HYPERION_HAS_SOURCE_LOCATION

#include <Hyperion/BasicTypes.h>
#include <Hyperion/FmtIO.h>
#include <Hyperion/Ignore.h>

namespace hyperion::error {
	IGNORE_UNUSED_MACROS_START

/// @def HYPERION_USES_CUSTOM_PANIC_HANDLER
/// @brief Configures Hyperion to use a custom panic handler if `true`
///
/// A panic handler is a function which is hooked in to the panic process and replaces the
/// default panic behavior.
///
/// By default, Hyperion doesn't use a panic handler and instead simply prints the panic message
/// and aborts. However, `HYPERION_USES_CUSTOM_PANIC_HANDLER` can be defined to `true` to inform
/// the library that a custom panic handler function will be provided and should be used. This
/// allows for custom behavior prior to aborting the process (eg, for showing an error window
/// popup in a graphical application before terminating).
///
/// In this case, `HYPERION_USES_CUSTOM_PANIC_HANDLER`
/// should be defined to `true` prior to including any Hyperion headers, and
/// `hyperion::error::Panic::handler` should be defined to the custom panic handler function in
/// __**exactly ONE (1)**__ translation unit. The panic handler function should take the
/// signature of:
///
/// @code {.cpp}
/// auto function_name(const std::string& panic_message) noexcept -> void;
/// @endcode
/// @ingroup error
/// @note even with a custom handler, a panic will still abort the process. The handler will
/// just be called before process termination
#ifndef HYPERION_USES_CUSTOM_PANIC_HANDLER
	/// @def HYPERION_USES_CUSTOM_PANIC_HANDLER
	/// @brief Configures Hyperion to use a custom panic handler if `true`
	///
	/// A panic handler is a function which is hooked in to the panic process and replaces the
	/// default panic behavior.
	///
	/// By default, Hyperion doesn't use a panic handler and instead simply prints the panic message
	/// and aborts. However, `HYPERION_USES_CUSTOM_PANIC_HANDLER` can be defined to `true` to inform
	/// the library that a custom panic handler function will be provided and should be used. This
	/// allows for custom behavior prior to aborting the process (eg, for showing an error window
	/// popup in a graphical application before terminating).
	///
	/// In this case, `HYPERION_USES_CUSTOM_PANIC_HANDLER`
	/// should be defined to `true` prior to including any Hyperion headers, and
	/// `hyperion::error::Panic::handler` should be defined to the custom panic handler function in
	/// __**exactly ONE (1)**__ translation unit. The panic handler function should take the
	/// signature of:
	///
	/// @code {.cpp}
	/// auto function_name(const std::string& panic_message) noexcept -> void;
	/// @endcode
	/// @ingroup error
	/// @note even with a custom handler, a panic will still abort the process. The handler will
	/// just be called before process termination
	#define HYPERION_USES_CUSTOM_PANIC_HANDLER false // NOLINT(cppcoreguidelines-macro-usage)
#endif												 // HYPERION_USES_CUSTOM_PANIC_HANDLER

#if HYPERION_USES_CUSTOM_PANIC_HANDLER
	// wrap the panic handler global in a struct to avoid any possible (admittedly unlikely) ADL
	// issues
	struct Panic {
		/// @brief The custom panic handler to use if `HYPERION_USES_CUSTOM_PANIC_HANDLER` is
		/// defined to true.
		///
		/// This should be defined in __**exactly ONE (1)**__ translation unit, and can only be
		/// defined when `HYPERION_USES_CUSTOM_PANIC_HANDLER` has been defined to `true` prior to
		/// including any Hyperion headers
		/// @ingroup error
		/// @headerfile "Hyperion/error/Panic.h"
		static void (*const handler)( // NOLINT (non-const globals)
			const std::string& message) noexcept;
	};
#endif // HYPERION_USES_CUSTOM_PANIC_HANDLER

#if HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

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
											  const std::source_location location,
											  Args&&... format_args) noexcept -> void {

	#if HYPERION_USES_CUSTOM_PANIC_HANDLER
		Panic::handler(
			fmt::format("Panic occured at [{}:{}:{}:{}]: {}",
						location.file_name(),
						location.line(),
						location.column(),
						location.function_name(),
						fmt::format(std::move(format_string), std::forward<Args>(format_args)...)));
		std::terminate();
	#else // HYPERION_USES_CUSTOM_PANIC_HANDLER
		eprintln("Panic occurred at [{}:{}:{}: {}]: {}",
				 location.file_name(),
				 location.line(),
				 location.column(),
				 location.function_name(),
				 fmt::format(std::move(format_string), std::forward<Args>(format_args)...));
		#if HYPERION_PLATFORM_DEBUG
		assert(false); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,
					   // hicpp-no-array-decay)
		#else  // HYPERION_PLATFORM_DEBUG
		std::terminate();
		#endif // HYPERION_PLATFORM_DEBUG
	#endif	   // HYPERION_USES_CUSTOM_PANIC_HANDLER
	}

	IGNORE_RESERVED_IDENTIFIERS_START
	#define __hyperion_panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage, 	   **/ \
												 /** bugprone-reserved-identifier,  		   **/       \
												 /** cert-dcl37-c,						   	   **/                 \
												 /** cert-dcl51-cpp) 					   	   **/               \
		(hyperion::error::panic)(format_string,                                                    \
								 std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
	IGNORE_RESERVED_IDENTIFIERS_STOP

#else // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

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

	#if HYPERION_USES_CUSTOM_PANIC_HANDLER
		Panic::handler(
			fmt::format("Panic occurred at [{}:{}]: {}",
						file, // NOLINT
						line,
						fmt::format(std::move(format_string), std::forward<Args>(format_args)...)));
		std::terminate();
	#else // HYPERION_USES_CUSTOM_PANIC_HANDLER
		eprintln("Panic occurred at [{}:{}]: {}",
				 file, // NOLINT
				 line,
				 fmt::format(std::move(format_string), std::forward<Args>(format_args)...));
		#if HYPERION_PLATFORM_DEBUG
		assert(false);
		#else									 // HYPERION_PLATFORM_DEBUG
		std::terminate();
		#endif									 // HYPERION_PLATFORM_DEBUG
	#endif										 // HYPERION_USES_CUSTOM_PANIC_HANDLER
	}

	IGNORE_RESERVED_IDENTIFIERS_START
	#define __hyperion_panic(format_string, ...) /** NOLINT( 				                   **/            \
												 /** cppcoreguidelines-macro-usage,            **/ \
												 /** bugprone-reserved-identifier,             **/ \
												 /** cert-dcl37-c,					           **/              \
												 /** cert-dcl51-cpp) 				           **/            \
		(hyperion::error::panic)(format_string, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)
	IGNORE_RESERVED_IDENTIFIERS_STOP

#endif // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

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
