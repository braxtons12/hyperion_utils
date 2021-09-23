#pragma once

#include <cassert>

#include "../HyperionDef.h"

#if HYPERION_HAS_SOURCE_LOCATION
	#include <source_location>
#elif HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION
	#include <experimental/source_location>
namespace std { // NOLINT
	using source_location = std::experimental::source_location;
} // namespace std
#endif // HYPERION_HAS_SOURCE_LOCATION

#include "../BasicTypes.h"
#include "../FmtIO.h"
#include "Assert.h"

namespace hyperion::error {
	IGNORE_UNUSED_MACROS_START

#if HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

	/// @brief Invokes a panic with the given message
	///
	/// A panic is a forced termination due to a detected irrecoverable error
	///
	/// @tparam Args - The types of the arguments to pass to the format string
	///
	/// @param string - The format string for formatting the error message to print bef
	template<typename... Args>
	[[noreturn]] constexpr inline auto(panic)(fmt::format_string<Args...>&& string,
											  const std::source_location location,
											  Args&&... args) noexcept -> void {

		eprintln("Panic occured at [{}:{}:{}: {}]: {}",
				 location.file_name(),
				 location.line(),
				 location.column(),
				 location.function_name(),
				 fmt::format(std::move(string), std::forward<Args>(args)...));
		std::fflush(stderr);
	#if HYPERION_PLATFORM_DEBUG
		assert(false);
	#else  // HYPERION_PLATFORM_DEBUG
		std::terminate();
	#endif // HYPERION_PLATFORM_DEBUG
	}

	#define panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		(hyperion::error::panic)(format_string,                                     \
								 std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)

#else // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

	/// @brief Invokes a panic with the given message
	///
	/// A panic is a forced termination due to a detected irrecoverable error
	///
	/// @tparam Args - The types of the arguments to pass to the format string
	///
	/// @param string - The format string for formatting the error message to print bef
	template<typename... Args, size_t N>
	[[noreturn]] constexpr inline auto(panic)(fmt::format_string<Args...>&& string,
											  const char (&file)[N], // NOLINT
											  i64 line,
											  Args&&... args) noexcept -> void {

		const auto str = fmt::format(string, std::forward<Args>(args)...);
		eprintln("Panic occured at [{}:{}]: {}",
				 file, // NOLINT
				 line,
				 str);
		std::fflush(stderr);
	#if HYPERION_PLATFORM_DEBUG
		error::hyperion_assert("{}", file, line, str);
	#endif // HYPERION_PLATFORM_DEBUG
		std::terminate();
	}

	#define panic(format_string, ...) /** NOLINT(cppcoreguidelines-macro-usage) **/ \
		(hyperion::error::panic)(format_string, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#endif // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

	IGNORE_UNUSED_MACROS_STOP
} // namespace hyperion::error
