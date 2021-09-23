#pragma once

#include "../Concepts.h"

namespace hyperion {
	template<typename T>
	class Option;

	/// @brief Convenience shorthand for `Option<T>::Some`
	///
	/// @param some - The value to store in the `Option`
	template<typename T,
			 typename U = std::conditional_t<concepts::Pointer<std::remove_reference_t<T>>,
											 std::remove_reference_t<T>,
											 T>>
	[[nodiscard]] inline constexpr auto Some(T&& some) noexcept -> Option<U>;

	/// @brief Convenience shorthand for `Option<T>::Some`. Constructs the `T` in place in the
	/// `Option`
	///
	/// @tparam T - The type to store in the `Option`
	/// @tparam Args - The types of arguments to pass to the `T` constructor
	/// @param args - The arguments to pass to the `T` constructor
	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] inline constexpr auto Some(Args&&... args) noexcept -> Option<T>;

	template<typename T>
	[[nodiscard]] inline constexpr auto
	Some(concepts::Convertible<T> auto&& some) noexcept -> Option<T>;

	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] constexpr inline auto
	make_option(Args&&... args) noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
		-> Option<T>;
} // namespace hyperion
