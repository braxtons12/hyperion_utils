#pragma once

#include <tuple>
#include <type_traits>

#include "BasicTypes.h"

namespace hyperion::utils::type_traits {
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

	/// @brief Type Trait to determine if the given parameter pack `Types` contains the type `T`
	///
	/// @tparam T - The type to search for
	/// @tparam Types - The parameter pack to search in
	template<typename T, typename... Types>
	struct contains : std::true_type { };

	/// @brief Type Trait to determine if the given parameter pack `Types` contains the type `T`
	///
	/// @tparam T - The type to search for
	/// @tparam Head - The first type in the parameter pack
	/// @tparam Types - The parameter pack to search in
	template<typename T, typename Head, typename... Types>
	struct contains<T, Head, Types...>
		: std::conditional_t<std::is_same_v<T, Head>, std::true_type, contains<T, Types...>> { };

	/// @brief Type Trait to determine if the given parameter pack `Types` contains the type `T`
	///
	/// @tparam T - The type to search for
	template<typename T>
	struct contains<T> : std::false_type { };

	/// @brief Value of Type Trait `contains`. Used to determine if the given parameter pack `Types`
	/// contains the given type `T`
	template<typename T, typename... Types>
	static inline constexpr auto contains_v = contains<T, Types...>::value;

	/// @brief Type Trait to find the `N`th type in the parameter pack `Types`
	///
	/// @tparam N - The index in the parameter pack to get the type of
	/// @tparam Types - The parameter pack to search in
	template<usize N, typename... Types>
	struct type_index : std::tuple_element<N, std::tuple<Types...>> { };

	/// @brief Type of Type Trait `type_index`. Used to get the `N`th type from the parameter pack
	/// `Types`
	///
	/// @tparam N - The index in the parameter pack to get the type of
	/// @tparam Types - The parameter pack to search in
	template<usize N, typename... Types>
	using type_index_t = typename type_index<N, Types...>::type;

} // namespace hyperion::utils::type_traits
