#pragma once

#include "../HyperionDef.h"
#include "Panic.h"
#include "StatusCodeDomain.h"

namespace hyperion::error {

	template<typename T>
	struct status_code_enum_info {
		using domain_type = void;
		static constexpr bool value = false;
	};

	/// @brief Type trait to determine if a type is an enum associated with a `StatusCodeDomain`
	///
	/// Determines if a type is an enum registered as the value type of a `StatusCodeDomain`, and
	/// provides the type of that `StatusCodeDomain` as `is_status_code_enum<T>::domain_type`.
	/// To register an enum as a `StatusCodeDomain` value type, specialize
	/// `status_code_enum_info<T>` for your enum
	///
	/// @tparam T - The type to check if it is a status code enum
	template<typename T>
	struct is_status_code_enum : public status_code_enum_info<T> { };

	template<typename T>
	static constexpr auto is_status_code_enum_v = is_status_code_enum<T>::value;

	template<typename T>
	concept StatusCodeEnum = is_status_code_enum_v<T> && std::is_enum_v<T> && StatusCodeDomain<
		typename status_code_enum_info<T>::domain_type>;

	template<typename T>
	requires StatusCodeEnum<T>
	using status_code_enum_domain = typename is_status_code_enum<T>::domain_type;

	template<typename T, typename Domain>
	requires StatusCodeEnum<T> && StatusCodeDomain<
		Domain> && std::same_as<status_code_enum_domain<T>, Domain>
	using status_code_enum_domain_checked = status_code_enum_domain<T>;
} // namespace hyperion::error

constexpr auto make_status_code(hyperion::error::StatusCodeEnum auto code) noexcept
	-> hyperion::error::StatusCode<hyperion::error::status_code_enum_domain<decltype(code)>>;

namespace hyperion::error {

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
	template<typename Domain>
	class [[nodiscard]] StatusCode {
	  public:
		using value_type = typename Domain::value_type;

		constexpr StatusCode() noexcept = default;
		explicit constexpr StatusCode(i64 code,
									  Domain&& domain = make_status_code_domain<Domain>()) noexcept
			// clang-format off
			requires StatusCodeDomain<Domain>
			: m_domain(std::forward<Domain>(domain)), m_code(static_cast<value_type>(code))
		{
			// clang-format on
		}

		explicit constexpr StatusCode(StatusCodeEnum auto code) noexcept
			// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
			: m_domain(make_status_code_domain<Domain>()), m_code(code)
		{
			// clang-format on
		}

		constexpr StatusCode(const StatusCode&) noexcept requires StatusCodeDomain<Domain>
		= default;
		constexpr StatusCode(StatusCode&&) noexcept requires StatusCodeDomain<Domain>
		= default;
		constexpr ~StatusCode() noexcept requires StatusCodeDomain<Domain>
		= default;

		constexpr inline auto assign(i64 code) noexcept -> void requires StatusCodeDomain<Domain> {
			m_code = static_cast<value_type>(code);
		}

		constexpr inline auto assign(StatusCodeEnum auto code) noexcept -> void
			// clang-format off
			requires StatusCodeDomain<Domain>
			&& std::same_as<Domain, status_code_enum_domain<decltype(code)>>
		{
			// clang-format on
			*this = make_status_code(code);
		}

		constexpr inline auto clear() noexcept -> void requires StatusCodeDomain<Domain> {
			m_code = m_domain.success_value();
		}

		[[nodiscard]] constexpr inline auto
		value() const noexcept -> i64 requires StatusCodeDomain<Domain> {
			return static_cast<i64>(m_code);
		}

		[[nodiscard]] constexpr inline auto
		code() const noexcept -> value_type requires StatusCodeDomain<Domain> {
			return m_code;
		}

		[[nodiscard]] constexpr inline auto
		message() const noexcept requires StatusCodeDomain<Domain> {
			return m_domain.message(*this);
		}

		[[nodiscard]] constexpr inline auto
		is_error() const noexcept -> bool requires StatusCodeDomain<Domain> {
			return m_domain.is_error(*this);
		}

		[[nodiscard]] constexpr inline auto
		is_success() const noexcept -> bool requires StatusCodeDomain<Domain> {
			return m_domain.is_success(*this);
		}

		template<typename Domain2>
		[[nodiscard]] constexpr inline auto
		is_equivalent(const StatusCode<Domain2>& rhs) const noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return m_domain.are_equivalent(*this, rhs);
		}

