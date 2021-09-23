/// @brief `Ok` represents a valid or successful value
#pragma once

#include "../Concepts.h"

namespace hyperion {

	/// @brief `Ok` represents a valid or successful value
	///
	/// @tparam T - The type of the successful value
	template<typename T>
	struct Ok;

	template<concepts::NotReference T>
	struct Ok<T> {
		/// @brief Constructs this `Ok` from the given `T`
		///
		/// @param ok - The successful value
		explicit constexpr Ok(const T& ok) noexcept requires concepts::NoexceptCopyConstructible<T>
			: m_ok(ok) {
		}
		/// @brief Constructs this `Ok` from the given `T`
		///
		/// @param ok - The successful value
		explicit constexpr Ok(T&& ok) noexcept requires concepts::NoexceptMoveConstructible<T>
			: m_ok(std::move(ok)) {
		}
		explicit constexpr Ok(concepts::Convertible<T> auto&& ok) noexcept // NOLINT
			requires
			concepts::NotSame<const Ok&, decltype(ok)> && concepts::NotSame<Ok&&, decltype(ok)>
			: m_ok(std::forward<T>(static_cast<T>(ok))) {
		}

		/// @brief Constructs a `T` in place in this `Ok`, passing the given `args` to the `T`'s
		/// constructor
		///
		/// @tparam Args - The types of the arguments to pass to `T()`
		/// @param args - The arguments to pass to `T()`
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr Ok(Args&&... args) noexcept : m_ok(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		constexpr Ok(const Ok& ok) noexcept requires concepts::NoexceptCopyConstructible<T>
		= default;
		/// @brief Move Constructor
		constexpr Ok(Ok&& ok) noexcept requires concepts::NoexceptMoveConstructible<T>
		= default;
		/// @brief Destructor
		constexpr ~Ok() noexcept = default;

		constexpr inline operator T() const noexcept { // NOLINT
			return m_ok;
		}

		/// @brief Copy Assignment Operator
		constexpr auto
		operator=(const Ok& ok) noexcept -> Ok& requires concepts::NoexceptCopyAssignable<T>
		= default;
		/// @brief Move Assignment Operator
		constexpr auto
		operator=(Ok&& ok) noexcept -> Ok& requires concepts::NoexceptMoveAssignable<T>
		= default;

		/// @brief The value representing success
		T m_ok;
	};

	template<concepts::Reference T>
	struct Ok<T> {
		using type = std::remove_reference_t<T>;

		/// @brief Constructs this `Ok` from the given `T`
		///
		/// @param ok - The successful value
		explicit constexpr Ok(const T& ok) noexcept requires std::is_const_v<T> : m_ok(ok) {
		}
		/// @brief Constructs this `Ok` from the given `T`
		///
		/// @param ok - The successful value
		explicit constexpr Ok(T& ok) noexcept : m_ok(ok) {
		}
		explicit constexpr Ok(concepts::Convertible<T> auto&& ok) noexcept // NOLINT
			requires
			concepts::NotSame<const Ok&, decltype(ok)> && concepts::NotSame<Ok&&, decltype(ok)>
			: m_ok(std::forward<T>(static_cast<T>(ok))) {
		}

		/// @brief Copy Constructor
		constexpr Ok(const Ok& ok) noexcept = default;
		/// @brief Move Constructor
		constexpr Ok(Ok&& ok) noexcept : m_ok(ok.m_ok) {
		}
		/// @brief Destructor
		constexpr ~Ok() noexcept = default;

		constexpr inline operator T() const noexcept { // NOLINT
			return m_ok;
		}
		/// @brief Copy Assignment Operator
		constexpr auto operator=(const Ok& ok) noexcept -> Ok& = default;
		/// @brief Move Assignment Operator
		constexpr auto operator=(Ok&& ok) noexcept -> Ok& {
			m_ok = ok.m_ok;
		}

		/// @brief The value representing success
		std::reference_wrapper<type> m_ok;
	};

	/// @brief CTAD Deduction Guide
	template<typename T>
	Ok(T) -> Ok<T>;

	/// @brief Constructs an `Ok` from the given ok
	///
	/// @tparam T - The ok type
	///
	/// @param ok - The ok to construct an `Ok` from
	/// @return an `Ok<T>`
	template<typename T>
	inline constexpr auto make_ok(T&& ok) noexcept -> Ok<T> {
		return Ok<T>(std::forward<T>(ok));
	}

	/// @brief Constructs an `Ok` from the given arguments
	///
	/// Constructs an `Ok` holding the ok type, `T`, constructing the `T` in place within the
	/// `Ok`
	///
	/// @tparam T - The ok type
	/// @tparam Args - The types of the arguments to construct the ok type from
	///
	/// @param args - The arguments to construct the ok from
	///
	/// # Requirements
	/// - `concepts::NoexceptConstructibleFrom<T, Args...>`: `T` must be noexcept constructible
	/// from `args` in order to construct an `Ok<T>` from them
	///
	/// @return an `Ok<T>`
	template<typename T, typename... Args>
	requires concepts::NoexceptConstructibleFrom<T, Args...>
	inline constexpr auto make_ok(Args&&... args) noexcept -> Ok<T> {
		return Ok<T>(std::forward<Args>(args)...);
	}

	/// @brief Constructs an `Ok` from the given ok
	///
	/// @tparam T - The ok type
	///
	/// @param ok - The ok to construct an `Ok` from
	/// @return an `Ok<T>`
	template<typename T>
	inline constexpr auto make_ok(concepts::Convertible<T> auto&& ok) noexcept -> Ok<T> {
		return Ok<T>(E(ok));
	}
} // namespace hyperion
