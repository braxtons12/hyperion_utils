/// @file StatusCodeDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Base requirements of a `StatusCodeDomain` and utilities for implementing one
/// @version 0.1
/// @date 2021-10-15
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
#include <Hyperion/BasicTypes.h>
#include <Hyperion/Concepts.h>
#include <concepts>
#include <type_traits>

namespace hyperion::error {
	template<typename T>
	class StatusCode;

	template<typename T>
	class ErrorCode;

	class GenericDomain;

	using GenericStatusCode = StatusCode<GenericDomain>;
	using GenericErrorCode = ErrorCode<GenericDomain>;
} // namespace hyperion::error

/// @brief Creates a `StatusCodeDomain` of type `Domain` from the given arguments
///
/// A `StatusCodeDomain` **must** have a corresponding specialization of this requiring zero
/// arguments, but may provide additional specializations taking arbitrary arguments
///
/// @tparam Domain - The `StatusCodeDomain` to construct
/// @tparam Args - The type of arguments to pass to the constructor
///
/// @param args - The arguments to pass to the constructor
/// @return the newly constructed `StatusCodeDomain`
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCodeDomain.h"
template<typename Domain, typename... Args>
constexpr auto make_status_code_domain(Args&&... args) noexcept -> Domain;

namespace hyperion::error {
	/// @brief The number of characters in a "typical-format",
	/// Eg: "d4b729dd-655f-4b86-b2a6-924844a4c5f3", UUID string literal
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	static constexpr u64 num_chars_in_uuid = 37U;
	/// @brief The number of characters in a "microsoft-format",
	/// Eg: "{d4b729dd-655f-4b86-b2a6-924844a4c5f3}", UUID string literal
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	static constexpr u64 num_chars_in_ms_uuid = 39U;

	/// @brief A `StatusCodeDomain` is a literal type that provides the semantic meaning for a
	/// given `StatusCode<T>`
	///
	/// A `StatusCodeDomain` is a literal type that provides the semantic information corresponding
	/// with an associated `StatusCode<T>`, such as semantic equivalence with another code, whether
	/// the code indicates success or an error, and the textual message associated with the code.
	///
	/// All `StatusCodeDomain` types must have a UNIQUE ID representing that domain.
	///
	/// In addition to the requirements listed here, if a domain has codes with semantic equivalence
	/// to POSIX error codes, the domain should provide a (preferably `constexpr`) member function
	/// of the signature:
	///
	/// @code {.cpp}
	/// auto as_generic_code(const StatusCode<YourDomain>& code) -> GenericStatusCode;
	/// @endcode
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	template<typename Domain>
	concept StatusCodeDomain
		= requires(Domain domain, Domain domain2, const StatusCode<Domain>& code) {

		typename Domain::value_type;

		std::constructible_from<Domain, u64>;
		std::constructible_from<Domain,
								const char[num_chars_in_uuid]>; // NOLINT(hicpp-avoid-c-arrays,
																// modernize-avoid-c-arrays,
																// cppcoreguidelines-avoid-c-arrays)
		std::constructible_from<
			Domain,
			const char[num_chars_in_ms_uuid]>; // NOLINT(hicpp-avoid-c-arrays,
											   // modernize-avoid-c-arrays,
											   // cppcoreguidelines-avoid-c-arrays)
		{
			domain.id()
			} -> std::same_as<u64>;
		{
			domain.name()
			} -> std::same_as<std::string_view>;
		// the return type of message must be implicitly convertible to `std::string` or
		// `std::string_view`
		{
			domain.message(code)
			} -> concepts::Stringable;
		// the return type of message must be implicitly convertible to `std::string` or
		// `std::string_view`
		//	{
		//		domain.message(value)
		//		} -> concepts::Stringable;
		{
			domain.are_equivalent(code, code)
			} -> std::same_as<bool>;
		{
			domain.is_error(code)
			} -> std::same_as<bool>;
		{
			domain.is_success(code)
			} -> std::same_as<bool>;
		{
			Domain::success_value()
			} -> std::same_as<typename Domain::value_type>;
		domain == domain2;
		domain != domain2;

		{
			make_status_code_domain<Domain>()
			} -> std::same_as<Domain>;
	};

	/// @brief A `UUIDString` is the string literal representation of a UUID.
	///
	/// A `UUIDString` is the 37 (typical) or 39 (microsoft) character (including the null
	/// terminator) string literal representation of a UUID.
	/// Eg: "d4b729dd-655f-4b86-b2a6-924844a4c5f3" (typical)
	/// or
	/// "{d4b729dd-655f-4b86-b2a6-924844a4c5f3}" (microsoft)
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	template<typename T>
	concept UUIDString = std::same_as<T, const char (&)[num_chars_in_uuid]> // NOLINT
		|| std::same_as<T, const char (&)[num_chars_in_ms_uuid]>;			// NOLINT

