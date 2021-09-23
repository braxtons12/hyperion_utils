/// @brief Error is the base type for communicating recoverable errors inside Hyperion
///
/// Error speaks `std::error_code` as well as custom error messages, and combined with `Result`
/// is easily composable and propogated
#pragma once

#include <cstring>
#include <gsl/gsl>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>

#include "BasicTypes.h"
#include "HyperionDef.h"
#include "Ignore.h"
#include "Memory.h"
#include "error/SystemDomain.h"

namespace hyperion::error {
	using namespace std::literals::string_literals;

	IGNORE_WEAK_VTABLES_START
	class [[nodiscard]] ErrorBase {
	  public:
		constexpr ErrorBase() noexcept = default;
		constexpr ErrorBase(const ErrorBase&) noexcept = default;
		constexpr ErrorBase(ErrorBase&&) noexcept = default;
		constexpr virtual ~ErrorBase() noexcept = default;

		[[nodiscard]] virtual constexpr auto value() const noexcept -> i64 = 0;
		[[nodiscard]] virtual auto message() const noexcept -> std::string = 0;
		[[nodiscard]] virtual auto to_string() const noexcept -> std::string = 0;

		constexpr auto operator=(const ErrorBase&) noexcept -> ErrorBase& = default;
		constexpr auto operator=(ErrorBase&&) noexcept -> ErrorBase& = default;
	};
	IGNORE_WEAK_VTABLES_STOP

	class [[nodiscard]] AnyError;

	/// @brief Base error interface.
	/// Used to implement custom error types used as the `E` in `Result<T, E>`
	/// to represent and communicate failure of a function
	///
	/// @see `Result<T, E>`
	template<StatusCodeDomain Domain = SystemDomain>
	class [[nodiscard]] Error final : public ErrorBase {
	  public:
		using value_type = typename ErrorCode<Domain>::value_type;

		friend class AnyError;

		/// Constructs a default `Error` with no message
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
		constexpr Error(const ErrorCode<Domain>& code) noexcept // NOLINT
			: m_error_code(code) {
		}

		/// @brief Constructs an `Error` from the given `ErrorCode<Domain>`
		///
		/// @param code - The error code
		constexpr Error(ErrorCode<Domain>&& code) noexcept // NOLINT
			: m_error_code(std::move(code)) {
		}

		constexpr Error(const value_type& code) noexcept // NOLINT
			requires(!StatusCodeEnum<value_type>)
			: m_error_code(make_error_code<Domain>(code)) {
		}

		constexpr Error(const value_type& code) noexcept // NOLINT
			requires StatusCodeEnum<value_type> : m_error_code(make_error_code(code)) {
		}

		constexpr Error(const Error& error) = default;
		constexpr Error(Error&& error) noexcept = default;

		constexpr ~Error() noexcept final = default;

		/// @brief Returns the `std::error_code` associated with this `Error`
		///
		/// @return The associated `std::error_code`
		[[nodiscard]] auto value() const noexcept -> i64 final {
			return m_error_code.value();
		}

		/// @brief Returns the `std::error_code` associated with this `Error`
		///
		/// @return The associated `std::error_code`
		[[nodiscard]] auto code() const noexcept -> const ErrorCode<Domain>& {
			return m_error_code;
		}

		/// @brief Returns the error message for this `Error`
		///
		/// @return The error message
		[[nodiscard]] inline auto message() const noexcept -> std::string final {
			return to_string();
		}

		/// @brief Returns the error message for this `Error`
		///
		/// @return The error message
		[[nodiscard]] auto message_as_cstr() const noexcept -> const char* {
			return m_error_code.message().c_str();
		}

		/// @brief Converts this `Error` to a `std::string`
		/// Generally implemented by combining the `source`'s `to_std_string`
		/// and this `Error`'s `message`
		///
		/// @return this `Error` formatted as a `std::string`
		[[nodiscard]] auto to_string() const noexcept -> std::string final {
			if constexpr(hyperion::concepts::Same<error_code_message_type, std::string_view>) {
				return "Error: "s + std::string(m_error_code.message());
			}
			else {
				return "Error: "s + m_error_code.message();
			}
		}

		constexpr operator ErrorCode<Domain>() const noexcept { // NOLINT
			return m_error_code;
		}

		constexpr operator value_type() const noexcept { // NOLINT
			return static_cast<value_type>(m_error_code);
		}

