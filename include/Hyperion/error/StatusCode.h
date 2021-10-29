/// @file StatusCode.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Types for reporting the result of a fallible operation
/// @version 0.1
/// @date 2021-10-19
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

#include <Hyperion/HyperionDef.h>
#include <Hyperion/error/Panic.h>
#include <Hyperion/error/StatusCodeDomain.h>

namespace hyperion::error {

	/// @brief Registers an enum type as a `StatusCodeEnum` representing a particular
	/// `StatusCodeDomain` as its `value_type`
	///
	/// A enum can be registered as a `StatusCodeEnum` representing __exactly one__
	/// `StatusCodeDomain`. To associate a particular enum with a `StatusCodeDomain`, specialize
	/// this template as:
	///
	/// @code {.cpp}
	/// template<>
	/// struct status_code_enum_info<YourEnum> {
	/// 	using domain_type = YourStatusCodeDomainType;
	///		static constexpr bool value = true;
	/// };
	/// @endcode
	///
	/// @tparam T - The enum to register as a `StatusCodeEnum`
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T>
	struct status_code_enum_info {
		using domain_type = void;
		static constexpr bool value = false;
	};

	/// @brief Type trait to determine if a type is an enum registered as a `StatusCodeEnum`
	///
	/// Determines if a type is an enum registered as the `value_type` of a `StatusCodeDomain` (a
	/// `StatusCodeEnum`), and provides the type of that `StatusCodeDomain` as
	/// `is_status_code_enum<T>::domain_type`. To register an enum as a `StatusCodeEnum`, specialize
	/// `status_code_enum_info<T>` for your enum
	///
	/// @tparam T - The type to check if it is a status code enum
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T>
	struct is_status_code_enum : public status_code_enum_info<std::remove_cvref_t<T>> { };

	/// @brief Shorthand for `is_status_code_enum<T>::value`
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T>
	static constexpr auto is_status_code_enum_v = is_status_code_enum<T>::value;

	/// @brief A `StatusCodeEnum` is an enum that has been registered as representing a
	/// `StatusCodeDomain` as its `value_type`, by specializing `status_code_enum_info<T>`
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T>
	concept StatusCodeEnum = is_status_code_enum_v<T> && std::is_enum_v<T> && StatusCodeDomain<
		typename is_status_code_enum<T>::domain_type>;

	/// @brief The associated domain type of a `StatusCodeEnum`. Shorthand for
	/// `is_status_code_enum<T>::domain_type` and `status_code_enum_info<T>::domain_type`
	/// @tparam T - The `StatusCodeEnum` to get the associated domain type of
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T>
	requires StatusCodeEnum<std::remove_cvref_t<T>>
	using status_code_enum_domain = typename is_status_code_enum<T>::domain_type;

	/// @brief Checked version of `status_code_enum_domain<T>` that fails if
	/// `status_code_enum_info<T>::domain_type` is not a `StatusCodeDomain`
	/// @tparam T - The `StatusCodeEnum` to get the associated domain type of
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename T, typename Domain = status_code_enum_domain<T>>
	requires StatusCodeEnum<T> && StatusCodeDomain<Domain>
	using status_code_enum_domain_checked = status_code_enum_domain<T>;
} // namespace hyperion::error

/// @brief Creates a `hyperion::error::StatusCode` from the given `hyperion::error::StatusCodeEnum`
///
/// @param code - The `hyperion::error::StatusCodeEnum` to create a `hyperion::error::StatusCode`
/// from
/// @return a `hyperion::error::StatusCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
constexpr auto make_status_code(hyperion::error::StatusCodeEnum auto code) noexcept
	-> hyperion::error::StatusCode<hyperion::error::status_code_enum_domain<decltype(code)>> {
	return hyperion::error::StatusCode(code);
}

namespace hyperion::error {

	/// @brief Concept that requires `Code` is a `StatusCode` convertible to a `GenericStatusCode`
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename Code>
	concept ConvertibleToGenericStatusCode = requires(const Code& code) {
		code.domain();
		StatusCodeDomain<decltype(code.domain())>;
		std::same_as<StatusCode<decltype(code.domain())>, decltype(code)>;
		{
			code.domain().as_generic_code(code)
			} -> std::same_as<GenericStatusCode>;
	};