		[[nodiscard]] constexpr inline auto as_generic_code() const noexcept -> bool requires
			StatusCodeDomain<Domain> && ConvertibleToGenericStatusCode<StatusCode> {
			return m_domain.as_generic_code(*this);
		}

		[[nodiscard]] constexpr inline auto
		domain() const noexcept -> const Domain& requires StatusCodeDomain<Domain> {
			return m_domain;
		}

		[[nodiscard]] explicit constexpr inline
		operator bool() const noexcept requires StatusCodeDomain<Domain> {
			return is_success();
		}

		[[nodiscard]] explicit constexpr inline
		operator value_type() const noexcept requires StatusCodeDomain<Domain> {
			return m_code;
		}

		template<typename Domain2>
		friend constexpr inline auto
		operator==(const StatusCode<Domain>& lhs, const StatusCode<Domain2>& rhs) noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return lhs.is_equivalent(rhs);
		}

		template<typename Domain2>
		friend constexpr inline auto
		operator!=(const StatusCode<Domain>& lhs, const StatusCode<Domain2>& rhs) noexcept -> bool
			// clang-format off
			requires StatusCodeDomain<Domain> && StatusCodeDomain<Domain2>
		{
			// clang-format on
			return !lhs.is_equivalent(rhs);
		}

		constexpr auto
		operator=(const StatusCode& code) noexcept -> StatusCode& requires StatusCodeDomain<Domain>
		= default;

		constexpr auto
		operator=(StatusCode&&) noexcept -> StatusCode& requires StatusCodeDomain<Domain>
		= default;

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

	template<typename Domain>
	class [[nodiscard]] ErrorCode final : public StatusCode<Domain> {
	  public:
		using value_type = typename StatusCode<Domain>::value_type;

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

		explicit constexpr ErrorCode(StatusCodeEnum auto code) noexcept // NOLINT
																		// clang-format off
			requires StatusCodeEnum<decltype(code)>
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
		constexpr ErrorCode(const ErrorCode& code) noexcept = default;
		constexpr ErrorCode(ErrorCode&& code) noexcept = default;
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

		template<typename Domain2>
		friend constexpr inline auto
		operator==(const ErrorCode<Domain>& lhs, const ErrorCode<Domain2>& rhs) noexcept -> bool {
			return lhs.is_equivalent(rhs);
		}

		template<typename Domain2>
		friend constexpr inline auto
		operator!=(const ErrorCode<Domain>& lhs, const ErrorCode<Domain2>& rhs) noexcept -> bool {
			return !lhs.is_equivalent(rhs);
		}

		constexpr auto
		operator=(const ErrorCode& code) noexcept -> ErrorCode& requires StatusCodeDomain<Domain>
		= default;

		constexpr auto
		operator=(ErrorCode&&) noexcept -> ErrorCode& requires StatusCodeDomain<Domain>
		= default;

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
	IGNORE_PADDING_STOP
} // namespace hyperion::error

template<typename Domain>
constexpr auto
make_status_code(hyperion::i64 code) noexcept -> hyperion::error::StatusCode<Domain> {
	return hyperion::error::StatusCode<Domain>(code);
}

template<typename Domain>
constexpr auto make_status_code(hyperion::i64 code, Domain&& domain) noexcept
	-> hyperion::error::StatusCode<Domain> {
	return hyperion::error::StatusCode(code, std::forward<Domain>(domain));
}

constexpr auto make_status_code(hyperion::error::StatusCodeEnum auto code) noexcept
	-> hyperion::error::StatusCode<hyperion::error::status_code_enum_domain<decltype(code)>> {

	return hyperion::error::StatusCode(code);
}

template<typename Domain>
constexpr auto make_error_code(hyperion::i64 code) noexcept -> hyperion::error::ErrorCode<Domain> {
	return hyperion::error::ErrorCode<Domain>(code);
}

template<typename Domain>
constexpr auto make_error_code(hyperion::i64 code, Domain&& domain) noexcept
	-> hyperion::error::ErrorCode<Domain> {
	return hyperion::error::ErrorCode(code, std::forward<Domain>(domain));
}

constexpr auto make_error_code(hyperion::error::StatusCodeEnum auto code) noexcept
	-> hyperion::error::ErrorCode<hyperion::error::status_code_enum_domain<decltype(code)>> {
	return hyperion::error::ErrorCode(code);
}
