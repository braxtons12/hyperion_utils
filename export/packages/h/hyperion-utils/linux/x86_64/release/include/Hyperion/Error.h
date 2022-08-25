/// @file Error.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Basic high-level types for communicating recoverable errors
/// @version 0.1
/// @date 2022-07-09
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
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
#include <Hyperion/HyperionDef.h>
#include <Hyperion/Ignore.h>
#include <Hyperion/Memory.h>
#include <Hyperion/error/SystemDomain.h>
#include <cstring>
#include <gsl/gsl>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>

///	@defgroup error Error
/// The Error module provides various types and facilities that make communicating and handling
/// recoverable errors simple and easy, while keeping all possible sources of errors and error
/// handling logic explicit and greppable (NO exceptions usage). It also provides a configurable
/// system for aborting gracefully when irrecoverable errors occur.
///
/// # Example
/// @code {.cpp}
/// @endcode
/// @headerfile "Hyperion/Error.h"

namespace hyperion::error {

	/// @brief Basic interface for recoverable errors. Error types may provide
	/// additional functionality beyond this, but this much is required.
	/// @ingroup error
	/// @headerfile "Hyperion/Error.h"
	IGNORE_WEAK_VTABLES_START

	class [[nodiscard("An error should not be discarded")]] ErrorBase {
	  public:
		constexpr ErrorBase() noexcept = default;

		constexpr ErrorBase(const ErrorBase&) noexcept = default;

		constexpr ErrorBase(ErrorBase &&) noexcept = default;

		constexpr virtual ~ErrorBase() noexcept = default;

		/// @brief Returns the value of the associated error code as an `i64`
		/// @return The value of the associated error code
		/// @ingroup error
		[[nodiscard]] virtual constexpr auto value() const noexcept->i64 = 0;

		/// @brief Returns the message associated with the error as a `std::string`
		/// @return The message associated with the error
		/// @ingroup error
		[[nodiscard]] virtual auto message() const noexcept->std::string = 0;

		/// @brief Converts this error into a `std::string`
		/// @return This error as a `std::string`
		/// @ingroup error
		[[nodiscard]] virtual auto to_string() const noexcept->std::string = 0;

		constexpr auto operator=(const ErrorBase&) noexcept->ErrorBase& = default;

		constexpr auto operator=(ErrorBase&&) noexcept->ErrorBase& = default;
	};

	IGNORE_WEAK_VTABLES_STOP

	// class [[nodiscard]] AnyError;

	/// @brief General purpose type for communicating recoverable errors.
	///
	/// Wraps an `error::ErrorCode<Domain>` in a type-safe manner.
	/// Usually makes up the `E` variant of a `Result<T, E>`
	///
	/// @tparam Domain - The `StatusCodeDomain` associated with the this error
	/// @ingroup error
	template<StatusCodeDomain Domain = SystemDomain>
	class [[nodiscard("An error should not be discarded")]] Error final : public ErrorBase {
	  public:
		/// @brief The type used to represent the error (an enum, integer error code,
		/// etc)
		/// @ingroup error
		using value_type = typename ErrorCode<Domain>::value_type;

		friend class AnyError;

		/// @brief Constructs a default `Error` with an error code representing an
		/// unknown error
		/// @ingroup error
		constexpr Error() noexcept {

			if constexpr(StatusCodeEnum<value_type>) {
				m_error_code = make_error_code(static_cast<value_type>(-1));
			}
			else {
				m_error_code = make_error_code<Domain>(static_cast<value_type>(-1));
			}
		}

		/// @brief Constructs an `Error` from the given `ErrorCode<Domain>`
		///
		/// @param code - The error code
		/// @ingroup error
		constexpr Error(const ErrorCode<Domain>& code) noexcept // NOLINT
			: m_error_code(code) {
		}