	IGNORE_PADDING_START

	/// @brief A type representing the result of a fallible operation in a particular domain
	///
	/// A `StatusCode` represents the result, from a particular domain of possible errors, of a
	/// fallible operation. A `StatusCode` is associated with a singular `StatusCodeDomain`, which
	/// gives the code its semantic meaning, but may be representable (with possibly different
	/// values) in several `StatusCodeDomain`s. For example, an error representing that an invalid
	/// parameter value was passed to a function could be representable in both `PosixDomain` (by
	/// `EINVAL`) and `Win32Domain` (by `ERROR_INVALID_PARAMETER`), but a `StatusCode` representing
	/// `EINVAL` isn't interchangeable with a `StatusCode` representing `ERROR_INVALID_PARAMETER`.
	///
	/// @tparam Domain - The `StatusCodeDomain` the `StatusCode` is associated with
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename Domain>
	class [[nodiscard]] StatusCode {
	  public:
		/// @brief The `value_type` of a `StatusCode` is the same as its associated
		/// `StatusCodeDomain`
		/// @ingroup error
		using value_type = typename Domain::value_type;

		/// @brief Constructs a default `StatusCode`, representing success
		/// @ingroup error
		constexpr StatusCode() noexcept = default;
		/// @brief Constructs a `StatusCode` representing the given numeric code
		///
		/// @param code - The numeric code representing the result of the operation
		/// @param domain - The `StatusCodeDomain` to associate with the `StatusCode`
		/// @ingroup error
		explicit constexpr StatusCode(i64 code,
									  Domain&& domain = make_status_code_domain<Domain>()) noexcept
			// clang-format off
			requires StatusCodeDomain<Domain>
			: m_domain(std::forward<Domain>(domain)), m_code(static_cast<value_type>(code))
		{
			// clang-format on
		}

		/// @brief Constructs a `StatusCode` representing the given `StatusCodeEnum`
		///
		/// @param code - The `StatusCodeEnum` representing the result of the operation
		/// @param domain - The `StatusCodeDomain` to associate with the `StatusCode`
		/// @ingroup error
		explicit constexpr StatusCode(StatusCodeEnum auto code) noexcept
			// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
			: m_domain(make_status_code_domain<Domain>()), m_code(code)
		{
			// clang-format on
		}

		/// @brief Copy-constructor
		/// @ingroup error
		constexpr StatusCode(const StatusCode&) noexcept requires StatusCodeDomain<Domain>
		= default;
		/// @brief Move-constructor
		/// @ingroup error
		constexpr StatusCode(StatusCode&&) noexcept requires StatusCodeDomain<Domain>
		= default;
		/// @brief Destructor
		/// @ingroup error
		constexpr ~StatusCode() noexcept requires StatusCodeDomain<Domain>
		= default;

		/// @brief Sets this `StatusCode` to represent the given numeric code
		/// @param code - The numeric code representing the result of an operation
		/// @ingroup error
		inline constexpr auto assign(i64 code) noexcept -> void requires StatusCodeDomain<Domain> {
			m_code = static_cast<value_type>(code);
		}

		/// @brief Sets this `StatusCode` to represent the given `StatusCodeEnum`
		/// @param code - The `StatusCodeEnum` representing the result of an operation
		/// @ingroup error
		inline constexpr auto assign(StatusCodeEnum auto code) noexcept -> void
			// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
		{
			// clang-format on
			*this = make_status_code(code);
		}

		/// @brief Clears the error (if any) represented by this `StatusCode`, setting it to
		/// represent success instead
		/// @ingroup error
		inline constexpr auto clear() noexcept -> void requires StatusCodeDomain<Domain> {
			m_code = m_domain.success_value();
		}

		/// @brief Returns the numeric value of this `StatusCode`
		///
		/// Converts the `value_type` value represented by this `StatusCode` to an `i64` and returns
		/// it.
		/// @return The status represented by this `StatusCode`, as an `i64`
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		value() const noexcept -> i64 requires StatusCodeDomain<Domain> {
			return static_cast<i64>(m_code);
		}

