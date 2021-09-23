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
#include "../Platform.h"

namespace hyperion::error {

	IGNORE_UNUSED_MACROS_START

#if HYPERION_PLATFORM_DEBUG

	#if HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

	template<typename... Args>
	[[noreturn]] constexpr inline auto hyperion_assert(fmt::format_string<Args...>&& string,
													   const std::source_location location,
													   Args&&... args) noexcept -> void {
		eprintln("Assertion triggered at [{}:{}:{}: {}]: {}",
				 location.file_name(),
				 location.line(),
				 location.column(),
				 location.function_name(),
				 fmt::format(std::move(string), std::forward<Args>(args)...));
		std::fflush(stderr);
		assert(false);
	}

		#define HYPERION_ASSERT(condition, format_string, ...) /** NOLINT **/         \
			[&]() {                                                                   \
				if(!condition) {                                                      \
					hyperion::error::hyperion_assert(condition,                       \
													 format_string,                   \
													 std::source_location::current()  \
														 __VA_OPT__(, ) __VA_ARGS__); \
				}                                                                     \
			}()

	#else // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

	template<typename... Args, size_t N>
	[[noreturn]] constexpr inline auto hyperion_assert(fmt::format_string<Args...>&& string,
													   const char (&file)[N], // NOLINT
													   i64 line,
													   Args&&... args) noexcept -> void {
		eprintln("Assertion triggered at [{}:{}]: {}",
				 file, // NOLINT
				 line,
				 fmt::format(std::move(string), std::forward<Args>(args)...));
		std::fflush(stderr);
		assert(false); // NOLINT
	}

		#define HYPERION_ASSERT(condition, format_string, ...) /** NOLINT **/              \
			[&]() {                                                                        \
				if(!condition) {                                                           \
					hyperion::error::hyperion_assert(format_string,                        \
													 __FILE__,                             \
													 __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
				}                                                                          \
			}()

	#endif // HYPERION_HAS_SOURCE_LOCATION || HYPERION_USE_EXPERIMENTAL_SOURCE_LOCATION

#else // HYPERION_PLATFORM_DEBUG

	#define HYPERION_ASSERT(condition, format_string, ...) /** NOLINT **/

#endif // HYPERION_PLATFORM_DEBUG

	IGNORE_UNUSED_MACROS_STOP
} // namespace hyperion::error
