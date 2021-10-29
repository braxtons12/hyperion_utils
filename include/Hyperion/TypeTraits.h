/// @file TypeTraits.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file provides a small collection of type traits useful for meta-programming
/// @version 0.1
/// @date 2021-10-15
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>
#include <tuple>
#include <type_traits>

/// @ingroup mpl
/// @{
///	@defgroup type_traits Type Traits
/// A small collection of type traits useful for meta programming
/// @headerfile "Hyperion/TypeTraits.h"
/// @}

namespace hyperion::type_traits {
	/// @brief Type Trait to determine if `T` is copyable or movable
	///
	/// @tparam T - The type to verify satisfies this Type Trait
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T>
	struct is_copy_or_move
		: std::bool_constant<std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>> {
	};

	/// @brief value of Type Trait  `is_copy_or_move`
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T>
	constexpr auto is_copy_or_move_v = is_copy_or_move<T>::value;

	/// @brief Type Trait to determine if `T` is copyable, movable, or a pointer type
	///
	/// @tparam T - The type to verify satisfies this Type Trait
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T>
	struct is_copy_move_or_pointer
		: std::bool_constant<is_copy_or_move_v<T> || std::is_pointer_v<T>> { };

	/// @brief value of Type Trait `is_copy_move_or_pointer`
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T>
	constexpr auto is_copy_move_or_pointer_v = is_copy_move_or_pointer<T>::value;

	/// @brief Type Trait to determine if type `T` is equality comparable with type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U = T, typename = std::void_t<>>
	struct equality_comparable : std::false_type { };

	/// @brief Type Trait to determine if type `T` is equality comparable with type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U>
	struct equality_comparable<T, U, std::void_t<decltype(std::declval<T>() == std::declval<U>())>>
		: std::true_type { };

	/// @brief Value of Type Trait `equality_comparable`
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U = T>
	inline static constexpr auto equality_comparable_v = equality_comparable<T, U>::value;

	/// @brief Type Trait to determine if type `T` is inequality comparable with type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U = T, typename = std::void_t<>>
	struct inequality_comparable : std::false_type { };

	/// @brief Type Trait to determine if type `T` is inequality comparable with type `U`,
	/// where `U` defaults to `T`
	///
	/// @tparam T - The LHS type
	/// @tparam U - The RHS type
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U>
	struct inequality_comparable<T,
								 U,
								 std::void_t<decltype(std::declval<T>() != std::declval<U>())>>
		: std::true_type { };

	/// @brief Value of Type Trait `inequality_comparable`
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename U = T>
	inline static constexpr auto inequality_comparable_v = inequality_comparable<T, U>::value;

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
	/// `T` in a non-evaluated context will compile (but whatever is relying on it will always
	/// fail), but attempting to do so __anywhere__ else will result in a compiler error because of
	/// failed overload resolution
	///
	/// @tparam T - The type to decl
	/// @return a `T`
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
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
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
	template<typename T, typename... Args>
	inline static constexpr auto is_constexpr_constructible(Args&&... args) noexcept -> bool {
		if constexpr(std::is_constructible_v<T, Args...>) {
			std::ignore = T(std::forward<Args>(args)...);
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
	/// @ingroup type_traits
	/// @headerfile "Hyperion/TypeTraits.h"
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
