/// @file BasicTypes.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief various type aliases for builtin types and user defined literals for them
/// @version 0.1
/// @date 2021-10-15
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
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

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

		inline static constexpr umax ten = 10U;

		template<umax Sum = 0U, char... Chars>
		struct literal_parser : std::integral_constant<umax, 0U> { };

		template<umax Sum, char Head, char... Chars>
		requires((Head >= '0' && Head <= '9')
				 || Head == '\'') struct literal_parser<Sum, Head, Chars...>
			: std::integral_constant<
				  umax,
				  literal_parser<Head == '\'' ? Sum : Sum * ten + static_cast<umax>(Head - '0'),
								 Chars...>::value> {
		};

		template<umax Sum>
		struct literal_parser<Sum> : std::integral_constant<umax, Sum> { };

		template<umax Sum, char... Chars>
		inline static constexpr umax literal_parser_v = literal_parser<Sum, Chars...>::value;

		template<umax Value, typename Type>
		concept ValidLiteral
			= std::is_integral_v<Type> &&(Value <= std::numeric_limits<Type>::max());
	} // namespace detail

	IGNORE_UNUSED_TEMPLATES_START
	/// @brief user defined literal for `byte`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, byte>
	[[nodiscard]] inline static constexpr auto operator""_byte() noexcept -> byte { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `u8`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, u8>
	[[nodiscard]] inline static constexpr auto operator""_u8() noexcept -> u8 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `u16`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, u16>
	[[nodiscard]] inline static constexpr auto operator""_u16() noexcept -> u16 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `u32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, u32>
	[[nodiscard]] inline static constexpr auto operator""_u32() noexcept -> u32 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `u64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, u64>
	[[nodiscard]] inline static constexpr auto operator""_u64() noexcept -> u64 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `usize`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, usize>
	[[nodiscard]] inline static constexpr auto operator""_usize() noexcept -> usize { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `umax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, umax>
	[[nodiscard]] inline static constexpr auto operator""_umax() noexcept -> umax { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `i8`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, i8>
	[[nodiscard]] inline static constexpr auto operator""_i8() noexcept -> i8 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `i16`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, i16>
	[[nodiscard]] inline static constexpr auto operator""_i16() noexcept -> i16 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `i32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, i32>
	[[nodiscard]] inline static constexpr auto operator""_i32() noexcept -> i32 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `i64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, i64>
	[[nodiscard]] inline static constexpr auto operator""_i64() noexcept -> i64 { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}

	/// @brief user defined literal for `imax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0U, Chars...>, imax>
	[[nodiscard]] inline static constexpr auto operator""_imax() noexcept -> imax { // NOLINT
		return detail::literal_parser_v<0U, Chars...>;
	}
	IGNORE_UNUSED_TEMPLATES_STOP

	IGNORE_UNUSED_FUNCTIONS_START
	/// @brief user defined literal for `f32`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	[[nodiscard]] inline static constexpr auto operator""_f32(long double val) noexcept -> f32 {
		return static_cast<f32>(val);
	}

	/// @brief user defined literal for `f64`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	[[nodiscard]] inline static constexpr auto operator""_f64(long double val) noexcept -> f64 {
		return static_cast<f64>(val);
	}

	/// @brief user defined literal for `fmax`
	/// @ingroup basic_types
	/// @headerfile "Hyperion/BasicTypes.h"
	[[nodiscard]] inline static constexpr auto operator""_fmax(long double val) noexcept -> fmax {
		return static_cast<fmax>(val);
	}
	IGNORE_UNUSED_FUNCTIONS_STOP

} // namespace hyperion