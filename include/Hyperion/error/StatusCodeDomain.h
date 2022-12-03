/// @file StatusCodeDomain.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Base requirements of a `StatusCodeDomain` and utilities for implementing one
/// @version 0.1
/// @date 2022-11-15
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
#include <string_view>
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
	concept UUIDString = std::same_as<T, const char (&)[num_chars_in_uuid]>		   // NOLINT
						 || std::same_as<T, const char (&)[num_chars_in_ms_uuid]>; // NOLINT

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
	///
	/// # Requirements
	/// Given a `const StatusCode<Domain>& code`, a `Domain domain`, and a
	/// `DomainOrOther domain2`, where `Domain` is the `StatusCodeDomain` being verified and
	/// `DomainOrOther` is a potentially different `StatusCodeDomain` a `StatusCodeDomain` must
	/// meet:
	/// * `typename Domain::value_type` - Every domain must have a value type
	/// * `std::constructible_from<Domain, u64>` - Every domain must be constructible from the `u64`
	/// representation of its UUID
	/// * `std::constructible_from<Domain, const char(&)[num_chars_in_uuid]` - Every domain must be
	/// constructible from the string representation of its UUID
	/// * `std::constructible_from<Domain, const char(&)[num_chars_in_ms_uuid]` - Every domain must
	/// be constructible from the string representation of its UUID
	/// * `{ domain.id() } -> std::same_as<u64>` - Every domain must have a member function `id`
	/// that returns its UUID in its `u64` representation
	/// * `{ domain.name() } -> std::same_as<std::string_view>` - Every domain must have a member
	/// function `name` which returns the user-facing name of the domain as a `std::string_view`
	/// * `{ domain.message(code) } -> concepts::Stringable` - Every
	/// domain must be capable of converting a `StatusCode` of its domain into the textual message
	/// associated with the low-level code, as a type convertible to `std::string` and/or
	/// `std::string_view`
	/// * `{ domain.are_equivalent(code, code) } -> std::same_as<bool>` - Every domain must have a
	/// member function `are_equivalent` which returns whether two status codes, of potentially
	/// different domains represent equivalent errors
	/// * `{ domain.is_error(code) } -> std::same_as<bool>` - Every domain must have a member
	/// function `is_error` which returns whether a status code of that domain represents an error
	/// * `{ domain.is_success(code) } -> std::same_as<bool>` - Every domain must have a member
	/// function `is_success` which returns whether a status code of that domain represents success
	/// * `{ domain.success_value() } -> std::same_as<typename Domain::value_type>` - Every domain
	/// must have a member function `success_value` which returns the value of that domain's
	/// `value_type` which represents success
	/// * `domain == domain2` - Every domain must be comparable to other domains for equality,
	/// and domains comparing equal must be the same domain
	/// * `domain != domain2` - Every domain must be comparable to other domains for inequality,
	/// and domains comparing non-equal must NOT be the same domain
	/// * `{ make_status_code_domain<Domain>() } -> std::same_as<Domain>` - Every domain must have
	/// an associated specialization of the global function `make_status_code_domain` taking zero
	/// arguments which returns an instance of that domain.
	///
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCodeDomain.h"
	template<typename Domain>
	concept StatusCodeDomain
		= requires(Domain domain, Domain domain2, const StatusCode<Domain>& code) {
			  typename Domain::value_type;

			  requires std::constructible_from<Domain, u64>;
			  requires std::constructible_from<
				  Domain,
				  const char(&)[num_chars_in_uuid]>; // NOLINT(hicpp-avoid-c-arrays,
													 // modernize-avoid-c-arrays,
													 // cppcoreguidelines-avoid-c-arrays)
			  requires std::constructible_from<
				  Domain,
				  const char(&)[num_chars_in_ms_uuid]>; // NOLINT(hicpp-avoid-c-arrays,
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

	namespace detail {
		/// @brief Parses the semantic numeric value from a character
		///
		/// @param character - The character to parse
		/// @return The semantic numerical value of `c`
		inline constexpr auto parse_byte_from_char(const char character) -> u64 {
			if('0' <= character && character <= '9') {
				return static_cast<u64>(character - '0');
			}
			// NOLINTNEXTLINE(readability-else-after-return)
			else if('a' <= character && character <= 'f') {
				return static_cast<u64>(10 + character - 'a'); // NOLINT
			}
			else if('A' <= character && character <= 'F') {
				return static_cast<u64>(10 + character - 'A'); // NOLINT
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
		inline constexpr auto
		parse_uuid(const char (&s)[N]) // NOLINT(hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
									   // cppcoreguidelines-avoid-c-arrays)
			-> u64
		requires UUIDString<decltype(s)>
		{

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
		noexcept -> u64
	requires UUIDString<decltype(uuid)>
	{
		return detail::parse_uuid(uuid);
	}

	IGNORE_RESERVED_IDENTIFIERS_START
	IGNORE_UNUSED_MACROS_START
// NOLINTNEXTLINE (macro, reserved ident)
#define ___STATUS_CODE_FIRST(...) ___STATUS_CODE_FIRST_IMPL(__VA_ARGS__, ~)
// NOLINTNEXTLINE (macro, reserved ident)
#define ___STATUS_CODE_FIRST_IMPL(first, ...) first

/// @brief Generates a `StatusCodeDomain` class from the given parameters
///
/// Generates a complete implementation for a `StatusCodeDomain` comprised of the given parameters,
/// including the class definition, supplementaty `make_status_code_domain` factory functions, and
/// aliases for `StatusCode`, `ErrorCode`, and `Error`s of that domain.
///
/// @param Name  The name of the class
/// @param Category  The category of the domain (ie if the domain is `PosixDomain`,
/// the category would be `Posix`)
/// @param _namespace  The namespace to generate the class in
/// @param ValueType  The value type of the domain (ie, an enum, an `i64` code, etc)
/// @param IsConvertibleToGenericStatusCode  Whether codes of this domain are convertible to codes
/// in the `GenericDomain`
/// @param uuid_string  The UUID of the domain, in string format
/// @param name_string  The name of the domain, as a string
/// @param _success_value  The `ValueType` value representing success
/// @param unknown_value  The `ValueType` value representing an unknown error
/// @param message_function  function or lambda converting a `ValueType` to a string message
/// @param ...  if `IsConvertibleToGenericStatusCode`, the function or lambda converting a
/// `ValueType` to the value type of `GenericDomain` (`hyperion::error::Errno`)
///
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCodeDomain.h"
#define STATUS_CODE_DOMAIN(Name, /** NOLINT(cppcoreguidelines-macro-usage) **/                     \
						   Category,                                                               \
						   _namespace,                                                             \
						   ValueType,                                                              \
						   IsConvertibleToGenericStatusCode,                                       \
						   uuid_string,                                                            \
						   name_string,                                                            \
						   _success_value,                                                         \
						   unknown_value,                                                          \
						   message_function,                                                       \
						   ... /** to_generic_code_function **/)                                   \
	namespace _namespace {                                                                         \
		class Name;                                                                                \
                                                                                                   \
		class [[nodiscard]] Name {                                                                 \
		  public:                                                                                  \
			using value_type = ValueType;                                                          \
			using Category##StatusCode = hyperion::error::StatusCode</**/ Name /**/>;              \
			using Category##ErrorCode = hyperion::error::ErrorCode</**/ Name /**/>;                \
                                                                                                   \
			static const constexpr char (&UUID)[hyperion::error::num_chars_in_uuid] /** NOLINT **/ \
				= uuid_string;                                                                     \
                                                                                                   \
			static constexpr hyperion::u64 ID					 /** NOLINT **/                    \
				= hyperion::error::parse_uuid_from_string(UUID); /** NOLINT **/                    \
                                                                                                   \
			constexpr Name() noexcept = default;                                                   \
			explicit constexpr Name(hyperion::u64 uuid) noexcept : m_uuid(uuid) { /**NOLINT**/     \
			}                                                                                      \
			template<hyperion::error::UUIDString UUID>	  /** NOLINT**/                            \
			explicit constexpr Name(UUID&& uuid) noexcept /** NOLINT **/                           \
				: m_uuid(hyperion::error::parse_uuid_from_string(std::forward<UUID>(uuid))) { }    \
			constexpr Name(const Name&) noexcept = default;                                        \
			constexpr Name(Name&&) noexcept = default;                                             \
			constexpr ~/****/ Name() noexcept = default;                                           \
                                                                                                   \
			[[nodiscard]] inline constexpr auto id() const noexcept -> hyperion::u64 {             \
				return m_uuid;                                                                     \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline constexpr auto name() const noexcept -> std::string_view {        \
				return name_string;                                                                \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline auto                                                              \
			message(value_type _code) const noexcept -> decltype(message_function(_code)) {        \
				return message_function(_code);                                                    \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline auto message(const Category##StatusCode& _code) const noexcept    \
				-> decltype(message_function(std::declval<value_type>())) {                        \
				return message(_code.code());                                                      \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline constexpr auto is_error(value_type code) const noexcept -> bool { \
				return code != _success_value;                                                     \
			}                                                                                      \
			[[nodiscard]] inline constexpr auto                                                    \
			is_error(const Category##StatusCode& code) const noexcept -> bool {                    \
				return is_error(code.code());                                                      \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline constexpr auto                                                    \
			is_success(value_type code) const noexcept -> bool {                                   \
				return code == _success_value;                                                     \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] inline constexpr auto                                                    \
			is_success(const Category##StatusCode& code) const noexcept -> bool {                  \
				return is_success(code.code());                                                    \
			}                                                                                      \
                                                                                                   \
			template<typename Domain>                                                              \
			[[nodiscard]] inline constexpr auto                                                    \
			are_equivalent(const Category##StatusCode& lhs,                                        \
						   const hyperion::error::StatusCode<Domain>& rhs) const noexcept          \
				-> bool {                                                                          \
				if constexpr(IsConvertibleToGenericStatusCode) {                                   \
					if constexpr(hyperion::error::ConvertibleToGenericStatusCode<                  \
									 hyperion::error::StatusCode<Domain>>)                         \
					{                                                                              \
						return as_generic_code(lhs) == rhs.as_generic_code();                      \
					}                                                                              \
				}                                                                                  \
                                                                                                   \
				if(rhs.domain() == *this) {                                                        \
					const auto lhs_code = lhs.code();                                              \
					const auto rhs_code = rhs.code();                                              \
					return lhs_code == rhs_code && lhs_code != unknown_value                       \
						   && rhs_code != unknown_value;                                           \
				}                                                                                  \
                                                                                                   \
				return false;                                                                      \
			}                                                                                      \
                                                                                                   \
			template<typename U = Category##StatusCode>                                            \
			requires hyperion::concepts::Same<std::remove_const_t<std::remove_reference_t<U>>,     \
											  Category##StatusCode>                                \
					 [[nodiscard]] inline constexpr auto                                           \
					 as_generic_code(const U& _code) const noexcept                                \
					 -> hyperion::error::GenericStatusCode                                         \
					 requires(IsConvertibleToGenericStatusCode)                                    \
			{                                                                                      \
				if constexpr(IsConvertibleToGenericStatusCode) {                                   \
					return ___STATUS_CODE_FIRST(__VA_ARGS__)(_code.code());                        \
				}                                                                                  \
			}                                                                                      \
                                                                                                   \
			[[nodiscard]] static inline constexpr auto success_value() noexcept -> value_type {    \
				return _success_value;                                                             \
			}                                                                                      \
                                                                                                   \
			template<typename Domain>                                                              \
			friend constexpr auto                                                                  \
			operator==(const Name& lhs, const Domain& rhs) noexcept -> bool {                      \
				return lhs.id() == rhs.id();                                                       \
			}                                                                                      \
                                                                                                   \
			template<typename Domain>                                                              \
			friend constexpr auto                                                                  \
			operator!=(const Name& lhs, const Domain& rhs) noexcept -> bool {                      \
				return lhs.id() != rhs.id();                                                       \
			}                                                                                      \
                                                                                                   \
			constexpr auto operator=(const Name&) noexcept -> Name& = default;                     \
                                                                                                   \
			constexpr auto operator=(Name&&) noexcept -> Name& = default;                          \
                                                                                                   \
		  private:                                                                                 \
			hyperion::u64 m_uuid = ID;                                                             \
                                                                                                   \
			template<typename U = Category##StatusCode>                                            \
			requires hyperion::concepts::Same<std::remove_const_t<std::remove_reference_t<U>>,     \
											  Category##StatusCode>                                \
					 [[nodiscard]] inline constexpr auto as_generic_code(                          \
						 [[maybe_unused]] const Category##StatusCode& code) const noexcept         \
					 -> hyperion::error::GenericStatusCode                                         \
					 requires(!IsConvertibleToGenericStatusCode)                                   \
			{                                                                                      \
				return {};                                                                         \
			}                                                                                      \
		};                                                                                         \
                                                                                                   \
		using Category##StatusCode = Name /**/ ::/**/ Category##StatusCode;                        \
		using Category##ErrorCode = Name /**/ ::/**/ Category##ErrorCode;                          \
		using Category##Error = hyperion::error::Error</**/ Name /**/>;                            \
	}                                                                                              \
                                                                                                   \
	template<>                                                                                     \
	[[nodiscard]] inline constexpr auto                                                            \
		make_status_code_domain<_namespace ::/****/ Name,						   /**NOLINT**/    \
								const char(&)[hyperion::error::num_chars_in_uuid]> /**NOLINT**/    \
		(const char(&uuid)[hyperion::error::num_chars_in_uuid]) noexcept		   /**NOLINT**/    \
			->_namespace ::/***/ Name {                                                            \
		return _namespace ::/****/ Name(uuid);                                                     \
	}                                                                                              \
                                                                                                   \
	template<>                                                                                     \
	[[nodiscard]] inline constexpr auto                                                            \
		make_status_code_domain<_namespace ::/****/ Name,							  /**NOLINT**/ \
								const char(&)[hyperion::error::num_chars_in_ms_uuid]> /**NOLINT**/ \
		(const char(&uuid)[hyperion::error::num_chars_in_ms_uuid]) noexcept			  /**NOLINT**/ \
			->_namespace ::/***/ Name {                                                            \
		return _namespace ::/****/ Name(uuid);                                                     \
	}                                                                                              \
	template<>                                                                                     \
	inline constexpr auto make_status_code_domain<_namespace ::/****/ Name>() noexcept             \
		-> _namespace ::/****/ Name {                                                              \
		return {};                                                                                 \
	}                                                                                              \
                                                                                                   \
	namespace _namespace {                                                                         \
		static_assert(hyperion::error::StatusCodeDomain<Name>);                                    \
	}

	IGNORE_UNUSED_MACROS_STOP
	IGNORE_RESERVED_IDENTIFIERS_STOP
} // namespace hyperion::error