		/// @brief Constructs an `Error` from the given `ErrorCode<Domain>`
		///
		/// @param code - The error code
		/// @ingroup error
		constexpr Error(ErrorCode<Domain> && code) noexcept // NOLINT
			: m_error_code(std::move(code)) {
		}

		/// @brief Constructs an `Error` from the given error code
		///
		/// @param code - The error code
		/// @ingroup error
		constexpr Error(const value_type& code) noexcept // NOLINT
			requires(!StatusCodeEnum<value_type>)
			: m_error_code(make_error_code<Domain>(code)) {
		}

		/// @brief Constructs an `Error` from the given error code
		///
		/// @param code - The error code
		/// @ingroup error
		constexpr Error(const value_type& code) noexcept // NOLINT
			requires StatusCodeEnum<value_type> : m_error_code(make_error_code(code)) {
		}

		/// @brief Copy-constructs an `Error`
		/// @ingroup error
		constexpr Error(const Error& error) = default;

		/// @brief Move-constructs an `Error`
		/// @ingroup error
		constexpr Error(Error && error) noexcept = default;

		/// @brief Destroys this `Error`
		/// @ingroup error
		constexpr ~Error() noexcept final = default;

		/// @brief Returns the `i64` value of the error code associated with this
		/// `Error`
		///
		/// @return The associated error code as an `i64`
		/// @ingroup error
		[[nodiscard]] auto value() const noexcept->i64 final {

			return m_error_code.value();
		}

		/// @brief Returns the `ErrorCode` associated with this `Error`
		///
		/// @return The associated `ErrorCode`
		/// @ingroup error
		[[nodiscard]] auto code() const noexcept->const ErrorCode<Domain>& {

			return m_error_code;
		}

		/// @brief Returns the error message associated with this `Error`
		///
		/// @return The error message
		/// @ingroup error
		[[nodiscard]] inline auto message() const noexcept->std::string final {

			if constexpr(concepts::Same<error_code_message_type, std::string>) {
				return m_error_code.message();
			}
			else {
				return std::string(m_error_code.message());
			}
		}

		/// @brief Returns the error message associated with this `Error`, as a cstring
		///
		/// @return The error message
		/// @ingroup error
		[[nodiscard]] auto message_as_cstr() const noexcept->const char* {

			return m_error_code.message().c_str();
		}

		/// @brief Gets the `std::string` representation of this `Error`
		///
		/// @return this `Error` formatted as a `std::string`
		/// @ingroup error
		[[nodiscard]] auto to_string() const noexcept->std::string final {

			return fmt::format("Error: {}", m_error_code.message());
		}

		/// @brief Converts this `Error` into its underlying `ErrorCode<Domain>`
		/// @ingroup error
		constexpr operator ErrorCode<Domain>() const noexcept { // NOLINT
			return m_error_code;
		}

		/// @brief Converts this `Error` into its underlying `value_type`
		/// @ingroup error
		constexpr operator value_type() const noexcept { // NOLINT
			return static_cast<value_type>(m_error_code);
		}

		/// @brief Equality comparison operator with another `Error` from a possibly different
		/// `StatusCodeDomain`
		/// @tparam Domain2 - The `StatusCodeDomain` of `error`
		/// @ingroup error
		template<StatusCodeDomain Domain2>
		constexpr auto operator==(const Error<Domain2>& error) const noexcept->bool {
			return m_error_code == error.m_error_code;
		}

		/// @brief Inequality comparison operator with another `Error` from a possibly different
		/// `StatusCodeDomain`
		/// @tparam Domain2 - The `StatusCodeDomain` of `error`
		/// @ingroup error
		template<StatusCodeDomain Domain2>
		constexpr auto operator!=(const Error<Domain2>& error) const noexcept->bool {
			return m_error_code != error.m_error_code;
		}

