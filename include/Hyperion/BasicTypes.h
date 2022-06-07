/// @file BasicTypes.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief various type aliases for builtin types and user defined literals for them
/// @version 0.1
/// @date 2022-06-04
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
#include <Hyperion/Ignore.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

IGNORE_UNUSED_MACROS_START

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_NO_COMPARISON_WARNING_SUPPRESSION
#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

IGNORE_RESERVED_IDENTIFIERS_START

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_SUITE(...)                                \
	IGNORE_RESERVED_IDENTIFIERS_START                  \
	IGNORE_UNUSED_TEMPLATES_START                      \
	DOCTEST_TEST_SUITE(__VA_ARGS__)                    \
	/** NOLINT(modernize-use-trailing-return-type) **/ \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_CASE(...)                \
	IGNORE_RESERVED_IDENTIFIERS_START \
	IGNORE_UNUSED_TEMPLATES_START     \
	/** NOLINTNEXTLINE **/            \
	DOCTEST_TEST_CASE(__VA_ARGS__)    \
	/** NOLINT **/                    \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SUBCASE(...)                  \
	IGNORE_RESERVED_IDENTIFIERS_START \
	IGNORE_UNUSED_TEMPLATES_START     \
	/** NOLINTNEXTLINE **/            \
	DOCTEST_SUBCASE(__VA_ARGS__)      \
	/** NOLINT **/                    \
	IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#if !defined(DOCTEST_CONFIG_DISABLE)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK(...)                                                                             \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK(__VA_ARGS__) \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_FALSE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_FALSE( \
			__VA_ARGS__)                                                                     \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_EQ(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_EQ( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_NE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_NE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_GT(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_LT(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LT( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_GE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_GE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_LE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_CHECK_LE( \
			__VA_ARGS__)                                                                  \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE( \
			__VA_ARGS__)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_FALSE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_FALSE( \
			__VA_ARGS__)                                                                       \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_EQ(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_EQ( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_NE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_NE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_GT(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_LT(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LT( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_GE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_GE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_LE(...)                                                                 \
		IGNORE_RESERVED_IDENTIFIERS_START IGNORE_UNUSED_TEMPLATES_START DOCTEST_REQUIRE_LE( \
			__VA_ARGS__)                                                                    \
		IGNORE_RESERVED_IDENTIFIERS_STOP IGNORE_UNUSED_TEMPLATES_STOP

#else
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK(...)		   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_FALSE(...)   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_EQ(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_NE(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_GT(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_LT(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_GE(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define CHECK_LE(...)	   hyperion::ignore(__VA_ARGS__)

	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_FALSE(...) hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_EQ(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_NE(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_GT(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_LT(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_GE(...)	   hyperion::ignore(__VA_ARGS__)
	// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define REQUIRE_LE(...)	   hyperion::ignore(__VA_ARGS__)
IGNORE_RESERVED_IDENTIFIERS_STOP
#endif // !defined(DOCTEST_CONFIG_DISABLE)

// #undef TRACY_ENABLE
//  #define TRACY_ENABLE false
#ifdef TRACY_ENABLE

IGNORE_RESERVED_IDENTIFIERS_START
IGNORE_OLD_STYLE_CASTS_START
IGNORE_PADDING_START
IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_START
	#include <Tracy.hpp>
IGNORE_SUGGEST_DESTRUCTOR_OVERRIDE_STOP
IGNORE_PADDING_STOP
IGNORE_OLD_STYLE_CASTS_STOP
IGNORE_RESERVED_IDENTIFIERS_STOP

	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_PROFILING_ENABLED true
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_FUNCTION()                                                           \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		ZoneScoped /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,                \
					   hicpp-no-array-decay) **/                                                  \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_START_FRAME(name)                                                    \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMarkStart(name) /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,      \
								 hicpp-no-array-decay) **/                                        \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_END_FRAME(name)                                                      \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMarkEnd(name) /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,        \
							   hicpp-no-array-decay) **/                                          \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_MARK_FRAME()                                                         \
		IGNORE_RESERVED_IDENTIFIERS_START                                                         \
		IGNORE_OLD_STYLE_CASTS_START                                                              \
		/** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay) **/ \
		FrameMark /** NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,                 \
					  hicpp-no-array-decay) **/                                                   \
			IGNORE_OLD_STYLE_CASTS_STOP IGNORE_RESERVED_IDENTIFIERS_STOP
#else
/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PLATFORM_PROFILING_ENABLED false
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_FUNCTION()
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_START_FRAME(name)
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_END_FRAME(name)
	/// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
	#define HYPERION_PROFILE_MARK_FRAME()
#endif

IGNORE_UNUSED_MACROS_STOP

///	@defgroup basic_types BasicTypes
/// Type aliases for builtin types and user-defined literal operators for them
/// @headerfile "Hyperion/BasicTypes.h"

namespace hyperion {

	/// @brief `u8` is an 8-bit unsigned integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using u8 = uint8_t;
	/// @brief `u16` is an 16-bit unsigned integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using u16 = uint16_t;
	/// @brief `u32` is an 32-bit unsigned integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using u32 = uint32_t;
	/// @brief `u64` is an 64-bit unsigned integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using u64 = uint64_t;
	/// @brief `usize` is the unsigned integer type best suited for sizes on the given platform
	/// (`size_t`)
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using usize = size_t;
	/// @brief `umax` is the largest unsigned integer on the platform (`uintmax_t`)
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using umax = uintmax_t;

	/// @brief `i8` is an 8-bit signed integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using i8 = int8_t;
	/// @brief `i16` is a 16-bit signed integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using i16 = int16_t;
	/// @brief `i32` is a 32-bit signed integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using i32 = int32_t;
	/// @brief `i64` is a 64-bit signed integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using i64 = int64_t;
	/// @brief `imax` is the largest signed integer on the platform (`intmax_t`)
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using imax = intmax_t;

	/// @brief `u8` is a single-byte unsigned integer
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using byte = u8;

	/// @brief `f32` is a single-precision (32-bit) floating point number
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using f32 = float;
	/// @brief `f64` is a double-precision (64-bit) floating point number
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using f64 = double;
	/// @brief `fmax` is the largest-precision floating point number on the platform (`long double`)
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	using fmax = long double;

	namespace detail {
		template<char... Chars>
		struct string_literal {
			std::array<char, sizeof...(Chars)> array = {Chars...};
		};

		enum class literal_status : u8 {
			Valid = 0,
			OutOfRange,
			InvalidCharacterSequence,
			InvalidLiteralType,
		};

		template<literal_status status>
		static constexpr auto check_literal_status() noexcept -> void {
			static_assert(status != detail::literal_status::OutOfRange,
						  "Invalid Literal: Literal out of numeric range for type");
			static_assert(status != detail::literal_status::InvalidCharacterSequence,
						  "Invalid Literal: Literal contains invalid character sequence for type");
			static_assert(status != detail::literal_status::InvalidLiteralType,
						  "Invalid Literal: Requested type is not a valid numeric literal type");
		}

		IGNORE_PADDING_START
		template<typename T>
		struct literal_pair {
			literal_status status = literal_status::Valid;
			T value = T(0);
		};
		IGNORE_PADDING_START

		IGNORE_UNUSED_TEMPLATES_START
		template<typename Type, char... Chars, usize N = sizeof...(Chars)>
		[[nodiscard]] static consteval auto
		// NOLINTNEXTLINE(readability-function-cognitive-complexity)
		parse_literal(string_literal<Chars...> literal) noexcept -> literal_pair<Type> {
			if constexpr(!(std::is_integral_v<Type> || std::is_floating_point_v<Type>)) {
				return {.status = literal_status::InvalidLiteralType};
			}
			else {
				Type sum = 0;
				const auto& str = literal.array;

				[[maybe_unused]] bool found_decimal = false;
				[[maybe_unused]] usize power_of_ten = 1;
				for(auto& digit : str) {
					if constexpr(std::is_floating_point_v<Type>) {
						if((digit < '0' || digit > '9') && digit != '\'' && digit != '.') {
							return {.status = literal_status::InvalidCharacterSequence};
						}
					}
					else {
						if((digit < '0' || digit > '9') && digit != '\'') {
							return {.status = literal_status::InvalidCharacterSequence};
						}
					}

					if constexpr(std::is_floating_point_v<Type>) {
						if(digit >= '0' && digit <= '9') {
							sum = sum * 10 + static_cast<Type>(digit - '0'); // NOLINT
							if(found_decimal) {
								power_of_ten *= 10; // NOLINT
							}
						}
						else if(digit == '.') {
							found_decimal = true;
						}
					}
					else {
						if(digit >= '0' && digit <= '9') {
							sum = sum * 10 + static_cast<Type>(digit - '0'); // NOLINT
						}
					}
				}

				if constexpr(std::is_floating_point_v<Type>) {
					if(found_decimal && power_of_ten != 1) {
						sum = static_cast<Type>(sum) / static_cast<Type>(power_of_ten);
					}
				}

				if(sum > std::numeric_limits<Type>::max()) {
					return {.status = literal_status::OutOfRange};
				}

				return {.status = literal_status::Valid, .value = sum};
			}
		}
		IGNORE_UNUSED_TEMPLATES_STOP
	} // namespace detail

	IGNORE_UNUSED_TEMPLATES_START
	/// @brief user defined literal for `byte`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_byte() noexcept -> byte { // NOLINT
		constexpr auto parsed = detail::parse_literal<byte>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `u8`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_u8() noexcept -> u8 { // NOLINT
		constexpr auto parsed = detail::parse_literal<u8>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `u16`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_u16() noexcept -> u16 { // NOLINT
		constexpr auto parsed = detail::parse_literal<u16>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `u32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_u32() noexcept -> u32 { // NOLINT
		constexpr auto parsed = detail::parse_literal<u32>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `u64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_u64() noexcept -> u64 { // NOLINT
		constexpr auto parsed = detail::parse_literal<u64>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `usize`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_usize() noexcept -> usize { // NOLINT
		constexpr auto parsed = detail::parse_literal<usize>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `umax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_umax() noexcept -> umax { // NOLINT
		constexpr auto parsed = detail::parse_literal<umax>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `i8`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_i8() noexcept -> i8 { // NOLINT
		constexpr auto parsed = detail::parse_literal<i8>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `i16`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_i16() noexcept -> i16 { // NOLINT
		constexpr auto parsed = detail::parse_literal<i16>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `i32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_i32() noexcept -> i32 { // NOLINT
		constexpr auto parsed = detail::parse_literal<i32>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `i64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_i64() noexcept -> i64 { // NOLINT
		constexpr auto parsed = detail::parse_literal<i64>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `imax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_imax() noexcept -> imax { // NOLINT
		constexpr auto parsed = detail::parse_literal<imax>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `f32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_f32() noexcept -> f32 { // NOLINT
		constexpr auto parsed = detail::parse_literal<f32>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `f64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_f64() noexcept -> f64 { // NOLINT
		constexpr auto parsed = detail::parse_literal<f64>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	/// @brief user defined literal for `fmax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	[[nodiscard]] static inline constexpr auto operator""_fmax() noexcept -> fmax { // NOLINT
		constexpr auto parsed = detail::parse_literal<fmax>(detail::string_literal<Chars...>());
		detail::check_literal_status<parsed.status>();
		return parsed.value;
	}

	IGNORE_UNUSED_TEMPLATES_STOP

	namespace literal_tests {
#if HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
		_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")
#endif // HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
	   // clang-format off

		// NOLINTNEXTLINE
		static_assert(static_cast<usize>(64) == 64_usize, "usize literal operator broken!");
		// NOLINTNEXTLINE
		static_assert(static_cast<usize>(64'000) == 64'000_usize, "usize literal operator broken!");
		// NOLINTNEXTLINE
		static_assert(static_cast<usize>(64'123'456) == 64'123'456_usize,
					  "usize literal operator broken!");
		// clang-format on

		// NOLINTNEXTLINE
		static_assert(static_cast<i64>(-64'123'456) == -64'123'456_i64,
					  "i64 literal operator broken!");

		static inline constexpr auto acceptable_deviation
			= static_cast<fmax>(0.000000000001261213356);
		// NOLINTNEXTLINE
		static_assert(static_cast<fmax>(64.123456789) - 64.123456789_fmax < acceptable_deviation,
					  "fmax literal operator broken!");
		// NOLINTNEXTLINE
		static_assert(static_cast<fmax>(64'000) - 64'000_fmax < acceptable_deviation,
					  "fmax literal operator broken!");
		// NOLINTNEXTLINE
		static_assert(static_cast<fmax>(64'000.123456789) - 64'000.123456789_fmax
						  < acceptable_deviation,
					  "fmax literal operator broken!");
		// NOLINTNEXTLINE
		static_assert(static_cast<fmax>(-64'000.123456789) - -64'000.123456789_fmax
						  < acceptable_deviation,
					  "fmax literal operator broken!");

#if HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
		_Pragma("GCC diagnostic pop")
#endif // HYPERION_PLATFORM_COMPILER_CLANG || HYPERION_PLATFORM_COMPILER_GCC
	}  // namespace literal_tests

} // namespace hyperion