		/// @brief Returns the value of this `StatusCode` as its `value_type`
		///
		/// Returns the `value_type` value represented by this `StatusCode`.
		/// @return The status represented by this `StatusCode` as its `value_type`.
		/// @note While this may be equivalent to `value()` for `StatusCode`s whose `value_type`s
		/// are integer types, in cases where an integer is strictly required, `value()` should
		/// always be preferred to prevent implicit conversions
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		code() const noexcept -> value_type requires StatusCodeDomain<Domain> {
			return m_code;
		}

		/// @brief Returns the textual message associated with this `StatusCode`
		/// @return The message associated with the value this `StatusCode` currently represents
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		message() const noexcept requires StatusCodeDomain<Domain> {
			return m_domain.message(*this);
		}

		/// @brief Returns whether this `StatusCode` represents an error
		/// @return `true` if this represents an error, false otherwise
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		is_error() const noexcept -> bool requires StatusCodeDomain<Domain> {
			return m_domain.is_error(*this);
		}

		/// @brief Returns whether this `StatusCode` represents success
		/// @return `true` if this represents success, false otherwise
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		is_success() const noexcept -> bool requires StatusCodeDomain<Domain> {
			return m_domain.is_success(*this);
		}

		/// @brief Returns whether this `StatusCode` is semantically equivalent with the given one
		///
		/// @tparam Domain2 - The `StatusCodeDomain` of `rhs`. May be a different domain than the
		/// one associated with `this`
		/// @param rhs - The `StatusCode` to compare this to
		/// @return Whether this is equivalent to `rhs`
		/// @ingroup error
		template<typename Domain2>
		[[nodiscard]] inline constexpr auto
		is_equivalent(const StatusCode<Domain2>& rhs) const noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return m_domain.are_equivalent(*this, rhs);
		}

		/// @brief Converts this into a `GenericStatusCode` (a `StatusCode<GenericDomain>`)
		/// @return This, converted into a `GenericStatusCode`
		/// @note This member function is disabled if this is not `ConvertibleToGenericStatusCode`
		/// @ingroup error
		[[nodiscard]] inline constexpr auto as_generic_code() const noexcept -> bool requires
			StatusCodeDomain<Domain> && ConvertibleToGenericStatusCode<StatusCode> {
			return m_domain.as_generic_code(*this);
		}

		/// @brief Returns the `StatusCodeDomain` associated with this
		/// @return The `StatusCodeDomain` associated with this
		/// @ingroup error
		[[nodiscard]] inline constexpr auto
		domain() const noexcept -> const Domain& requires StatusCodeDomain<Domain> {
			return m_domain;
		}

		/// @brief Converts this `StatusCode` to a `bool`
		///
		/// Converts this `StatusCode` to a `bool`. This is equivalent to `is_success()`
		/// @return this, as a `bool`
		/// @ingroup error
		[[nodiscard]] explicit inline constexpr
		operator bool() const noexcept requires StatusCodeDomain<Domain> {
			return is_success();
		}

		/// @brief Converts this `StatusCode` to its `value_type`
		///
		/// Converts this `StatusCode` to its `value_type`. This is equivalent to `code()`
		/// @return this, as its `value_type`
		/// @ingroup error
		[[nodiscard]] explicit inline constexpr
		operator value_type() const noexcept requires StatusCodeDomain<Domain> {
			return m_code;
		}