		constexpr auto operator=(const Error& error) noexcept -> Error& = default;
		constexpr auto operator=(Error&& error) noexcept -> Error& = default;
		constexpr auto operator=(ErrorCode<Domain>&& code) noexcept -> Error& {
			m_error_code = code;

			return *this;
		}

		template<usize Index>
		auto get() const&& noexcept {
			return get_impl<Index>(*this);
		}

		template<usize Index>
		auto get() const& noexcept {
			return get_impl<Index>(*this);
		}

	  private:
		template<usize Index, typename T>
		auto get_impl(T&& t) const noexcept {
			static_assert(Index < 2, "Index out of bounds for hyperion::error::Error::get");
			if constexpr(Index == 0) {
				return std::forward<T>(t).m_error_code.value();
			}
			else {
				return std::forward<T>(t).m_error_code.message();
			}
		}
		/// error code
		ErrorCode<Domain> m_error_code;

		using error_code_message_type = decltype(m_error_code.message());
	};
} // namespace hyperion::error

template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_size<hyperion::error::Error<Domain>> {
	static constexpr hyperion::usize value = 2;
};

template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_element<0, hyperion::error::Error<Domain>> {
	using type = typename hyperion::error::Error<Domain>::value_type;
};

template<hyperion::error::StatusCodeDomain Domain>
struct std::tuple_element<1, hyperion::error::Error<Domain>> {
	using type = std::string;
};

namespace hyperion::error {

	using SystemError = Error<SystemDomain>;
	using PosixError = Error<PosixDomain>;
	using GenericError = Error<GenericDomain>;

#if HYPERION_PLATFORM_WINDOWS
	using Win32Error = Error<Win32Domain>;
	using NTError = Error<NTDomain>;
#endif // HYPERION_PLATFORM_WINDOWS

	/// @brief Concept that requires `E` to be an `ErrorType`
	/// (aka derived from `hyperion::Error`)
	template<typename E>
	concept ErrorType = std::derived_from<E, ErrorBase>;

	/// @brief Concept that requires `E` is not an `ErrorType`
	/// (aka __NOT__ derived from `hyperion::Error`)
	template<typename E>
	concept NotErrorType = !ErrorType<E>;

	class [[nodiscard]] AnyError final : public ErrorBase {
	  public:
		HYPERION_CONSTEXPR_STRINGS AnyError() noexcept = default;
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(const ErrorCode<Domain>& code) noexcept // NOLINT
			: m_error_code(code.value()), m_message(code.message()) {
		}
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS AnyError(Error<Domain>&& error) noexcept // NOLINT
			: m_error_code(error.code().value()), m_message(error.message()) {
		}
		HYPERION_CONSTEXPR_STRINGS AnyError(const AnyError&) noexcept = delete;
		HYPERION_CONSTEXPR_STRINGS AnyError(AnyError&&) noexcept = default;
		HYPERION_CONSTEXPR_STRINGS ~AnyError() noexcept final = default;

		[[nodiscard]] constexpr auto value() const noexcept -> i64 final {
			return m_error_code;
		}

		[[nodiscard]] auto message() const noexcept -> std::string final {
			return "Error: "s + m_message;
		}

		[[nodiscard]] HYPERION_CONSTEXPR_STRINGS auto
		to_string() const noexcept -> std::string final {
			return "Error: "s + std::to_string(m_error_code) + "\n"s;
		}

		HYPERION_CONSTEXPR_STRINGS auto operator=(const AnyError&) noexcept -> AnyError& = delete;
		HYPERION_CONSTEXPR_STRINGS auto operator=(AnyError&&) noexcept -> AnyError& = default;
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto
		operator=(const ErrorCode<Domain>& code) noexcept -> AnyError& {
			m_error_code = code.value();
			m_message = code.message();

			return *this;
		}
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(ErrorCode<Domain>&& code) noexcept -> AnyError& {
			m_error_code = code.value();
			m_message = code.message();

			return *this;
		}
		template<StatusCodeDomain Domain>
		HYPERION_CONSTEXPR_STRINGS auto operator=(Error<Domain>&& error) noexcept -> AnyError& {
			m_error_code = error.value();
			m_message = error.message();

			return *this;
		}

	  private:
		i64 m_error_code = 0;
		std::string m_message;
	};
} // namespace hyperion::error
