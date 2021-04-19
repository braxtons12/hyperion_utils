/// @brief This is a small collection of basic type traits
#pragma once

#include <tuple>
#include <type_traits>

#include "BasicTypes.h"

namespace hyperion::type_traits {
	/// @brief Type Trait to determine if `T` is copyable or movable
	///
	/// @tparam T - The type to verify satisfies this Type Trait
	template<typename T>
	struct is_copy_or_move
		: std::bool_constant<std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>> {
	};

	/// @brief value of Type Trait  `is_copy_or_move`
	template<typename T>
	constexpr auto is_copy_or_move_v = is_copy_or_move<T>::value;

	/// @brief Type Trait to determine if `T` is copyable, movable, or a pointer type
	///
	/// @tparam T - The type to verify satisfies this Type Trait
	template<typename T>
	struct is_copy_move_or_pointer
		: std::bool_constant<is_copy_or_move_v<T> || std::is_pointer_v<T>> { };

	/// @brief value of Type Trait `is_copy_move_or_pointer`
	template<typename T>
	constexpr auto is_copy_move_or_pointer_v = is_copy_move_or_pointer<T>::value;

	/// @brief Type Trait to determine if `T` has the `!=` operator comparing to type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	template<typename T, typename U = T, typename = std::void_t<>>
	struct has_not_equal : std::false_type { };

	/// @brief Type Trait to determine if `T` has the `!=` operator comparing to type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	template<typename T, typename U>
	struct has_not_equal<T, U, std::void_t<decltype(std::declval<T>() != std::declval<U>())>>
		: std::true_type { };

	/// @brief Value of Type Trait `has_not_equal`
	template<typename T, typename U = T>
	static inline constexpr auto has_not_equal_v = has_not_equal<T, U>::value;
} // namespace hyperion::type_traits