		/// @brief Equality Comparison operator
		///
		/// Compares the left-hand-side `StatusCode`, `lhs`, and the right-hand-side `StatusCode`,
		/// `rhs`, for semantic equivalence. `rhs` may be a `StatusCode` of a different domain than
		/// `lhs`
		///
		/// @tparam Domain2 - The `StatusCodeDomain` associated with `rhs`
		/// @param lhs - The left-hand `StatusCode` to compare for semantic equality
		/// @param rhs - The right-hand `StatusCode` to compare for semantic equality
		/// @return Whether `lhs` and `rhs` are semantically equivalent
		/// @ingroup error
		template<typename Domain2>
		friend inline constexpr auto
		operator==(const StatusCode<Domain>& lhs, const StatusCode<Domain2>& rhs) noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return lhs.is_equivalent(rhs);
		}

		/// @brief Inequality Comparison operator
		///
		/// Compares the left-hand-side `StatusCode`, `lhs`, and the right-hand-side `StatusCode`,
		/// `rhs`, for semantic inequivalence. `rhs` may be a `StatusCode` of a different domain
		/// than `lhs`
		///
		/// @tparam Domain2 - The `StatusCodeDomain` associated with `rhs`
		/// @param lhs - The left-hand `StatusCode` to compare for semantic inequality
		/// @param rhs - The right-hand `StatusCode` to compare for semantic inequality
		/// @return Whether `lhs` and `rhs` are semantically inequivalent
		/// @ingroup error
		template<typename Domain2>
		friend inline constexpr auto
		operator!=(const StatusCode<Domain>& lhs, const StatusCode<Domain2>& rhs) noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return !lhs.is_equivalent(rhs);
		}

		/// @brief Copy-assignment operator
		/// @ingroup error
		constexpr auto
		operator=(const StatusCode& code) noexcept -> StatusCode& requires StatusCodeDomain<Domain>
		= default;
		/// @brief Move-assignment operator
		/// @ingroup error
		constexpr auto
		operator=(StatusCode&&) noexcept -> StatusCode& requires StatusCodeDomain<Domain>
		= default;

		/// @brief Assigns the value of the `StatusCodeEnum`, `code`, to this as the value it should
		/// represent
		///
		/// @param code - The `StatusCodeEnum` to assign to this
		/// @return this, reassigned to represent `code`
		/// @ingroup error
		constexpr auto operator=(StatusCodeEnum auto code) noexcept -> StatusCode&
			// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
		{
			// clang-format on
			*this = make_status_code(code);
			return *this;
		}

		// clang-format off
	  protected:
		Domain m_domain = make_status_code_domain<Domain>();
		value_type m_code = m_domain.success_value();
		// clang-format on
	};
	template<typename Domain>
	StatusCode(i64, const Domain&) -> StatusCode<std::decay_t<Domain>>;
	template<typename Domain>
	StatusCode(i64, Domain&&) -> StatusCode<std::decay_t<Domain>>;
	template<StatusCodeEnum Enum, typename Domain = status_code_enum_domain<Enum>>
	StatusCode(const Enum&) -> StatusCode<Domain>;
	template<StatusCodeEnum Enum, typename Domain = status_code_enum_domain<Enum>>
	StatusCode(Enum&&) -> StatusCode<Domain>;

	IGNORE_MARKED_NOEXCEPT_BUT_THROWS_START

	/// @brief An `ErrorCode` is a `StatusCode` that may only represent an error
	///
	/// Constructing an `ErrorCode` from a value that indicates success is invalid, and will cause
	/// a compilation error when evaluated in a `constexpr` context, or cause a panic at runtime.
	///
	/// @tparam Domain - The `StatusCodeDomain` the `ErrorCode` is associated with
	/// @ingroup error
	/// @headerfile "Hyperion/error/StatusCode.h"
	template<typename Domain>
	class [[nodiscard]] ErrorCode final : public StatusCode<Domain> {
	  public:
		using value_type = typename StatusCode<Domain>::value_type;

		/// @brief Constructs an `ErrorCode` representing the given numeric code
		///
		/// @param code - The numeric code representing the result of the operation
		/// @param domain - The `StatusCodeDomain` to associate with the `ErrorCode`
		/// @ingroup error
		explicit constexpr ErrorCode(i64 code, // NOLINT
									 Domain&& domain = make_status_code_domain<Domain>()) noexcept
			: StatusCode<Domain>(code, std::forward<Domain>(domain)) {
			if(std::is_constant_evaluated()) {
				if(this->is_success()) {
					throw "hyperion::error::ErrorCode must be an error value! " // NOLINT
						  "(this->is_error() must be true, but was false";
				}
			}
			else if(this->is_success()) {
				panic("hyperion::error::ErrorCode must be an error value! (this->is_error() must "
					  "be true, but was false");
			}
		}

		/// @brief Constructs a `ErrorCode` representing the given `StatusCodeEnum`
		///
		/// @param code - The `StatusCodeEnum` representing the result of the operation
		/// @param domain - The `StatusCodeDomain` to associate with the `ErrorCode`
		/// @ingroup error
		explicit constexpr ErrorCode(StatusCodeEnum auto code) noexcept // NOLINT
																		// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
			: StatusCode<Domain>(code)
		{
			// clang-format on
			this->m_code = code;

			if(std::is_constant_evaluated()) {
				if(this->is_success()) {
					throw "hyperion::error::ErrorCode must be an error value! " // NOLINT
						  "(this->is_error() must be true, but was false";
				}
			}
			else if(this->is_success()) {
				panic("hyperion::error::ErrorCode must be an error value! (this->is_error() must "
					  "be true, but was false");
			}
		}
		/// @brief Copy-constructor
		/// @ingroup error
		constexpr ErrorCode(const ErrorCode& code) noexcept = default;
		/// @brief Move-constructor
		/// @ingroup error
		constexpr ErrorCode(ErrorCode&& code) noexcept = default;
		/// @brief Destructor
		/// @ingroup error
		constexpr ~ErrorCode() noexcept = default;

		using StatusCode<Domain>::value;
		using StatusCode<Domain>::code;
		using StatusCode<Domain>::message;
		using StatusCode<Domain>::is_success;
		using StatusCode<Domain>::is_error;
		using StatusCode<Domain>::is_equivalent;
		using StatusCode<Domain>::as_generic_code;
		using StatusCode<Domain>::operator bool;
		using StatusCode<Domain>::operator value_type;

		/// @brief Equality Comparison operator
		///
		/// Compares the left-hand-side `ErrorCode`, `lhs`, and the right-hand-side `ErrorCode`,
		/// `rhs`, for semantic equivalence. `rhs` may be a `ErrorCode` of a different domain than
		/// `lhs`
		///
		/// @tparam Domain2 - The `StatusCodeDomain` associated with `rhs`
		/// @param lhs - The left-hand `ErrorCode` to compare for semantic equality
		/// @param rhs - The right-hand `ErrorCode` to compare for semantic equality
		/// @return Whether `lhs` and `rhs` are semantically equivalent
		/// @ingroup error
		template<typename Domain2>
		friend inline constexpr auto
		operator==(const ErrorCode<Domain>& lhs, const ErrorCode<Domain2>& rhs) noexcept -> bool {
			return lhs.is_equivalent(rhs);
		}

		/// @brief Inequality Comparison operator
		///
		/// Compares the left-hand-side `ErrorCode`, `lhs`, and the right-hand-side `ErrorCode`,
		/// `rhs`, for semantic inequivalence. `rhs` may be a `ErrorCode` of a different domain
		/// than `lhs`
		///
		/// @tparam Domain2 - The `StatusCodeDomain` associated with `rhs`
		/// @param lhs - The left-hand `ErrorCode` to compare for semantic inequality
		/// @param rhs - The right-hand `ErrorCode` to compare for semantic inequality
		/// @return Whether `lhs` and `rhs` are semantically inequivalent
		/// @ingroup error
		template<typename Domain2>
		friend inline constexpr auto
		operator!=(const ErrorCode<Domain>& lhs, const ErrorCode<Domain2>& rhs) noexcept -> bool {
			return !lhs.is_equivalent(rhs);
		}

		/// @brief Copy-assignment operator
		/// @ingroup error
		constexpr auto
		operator=(const ErrorCode& code) noexcept -> ErrorCode& requires StatusCodeDomain<Domain>
		= default;
		/// @brief Move-assignment operator
		/// @ingroup error
		constexpr auto
		operator=(ErrorCode&&) noexcept -> ErrorCode& requires StatusCodeDomain<Domain>
		= default;

		/// @brief Assigns the value of the `StatusCodeEnum`, `code`, to this as the value it should
		/// represent
		///
		/// @param code - The `StatusCodeEnum` to assign to this
		/// @return this, reassigned to represent `code`
		/// @ingroup error
		constexpr auto
		operator=(StatusCodeEnum auto code) noexcept -> ErrorCode& // NOLINT
																   // clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
		{
			// clang-format on
			this->m_code = code;
			if(std::is_constant_evaluated()) {
				if(this->is_success()) {
					throw "hyperion::error::ErrorCode must be an error value! " // NOLINT
						  "(this->is_error() must be true, but was false";
				}
			}
			else if(this->is_success()) {
				panic("hyperion::error::ErrorCode must be an error value! (this->is_error() must "
					  "be true, but was false");
			}
			return *this;
		}
	};
	template<typename Domain>
	ErrorCode(i64, const Domain&) -> ErrorCode<std::decay_t<Domain>>;
	template<typename Domain>
	ErrorCode(i64, Domain&&) -> ErrorCode<std::decay_t<Domain>>;
	template<StatusCodeEnum Enum, typename Domain = status_code_enum_domain<Enum>>
	ErrorCode(const Enum&) -> ErrorCode<Domain>;
	template<StatusCodeEnum Enum, typename Domain = status_code_enum_domain<Enum>>
	ErrorCode(Enum&&) -> ErrorCode<Domain>;
	IGNORE_MARKED_NOEXCEPT_BUT_THROWS_STOP
	IGNORE_PADDING_STOP
} // namespace hyperion::error

