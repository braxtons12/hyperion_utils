/// @brief `Err` represents an error
#pragma once

#include "../Concepts.h"

namespace hyperion {
	using concepts::NotReference, concepts::CopyConstructible, concepts::MoveConstructible,
		concepts::CopyAssignable, concepts::MoveAssignable, concepts::ConstructibleFrom,
		concepts::Pointer, concepts::NotPointer, concepts::ErrorType, concepts::Destructible;

	/// @brief `Err` represents an error value
	///
	/// @tparam E - The error type
	template<ErrorType E>
	requires NotReference<E>
	struct Err {
		/// @brief Constructs this `Err` from the given error value
		///
		/// @param error - The value representing an error
		explicit constexpr Err(E&& error) noexcept : m_error(std::forward<E>(error)) {
		}
		/// @brief Constructs the error value in place in this `Err` from the given `args`
		///
		/// @tparam Args - The types of arguments to pass to `E()`
		/// @param args - The arguments to pass to `E()`
		template<typename... Args>
		requires ConstructibleFrom<E, Args...>
		explicit constexpr Err(Args&&... args) noexcept : m_error(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		constexpr Err(const Err& err) noexcept requires CopyConstructible<E>
		= default;
		/// @brief Move Constructor
		constexpr Err(Err&& err) noexcept requires MoveConstructible<E>
		= default;
		/// @brief Destructor
		constexpr ~Err() noexcept = default;

		/// @brief Copy Assignment Operator
		constexpr auto operator=(const Err& err) noexcept -> Err& requires CopyAssignable<E>
		= default;
		/// @brief Move Assignment Operator
		constexpr auto operator=(Err&& err) noexcept -> Err& requires MoveAssignable<E>
		= default;

		/// @brief The error
		E m_error;
	};

	/// @brief CTAD Deduction Guide
	template<typename E>
	Err(E) -> Err<E>;
} // namespace hyperion
