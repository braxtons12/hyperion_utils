/// @brief Collection of aliases and literal suffixes for builtin integral and floating point types
///
/// This is uses Rust's notation for builtin types, thus `uint8_t` maps to `u8`,
/// `size_t` to `usize`, etc. Literal suffixes use the full name of the alias
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

namespace hyperion {

	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	using usize = size_t;
	using umax = uintmax_t;

	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;
	using imax = intmax_t;

	using byte = u8;

	using f32 = float;
	using f64 = double;
	using fmax = long double;

	namespace detail {

		inline static constexpr umax ten = 10;

		template<umax Sum = 0, char... Chars>
		struct literal_parser : std::integral_constant<umax, 0> { };

		template<umax Sum, char Head, char... Chars>
		requires(Head >= '0' && Head <= '9') struct literal_parser<Sum, Head, Chars...>
			: std::integral_constant<umax,
									 literal_parser<Sum * ten + Head - '0', Chars...>::value> {
		};

		template<umax Sum>
		struct literal_parser<Sum> : std::integral_constant<umax, Sum> { };

		template<umax Sum, char... Chars>
		inline static constexpr umax literal_parser_v = literal_parser<Sum, Chars...>::value;

		template<umax Value, typename Type>
		concept ValidLiteral
			= std::is_integral_v<Type> &&(Value <= std::numeric_limits<Type>::max());
	} // namespace detail

	// clang-format off
	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, byte>
	[[nodiscard]] inline static constexpr auto operator""_byte() noexcept -> byte { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, u8>
	[[nodiscard]] inline static constexpr auto operator""_u8() noexcept -> u8 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, u16>
	[[nodiscard]] inline static constexpr auto operator""_u16() noexcept -> u16 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, u32>
	[[nodiscard]] inline static constexpr auto operator""_u32() noexcept -> u32 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, u64>
	[[nodiscard]] inline static constexpr auto operator""_u64() noexcept -> u64 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, usize>
	[[nodiscard]] inline static constexpr auto operator""_usize() noexcept -> usize { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, umax>
	[[nodiscard]] inline static constexpr auto operator""_umax() noexcept -> umax { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, i8>
	[[nodiscard]] inline static constexpr auto operator""_i8() noexcept -> i8 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, i16>
	[[nodiscard]] inline static constexpr auto operator""_i16() noexcept -> i16 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, i32>
	[[nodiscard]] inline static constexpr auto operator""_i32() noexcept -> i32 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, i64>
	[[nodiscard]] inline static constexpr auto operator""_i64() noexcept -> i64 { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	template<char... Chars>
	requires detail::ValidLiteral<detail::literal_parser_v<0, Chars...>, imax>
	[[nodiscard]] inline static constexpr auto operator""_imax() noexcept -> imax { // NOLINT
		return detail::literal_parser_v<0, Chars...>;
	}

	// clang-format on

	[[nodiscard]] inline static constexpr auto operator""_f32(long double val) noexcept -> f32 {
		return static_cast<f32>(val);
	}

	[[nodiscard]] inline static constexpr auto operator""_f64(long double val) noexcept -> f64 {
		return static_cast<f64>(val);
	}

	[[nodiscard]] inline static constexpr auto operator""_fmax(long double val) noexcept -> fmax {
		return static_cast<fmax>(val);
	}

} // namespace hyperion