		/// @brief Equality comparison operator with an `ErrorCode` from a possibly different
		/// `StatusCodeDomain`
		/// @tparam Domain2 - The `StatusCodeDomain` of `code`
		/// @ingroup error
		template<StatusCodeDomain Domain2>
		constexpr auto operator==(const ErrorCode<Domain2>& code) const noexcept->bool {
			return m_error_code == code;
		}

		/// @brief Inequality comparison operator with an `ErrorCode` from a possibly different
		/// `StatusCodeDomain`
		/// @tparam Domain2 - The `StatusCodeDomain` of `code`
		/// @ingroup error
		template<StatusCodeDomain Domain2>
		constexpr auto operator!=(const ErrorCode<Domain2>& code) const noexcept->bool {
			return m_error_code != code;
		}

		/// @brief Copy-assigns the given `error` to this `Error`
		/// @ingroup error
		constexpr auto operator=(const Error& error) noexcept->Error& = default;

		/// @brief Move-assigns the given `error` to this `Error`
		/// @ingroup error
		constexpr auto operator=(Error&& error) noexcept->Error& = default;

		/// @brief Copy-assigns the given `ErrorCode<Domain>` to this `Error`
		/// @ingroup error
		constexpr auto operator=(const ErrorCode<Domain>& code) noexcept->Error& {

			m_error_code = code;

			return *this;
		}

		/// @brief Move-assigns the given `ErrorCode<Domain>` to this `Error`
		/// @ingroup error
		constexpr auto operator=(ErrorCode<Domain>&& code) noexcept->Error& {

			m_error_code = std::move(code);

			return *this;
		}

		/// @brief Provides `std::tuple`-like structured binding support.
		///
		/// `Index == 0` returns the error code value.
		/// `Index == 1` returns the error message.
		/// Other `Index` values ar invalid
		///
		/// @return The error code value (`Index == 0`), or the error message (`Index
		/// == 1`)
		/// @ingroup error
		template<usize Index>
		auto get() const&& noexcept {

			return get_impl<Index>(*this);
		}

		/// @brief Provides `std::tuple`-like structured binding support.
		///
		/// `Index == 0` returns the error code value.
		/// `Index == 1` returns the error message.
		/// Other `Index` values ar invalid
		///
		/// @return The error code value (`Index == 0`), or the error message (`Index
		/// == 1`)
		/// @ingroup error
		template<usize Index>
		auto get() const& noexcept {

			return get_impl<Index>(*this);
		}

	  private:
		template<usize Index, typename T>
		auto get_impl(T && val) const noexcept {

			static_assert(Index < 2, "Index out of bounds for hyperion::error::Error::get");
			if constexpr(Index == 0) {
				return std::forward<T>(val).m_error_code.value();
			}
			else {
				return std::forward<T>(val).m_error_code.message();
			}
		}

		/// error code
		ErrorCode<Domain> m_error_code;

		using error_code_message_type = decltype(m_error_code.message());
	};
} // namespace hyperion::error

/// @brief Specialize `std::tuple_size` for `error::Error<Domain>`
/// @ingroup error
template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_size<hyperion::error::Error<Domain>> {
	static constexpr hyperion::usize value = 2;
};

/// @brief Specialize `std::tuple_element<0, T>` for `error::Error<Domain>`
/// @ingroup error
template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_element<0, hyperion::error::Error<Domain>> {
	using type = typename hyperion::error::Error<Domain>::value_type;
};

/// @brief Specialize `std::tuple_element<1, T>` for `error::Error<Domain>`
/// @ingroup error
template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_element<1, hyperion::error::Error<Domain>> {
	using type = std::string;
};

namespace hyperion::error {