	namespace detail {
		/// @brief Parses the semantic numeric value from a character
		///
		/// @param c - The character to parse
		/// @return The semantic numerical value of `c`
		inline constexpr auto parse_byte_from_char(const char c)->u64 {
			if('0' <= c && c <= '9') {
				return static_cast<u64>(c - '0');
			}
			else if('a' <= c && c <= 'f') {
				return static_cast<u64>(10 + c - 'a'); // NOLINT
			}
			else if('A' <= c && c <= 'F') {
				return static_cast<u64>(10 + c - 'A'); // NOLINT
			}
			else {
				throw "Invalid character in UUID"; // NOLINT
			}
		}

		/// @brief Parses a UUID into a numerical representation
		///
		/// @param s - The `UUIDString` representing the UUID
		/// @return The UUID represented as a `u64`
		template<usize N>
		inline constexpr auto parse_uuid(
			const char(&s)[N]) // NOLINT(hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
							   // cppcoreguidelines-avoid-c-arrays)
			->u64 requires UUIDString<decltype(s)> {

			const char* uuid = s;														   // NOLINT
			if constexpr(std::same_as<decltype(s), const char(&)[num_chars_in_ms_uuid]>) { // NOLINT
				uuid = s + 1;
			}

			return ((parse_byte_from_char(uuid[0]) << 0U)		  // NOLINT
					| (parse_byte_from_char(uuid[1]) << 4U)		  // NOLINT
					| (parse_byte_from_char(uuid[2]) << 8U)		  // NOLINT
					| (parse_byte_from_char(uuid[3]) << 12U)	  // NOLINT
					| (parse_byte_from_char(uuid[4]) << 16U)	  // NOLINT
					| (parse_byte_from_char(uuid[5]) << 20U)	  // NOLINT
					| (parse_byte_from_char(uuid[6]) << 24U)	  // NOLINT
					| (parse_byte_from_char(uuid[7]) << 28U)	  // NOLINT
					| (parse_byte_from_char(uuid[9]) << 32U)	  // NOLINT
					| (parse_byte_from_char(uuid[10]) << 36U)	  // NOLINT
					| (parse_byte_from_char(uuid[11]) << 40U)	  // NOLINT
					| (parse_byte_from_char(uuid[12]) << 44U)	  // NOLINT
					| (parse_byte_from_char(uuid[14]) << 48U)	  // NOLINT
					| (parse_byte_from_char(uuid[15]) << 52U)	  // NOLINT
					| (parse_byte_from_char(uuid[16]) << 56U)	  // NOLINT
					| (parse_byte_from_char(uuid[17]) << 60U))	  // NOLINT
				   ^ ((parse_byte_from_char(uuid[19]) << 0U)	  // NOLINT
					  | (parse_byte_from_char(uuid[20]) << 4U)	  // NOLINT
					  | (parse_byte_from_char(uuid[21]) << 8U)	  // NOLINT
					  | (parse_byte_from_char(uuid[22]) << 12U)	  // NOLINT
					  | (parse_byte_from_char(uuid[24]) << 16U)	  // NOLINT
					  | (parse_byte_from_char(uuid[25]) << 20U)	  // NOLINT
					  | (parse_byte_from_char(uuid[26]) << 24U)	  // NOLINT
					  | (parse_byte_from_char(uuid[27]) << 28U)	  // NOLINT
					  | (parse_byte_from_char(uuid[28]) << 32U)	  // NOLINT
					  | (parse_byte_from_char(uuid[29]) << 36U)	  // NOLINT
					  | (parse_byte_from_char(uuid[30]) << 40U)	  // NOLINT
					  | (parse_byte_from_char(uuid[31]) << 44U)	  // NOLINT
					  | (parse_byte_from_char(uuid[32]) << 48U)	  // NOLINT
					  | (parse_byte_from_char(uuid[33]) << 52U)	  // NOLINT
					  | (parse_byte_from_char(uuid[34]) << 56U)	  // NOLINT
					  | (parse_byte_from_char(uuid[35]) << 60U)); // NOLINT
		}

	} // namespace detail

	/// @brief Parses a `UUIDString` into a numerical representation
	///
	/// @tparam N - The size of the string literal passed in
	/// @param uuid - The `UUIDString` to parse
	///
	/// @return the given `UUIDString` parsed into a `u64`
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	template<usize N>
	inline constexpr auto parse_uuid_from_string(const char (&uuid)[N]) // NOLINT
		noexcept -> u64 requires UUIDString<decltype(uuid)> {
		return detail::parse_uuid(uuid);
	}
} // namespace hyperion::error
