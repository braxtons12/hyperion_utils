/// @brief `Err` represents an error
#pragma once

#include "../Concepts.h"
#include "../Error.h"

namespace hyperion {

	template<typename E = error::SystemError>
	struct Err;

	/// @brief `Err` represents an error value
	///
	/// @tparam E - The error type
	template<concepts::NotReference E>
	struct Err<E> {
		/// @brief Constructs this `Err` from the given error value
		///
		/// @param error - The value representing an error
		explicit constexpr Err(
			const E& error) noexcept requires concepts::NoexceptCopyConstructible<E>
			: m_error(error) {
		}
		/// @brief Constructs this `Err` from the given error value
		///
		/// @param error - The value representing an error
		explicit constexpr Err(E&& error) noexcept requires concepts::NoexceptMoveConstructible<E>
			: m_error(std::move(error)) {
		}
		explicit constexpr Err(concepts::Convertible<E> auto&& error) noexcept // NOLINT
			requires concepts::NotSame<const Err&,
									   decltype(error)> && concepts::NotSame<Err&&, decltype(error)>
			: m_error(std::forward<E>(static_cast<E>(error))) {
		}

		/// @brief Constructs the error value in place in this `Err` from the given `args`
		///
		/// @tparam Args - The types of arguments to pass to `E()`
		/// @param args - The arguments to pass to `E()`
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr Err(Args&&... args) noexcept : m_error(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		constexpr Err(const Err& err) noexcept requires concepts::NoexceptCopyConstructible<E>
		= default;
		/// @brief Move Constructor
		constexpr Err(Err&& err) noexcept requires concepts::NoexceptMoveConstructible<E>
		= default;
		/// @brief Destructor
		constexpr ~Err() noexcept = default;

		constexpr inline operator E() const noexcept { // NOLINT
			return m_error;
		}

		/// @brief Copy Assignment Operator
		constexpr auto
		operator=(const Err& err) noexcept -> Err& requires concepts::NoexceptCopyAssignable<E>
		= default;
		/// @brief Move Assignment Operator
		constexpr auto
		operator=(Err&& err) noexcept -> Err& requires concepts::NoexceptMoveAssignable<E>
		= default;

		/// @brief The error
		E m_error;
	};

	template<concepts::Reference E>
	struct Err<E> {
		using type = std::remove_reference_t<E>;
		/// @brief Constructs this `Err` from the given error value
		///
		/// @param error - The value representing an error
		explicit constexpr Err(const type& error) noexcept requires std::is_const_v<E>
			: m_error(error) {
		}
		/// @brief Constructs this `Err` from the given error value
		///
		/// @param error - The value representing an error
		explicit constexpr Err(type& error) noexcept : m_error(error) {
		}
		explicit constexpr Err(concepts::Convertible<E> auto&& error) noexcept // NOLINT
			requires concepts::NotSame<const Err&,
									   decltype(error)> && concepts::NotSame<Err&&, decltype(error)>
			: m_error(std::forward<E>(static_cast<E>(error))) {
		}
		/// @brief Copy Constructor
		constexpr Err(const Err& err) noexcept = default;
		/// @brief Move Constructor
		constexpr Err(Err&& err) noexcept : m_error(err.m_error) {
		}
		/// @brief Destructor
		constexpr ~Err() noexcept = default;

		constexpr inline operator E() const noexcept { // NOLINT
			return m_error;
		}

		/// @brief Copy Assignment Operator
		constexpr auto operator=(const Err& err) noexcept -> Err& = default;
		/// @brief Move Assignment Operator
		constexpr auto operator=(Err&& err) noexcept -> Err& {
			m_error = err.m_error;
		}

		/// @brief The error
		std::reference_wrapper<type> m_error;
	};

	/// @brief CTAD Deduction Guide
	template<typename E>
	Err(E) -> Err<E>;

	/// @brief Constructs an `Err` from the given error
	///
	/// @tparam E - The error type
	///
	/// @param err - The error to construct an `Err` from
	/// @return an `Err<E>`
	template<typename E>
	inline constexpr auto make_err(E&& err) noexcept -> Err<E> {
		return Err<E>(std::forward<E>(err));
	}

	/// @brief Constructs an `Err` from the given arguments
	///
	/// Constructs an `Err` holding the error type, `E`, constructing the `E` in place within the
	/// `Err`
	///
	/// @tparam E - The error type
	/// @tparam Args - The types of the arguments to construct the error type from
	///
	/// @param args - The arguments to construct the error from
	///
	/// # Requirements
	/// - `concepts::NoexceptConstructibleFrom<E, Args...>`: `E` must be noexcept constructible from
	/// `args` in order to construct an `Err<E>` from them
	///
	/// @return an `Err<E>`
	template<typename E, typename... Args>
	requires concepts::NoexceptConstructibleFrom<E, Args...>
	inline constexpr auto make_err(Args&&... args) noexcept -> Err<E> {
		return Err<E>(std::forward<Args>(args)...);
	}

	/// @brief Constructs an `Err` from the given error
	///
	/// @tparam E - The error type
	///
	/// @param err - The error to construct an `Err` from
	/// @return an `Err<E>`
	template<typename E>
	inline constexpr auto make_err(concepts::Convertible<E> auto&& err) noexcept -> Err<E> {
		return Err<E>(E(err));
	}
} // namespace hyperion