	/// @brief `SystemError` is an error representing the default platform/OS level
	/// errors (eg POSIX or WIN32 error codes)
	///
	/// In practice, this will usually be an alias for one of `PosixError`,
	/// `Win32Error`, or `NTError`, depending on platform and configuration
	/// @ingroup error
	using SystemError = Error<SystemDomain>;
	/// @brief `PosixError` is an error representing a platform's implementation of
	/// POSIX error codes (it includes support for additional platform specific
	/// codes not required by POSIX)
	/// @ingroup error
	using PosixError = Error<PosixDomain>;
	/// @brief `GenericError` represents the strict set of POSIX required error
	/// codes.
	///
	/// In Hyperion's error system, `GenericError` fills a role similar to a
	/// `std::error_code` of `std::generic_category`
	/// @ingroup error
	using GenericError = Error<GenericDomain>;

#if HYPERION_PLATFORM_WINDOWS
	/// @brief `Win32Error` is an error representing the Win32 error codes
	/// @ingroup error
	using Win32Error = Error<Win32Domain>;
	/// @brief `NTError` is an error representing the Windows NT error codes
	/// @ingroup error
	using NTError = Error<NTDomain>;
#endif // HYPERION_PLATFORM_WINDOWS

	/// @brief Concept specifying the requirements of a type necessary to be
	/// guaranteed compatible with Hyperion's error system.
	///
	/// Users are encouraged, but not required, to fulfill this concept for their own error types,
	/// particularly when using them with Hyperion's higher-level error-handling
	/// facilities like `Result<T, E>`
	/// @ingroup error
	template<typename E>
	concept ErrorType = std::derived_from<E, ErrorBase>;

	/// @brief Concept useful for shorthanding `!ErrorType<E>`
	/// @note Removes the need for extra parentheses and enables cleaner
	/// auto-formatting (eg `SomeConcept &&(!ErrorType<E>)` vs `SomeConcept &&
	/// NotErrorType<E>`)
	/// @ingroup error
	template<typename E>
	concept NotErrorType = !ErrorType<E>;

	IGNORE_WEAK_VTABLES_START

	/// @brief `AnyError` represents a type erased error from any
	/// `error::StatusCodeDomain`
	/// @ingroup error
	class [[nodiscard("An error should not be discarded")]] AnyError final : public ErrorBase {
	  public:
		/// @brief Constructs an `AnyError` as an unknown error
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS AnyError() noexcept = default;

		/// @brief Constructs an `AnyError` from the given `error::ErrorCode<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param code - The error code this `AnyError` should represent
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(const ErrorCode<Domain>& code) noexcept // NOLINT
			: m_error_code(code.value()), m_message(code.message()) {
		}

		/// @brief Constructs an `AnyError` from the given `error::ErrorCode<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param code - The error code this `AnyError` should represent
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(ErrorCode<Domain> && code) noexcept // NOLINT
			: m_error_code(code.value()), m_message(code.message()) {
		}

		/// @brief Constructs an `AnyError` from the given `error::Error<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param error - The error this `AnyError` should represent
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(const Error<Domain>& error) noexcept // NOLINT
			: m_error_code(error.code().value()), m_message(error.message()) {
		}

		/// @brief Constructs an `AnyError` from the given `error::Error<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param error - The error this `AnyError` should represent
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(Error<Domain> && error) noexcept // NOLINT
			: m_error_code(error.code().value()), m_message(error.message()) {
		}
		/// @brief Copy-constructs an `AnyError` from the given one
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS AnyError(const AnyError&) noexcept = default;
		/// @brief Move-constructs an `AnyError` from the given one
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS AnyError(AnyError &&) noexcept = default;
		/// @brief Destroys this `AnyError`
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS ~AnyError() noexcept final = default;

		/// @brief Returns the `i64` value corresponding with the error code this
		/// represents
		/// @return The error code as an `i64`
		/// @ingroup error
		[[nodiscard]] constexpr auto value() const noexcept->i64 final {
			return m_error_code;
		}

		/// @brief Returns the error message associated with the error code this
		/// represents
		/// @return The error code message
		/// @ingroup error
		[[nodiscard]] auto message() const noexcept->std::string final {
			return m_message;
		}

