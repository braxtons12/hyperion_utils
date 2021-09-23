/// @brief This is a small collection of basic type traits
#pragma once

#include <tuple>
#include <type_traits>

#include "BasicTypes.h"
#include "HyperionDef.h"

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

	namespace detail {
		/// @brief Alternative declval implementation
		/// @tparam T - The type to decl
		/// @return a `T`
		template<typename T, typename U = std::remove_cvref_t<T>>
		requires std::is_default_constructible_v<U>
		constexpr auto declval([[maybe_unused]] int i) noexcept -> U {
			return U();
		}
		/// @brief Alternative declval implementation
		/// @tparam T - The type to decl
		/// @return a `T`
		template<typename T, typename U = std::remove_cvref_t<T>>
		requires std::is_default_constructible_v<U>
		constexpr auto declval([[maybe_unused]] long i) noexcept -> U { // NOLINT
			return U();
		}
	} // namespace detail

	/// @brief Alternative declval implementation to use in evaluated contexts
	///
	/// An alternative to `std::declval<T>()` that can be used in evaluated contexts,
	/// enabling further possibilities in meta-programming situations.
	/// Its use comes with some caveats, mainly that `T` must be default constructible
	/// for this to work. This is enforced with a concept requirement on the internal
	/// implementation. This means that attempting to `declval` a non-default-constructible
	/// `T` in a concept definition will compile (but result in the concept always failing),
	/// but attempting to do so __anywhere__ else will result in a compiler error because of
	/// failed overload resolution
	///
	///
	/// @tparam T - The type to decl
	/// @return a `T`
	template<typename T>
	constexpr auto declval() noexcept -> decltype(detail::declval<T>(0)) {
		return detail::declval<T>(0);
	}

	IGNORE_UNUSED_TEMPLATES_START

	/// @brief Test function to determine if `T` is constructible from `Args...`
	/// in a constexpr context
	///
	/// This is used internally in our `ConstexprConstructibleFrom` and
	/// `ConstexprXXXXConstructible` concepts
	///
	/// @tparam T - The type to check
	/// @tparam Args - The types of arguments to pass to `T`'s constructor
	///
	/// @param args - The arguments to pass to `T`'s constructor
	/// @return `true` if `T` is constexpr constructible from `Args`, false otherwise
	template<typename T, typename... Args>
	inline static constexpr auto is_constexpr_constructible(Args... args) noexcept -> bool {
		if constexpr(std::is_constructible_v<T, Args...>) {
			std::ignore = T(args...);
			return std::is_constant_evaluated();
		}
		else {
			return false;
		}
	}

	/// @brief Test function to determine if `T` is assignable from `U`
	/// in a constexpr context
	///
	/// This is used internally in our `ConstexprAssignable` and
	/// `ConstexprXXXXAssignable` concepts
	///
	/// @tparam T - The type to check
	/// @tparam U - The type to pass to `T`'s assignment operator
	///
	/// @param u - The argument to pass to `T`'s assignment operator
	/// @return `true` if `T` is constexpr assignable from `U`, false otherwise
	template<typename T, typename U>
	inline static constexpr auto is_constexpr_assignable(U u) noexcept -> bool {
		if constexpr(std::is_assignable_v<T, U>) {
			[[maybe_unused]] T t = u;
			return std::is_constant_evaluated();
		}
		else {
			return false;
		}
	}

	IGNORE_UNUSED_TEMPLATES_STOP

} // namespace hyperion::type_traits
