/// @brief `Ok` represents a valid or successful value
#pragma once

#include "../Concepts.h"

namespace hyperion {
	using concepts::NotReference, concepts::CopyConstructible, concepts::MoveConstructible,
		concepts::CopyAssignable, concepts::MoveAssignable, concepts::ConstructibleFrom,
		concepts::Pointer, concepts::NotPointer;

	/// @brief `Ok` represents a valid or successful value
	///
	/// @tparam T - The type of the successful value
	template<NotReference T>
	struct Ok {
		/// @brief Constructs this `Ok` from the given `T`
		///
		/// @param ok - The successful value
		explicit constexpr Ok(T&& ok) noexcept : m_ok(std::forward<T>(ok)) {
		}
		/// @brief Constructs a `T` in place in this `Ok`, passing the given `args` to the `T`'s
		/// constructor
		///
		/// @tparam Args - The types of the arguments to pass to `T()`
		/// @param args - The arguments to pass to `T()`
		template<typename... Args>
		requires ConstructibleFrom<T, Args...>
		explicit constexpr Ok(Args&&... args) noexcept : m_ok(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		constexpr Ok(const Ok& ok) noexcept requires CopyConstructible<T>
		= default;
		/// @brief Move Constructor
		constexpr Ok(Ok&& ok) noexcept requires MoveConstructible<T>
		= default;
		/// @brief Destructor
		constexpr ~Ok() noexcept = default;

		/// @brief Copy Assignment Operator
		constexpr auto operator=(const Ok& ok) noexcept -> Ok& requires CopyAssignable<T>
		= default;
		/// @brief Move Assignment Operator
		constexpr auto operator=(Ok&& ok) noexcept -> Ok& requires MoveAssignable<T>
		= default;

		/// @brief The value representing success
		T m_ok;
	};

	/// @brief CTAD Deduction Guide
	template<typename T>
	Ok(T) -> Ok<T>;
} // namespace hyperion