/// @brief Creates a `hyperion::error::StatusCode` from the given integer code
///
/// @param code - The integer code to create a `hyperion::error::StatusCode`
/// from
/// @return a `hyperion::error::StatusCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
template<typename Domain>
constexpr auto
make_status_code(hyperion::i64 code) noexcept -> hyperion::error::StatusCode<Domain> {
	return hyperion::error::StatusCode<Domain>(code);
}

/// @brief Creates a `hyperion::error::StatusCode` from the given integer code
///
/// @param code - The integer code to create a `hyperion::error::StatusCode`
/// from
/// @param domain - The `Domain` to associate with the `hyperion:error::StatusCode`
/// @return a `hyperion::error::StatusCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
template<typename Domain>
constexpr auto make_status_code(hyperion::i64 code, Domain&& domain) noexcept
	-> hyperion::error::StatusCode<Domain> {
	return hyperion::error::StatusCode(code, std::forward<Domain>(domain));
}

/// @brief Creates a `hyperion::error::ErrorCode` from the given integer code
///
/// @param code - The integer code to create a `hyperion::error::ErrorCode`
/// from
/// @return a `hyperion::error::ErrorCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
template<typename Domain>
constexpr auto make_error_code(hyperion::i64 code) noexcept -> hyperion::error::ErrorCode<Domain> {
	return hyperion::error::ErrorCode<Domain>(code);
}

/// @brief Creates a `hyperion::error::ErrorCode` from the given integer code
///
/// @param code - The integer code to create a `hyperion::error::ErrorCode`
/// from
/// @param domain - The `Domain` to associate with the `hyperion:error::ErrorCode`
/// @return a `hyperion::error::ErrorCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
template<typename Domain>
constexpr auto make_error_code(hyperion::i64 code, Domain&& domain) noexcept
	-> hyperion::error::ErrorCode<Domain> {
	return hyperion::error::ErrorCode(code, std::forward<Domain>(domain));
}

/// @brief Creates a `hyperion::error::ErrorCode` from the given `hyperion::error::StatusCodeEnum`
///
/// @param code - The `hyperion::error::StatusCodeEnum` to create a `hyperion::error::ErrorCode`
/// from
/// @return a `hyperion::error::ErrorCode` associated with the given code
/// @ingroup error
/// @headerfile "Hyperion/error/StatusCode.h"
constexpr auto make_error_code(hyperion::error::StatusCodeEnum auto code) noexcept
	-> hyperion::error::ErrorCode<hyperion::error::status_code_enum_domain<decltype(code)>> {
	return hyperion::error::ErrorCode(code);
}