		/// @brief Returns the string representation of this `AnyError`
		/// @return this `AnyError` as a `std::string`
		/// @ingroup error
		[[nodiscard]] HYPERION_CONSTEXPR_STRINGS auto to_string()
			const noexcept->std::string final {
			using namespace std::literals::string_literals;
			return "Error: "s + m_message + "\n"s;
		}

		/// @brief Copy-assigns the given `AnyError` to this one
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS auto operator=(const AnyError&) noexcept->AnyError& = default;
		/// @brief Move-assigns the given `AnyError` to this one
		/// @ingroup error
		HYPERION_CONSTEXPR_STRINGS auto operator=(AnyError&&) noexcept->AnyError& = default;

		/// @brief Copy-assigns this `AnyError` with the given
		/// `error::ErrorCode<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error code
		///
		/// @param code - The error code to assign to this
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(
			const ErrorCode<Domain>& code) noexcept->AnyError& {

			m_error_code = code.value();
			m_message = code.message();

			return *this;
		}

		/// @brief Move-assigns this `AnyError` with the given
		/// `error::ErrorCode<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error code
		///
		/// @param code - The error code to assign to this
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(ErrorCode<Domain>&& code) noexcept->AnyError& {

			m_error_code = code.value();
			m_message = code.message();

			return *this;
		}

		/// @brief Copy-assigns this `AnyError` with the given `error::Error<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param error - The error to assign to this
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(const Error<Domain>& error) noexcept->AnyError& {

			m_error_code = error.value();
			m_message = error.message();

			return *this;
		}

		/// @brief Move-assigns this `AnyError` with the given `error::Error<Domain>`
		///
		/// @tparam Domain - The `error::StatusCodeDomain` of the error
		///
		/// @param error - The error to assign to this
		/// @ingroup error
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(Error<Domain>&& error) noexcept->AnyError& {

			m_error_code = error.value();
			m_message = error.message();

			return *this;
		}

		/// @brief Provides `std::tuple`-like structured binding support.
		///
		/// `Index == 0` returns the error code value.
		/// `Index == 1` returns the error message.
		/// Other `Index` values ar invalid
		///
		/// @return The error code value (`Index == 0`), or the error message (`Index
		/// == 1`)
		/// @ingroup error
		template<usize Index>
		auto get() const&& noexcept {

			return get_impl<Index>(*this);
		}

		/// @brief Provides `std::tuple`-like structured binding support.
		///
		/// `Index == 0` returns the error code value.
		/// `Index == 1` returns the error message.
		/// Other `Index` values ar invalid
		///
		/// @return The error code value (`Index == 0`), or the error message (`Index
		/// == 1`)
		/// @ingroup error
		template<usize Index>
		auto get() const& noexcept {

			return get_impl<Index>(*this);
		}

	  private:
		template<usize Index, typename T>
		auto get_impl(T && val) const noexcept {

			static_assert(Index < 2, "Index out of bounds for hyperion::error::AnyError::get");
			if constexpr(Index == 0) {
				return std::forward<T>(val).m_error_code;
			}
			else {
				return std::forward<T>(val).m_message;
			}
		}

		i64 m_error_code = -1;

		std::string m_message;
	};

	IGNORE_WEAK_VTABLES_STOP
} // namespace hyperion::error

/// @brief Specialize `std::tuple_size` for `error::AnyError`
/// @ingroup error
template<>
struct std::tuple_size<hyperion::error::AnyError> {
	static constexpr hyperion::usize value = 2;
};

/// @brief Specialize `std::tuple_element<0, T>` for `error::AnyError`
/// @ingroup error
template<>
struct std::tuple_element<0, hyperion::error::AnyError> {
	using type = hyperion::i64;
};

/// @brief Specialize `std::tuple_element<1, T>` for `error::AnyError`
/// @ingroup error
template<>
struct std::tuple_element<1, hyperion::error::AnyError> {
	using type = std::string;
};
