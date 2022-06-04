/// @file Concepts.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Aliases for and additions to the C++20 standard concepts library
/// @version 0.1
/// @date 2022-06-04
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

#include <Hyperion/TypeTraits.h>
#include <Hyperion/mpl/ForAll.h>
#include <Hyperion/mpl/List.h>
#include <concepts>
#include <type_traits>

/// @ingroup mpl
/// @{
///	@defgroup concepts Concepts
/// Aliases for and additions to the C++20 standard concepts library
/// @headerfile "Hyperion/Concepts.h"
/// @}

namespace hyperion::concepts {

	/// @brief Alias for `std::floating_point<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept FloatingPoint = std::floating_point<T>;

	/// @brief Alias for `std::integral<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Integral = std::integral<T>;

	/// @brief Alias for `std::unsigned_integral<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept UnsignedIntegral = std::unsigned_integral<T>;

	/// @brief Alias for `std::signed_integral<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept SignedIntegral = std::signed_integral<T>;

	/// @brief Concept that requires the type to be a numeric type
	/// ie: FloatingPoint<T> or Integral<T> is true
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Numeric = FloatingPoint<T> || Integral<T>;

	/// @brief Concept that requires the type to be a numeric type
	/// ie: FloatingPoint<T> or Integral<T> is true
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept SignedNumeric = FloatingPoint<T> || SignedIntegral<T>;

	/// @brief Concept requiring `T` to be a pointer
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Pointer = std::is_pointer_v<T>;

	/// @brief Concept requiring `T` is __NOT__ a pointer
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotPointer = !std::is_pointer_v<T>;

	/// @brief Concept requiring `T` is a reference (lvalue or rvalue)
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Reference = std::is_reference_v<T>;

	/// @brief Concept requiring `T` is __NOT__ a reference
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotReference = !Reference<T>;

	/// @brief Concept requiring `T` is an rvalue reference
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept RValueReference = std::is_rvalue_reference_v<T>;

	/// @brief Concept requiring `T` is __NOT__ an rvalue reference
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotRValueReference = !RValueReference<T>;

	/// @brief Concept requiring `T` is an lvalue reference
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept LValueReference = std::is_lvalue_reference_v<T>;

	/// @brief Concept requiring `T` is __NOT__ an lvalue reference
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotLValueReference = !LValueReference<T>;

	/// @brief Concept requiring that `T` is a function
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Function = std::is_function_v<T>;

	/// @brief Concept requiring that `T` is __NOT__ a function
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotFunction = !Function<T>;

	/// @brief Concept requiring that `Der` is derived from `Base`
	///
	/// `Der` will be tested as the unqualified, pointer-removed underlying type if `Der` is not a
	/// raw type (it will be tested as `std::remove_pointer_t<std::decay_t<Der>>`).
	/// EG: if `Der` is `const volatile T*`, it will be tested as `T`
	///
	/// `Base` will be tested as the unqualified, pointer-removed underlying type if `Base` is not a
	/// raw type (eg, it will be tested as `std::remove_pointer_t<std::decay_t<Base>>`)
	/// EG: if `Base` is `const volatile U*`, it will be tested as `U`
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename Der, typename Base>
	concept Derived = std::derived_from<std::remove_pointer_t<std::decay_t<Der>>,
										std::remove_pointer_t<std::decay_t<Base>>>;

	/// @brief Concept that requires `From` is convertible to `To`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename From, typename To>
	concept Convertible = std::is_convertible_v<From, To>;

	/// @brief Concept that requires `From` is __not__ convertible to `To`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename From, typename To>
	concept NotConvertible = !Convertible<From, To>;

	/// @brief Concept that requires `T` to be constructible from the parameter pack `Args`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename... Args>
	concept ConstructibleFrom = std::is_constructible_v<T, Args...>;

	/// @brief Concept that requires `T` to be noexcept constructible from the parameter pack `Args`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename... Args>
	concept NoexceptConstructibleFrom = std::is_nothrow_constructible_v<T, Args...>;

	/// @brief Concept that requires `T` and `U` to be the same type
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U>
	concept Same = std::same_as<T, U>;

	/// @brief Concept that requires `T` and `U` to be the same type
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U>
	concept NotSame = !Same<T, U>;

	/// @brief Concept requiring `T` to be inequality comparable to `U`
	/// (`T` has `operator!=` for `U`)
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept InequalityComparable = type_traits::inequality_comparable_v<T, U>;

	/// @brief  Concept requiring that the `mpl::list`, `List`, contains the type `T`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename List>
	concept Contains = mpl::contains_v<T, List>;

	/// @brief Concept that requires that `T` is copy constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept CopyConstructible = std::is_copy_constructible_v<T>;

	/// @brief Concept that requires that `T` is trivially copy constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllCopyConstructible
		= mpl::for_all_types_v<std::is_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyCopyConstructible
		= mpl::for_all_types_v<std::is_trivially_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is copy assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept CopyAssignable = std::is_copy_assignable_v<T>;

	/// @brief Concept that requires that `T` is copy assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllCopyAssignable = mpl::for_all_types_v<std::is_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyCopyAssignable
		= mpl::for_all_types_v<std::is_trivially_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept copy constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptCopyConstructible
		= mpl::for_all_types_v<std::is_nothrow_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept copy assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptCopyAssignable = std::is_nothrow_copy_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptCopyAssignable
		= mpl::for_all_types_v<std::is_nothrow_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is move constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept MoveConstructible = std::is_move_constructible_v<T>;

	/// @brief Concept that requires that `T` is __not__ move constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotMoveConstructible = !std::is_move_constructible_v<T>;

	/// @brief Concept that requires that `T` is move constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllMoveConstructible
		= mpl::for_all_types_v<std::is_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyMoveConstructible
		= mpl::for_all_types_v<std::is_trivially_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept MoveAssignable = std::is_move_assignable_v<T>;

	/// @brief Concept that requires that `T` is __not__ move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotMoveAssignable = !std::is_move_assignable_v<T>;

	/// @brief Concept that requires that `T` is move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllMoveAssignable = mpl::for_all_types_v<std::is_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyMoveAssignable
		= mpl::for_all_types_v<std::is_trivially_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is assignable from `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U>
	concept Assignable = std::is_assignable_v<T, U>;

	/// @brief Concept that requires that `T` is noexcept assignable from `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U>
	concept NoexceptAssignable = std::is_nothrow_assignable_v<T, U>;

	/// @brief Concept that requires that `T` is noexcept move constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptMoveConstructible = std::is_nothrow_move_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptMoveConstructible
		= mpl::for_all_types_v<std::is_nothrow_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptMoveAssignable = std::is_nothrow_move_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptMoveAssignable
		= mpl::for_all_types_v<std::is_nothrow_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is destructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Destructible = std::is_destructible_v<T>;

	/// @brief Concept that requires that `T` is noexcept destructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

	/// @brief Concept that requires that `T` is noexcept destructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptDestructible = std::is_nothrow_destructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow
	/// destructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyDestructible
		= mpl::for_all_types_v<std::is_trivially_destructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow
	/// destructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptDestructible
		= mpl::for_all_types_v<std::is_nothrow_destructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is move constructible and move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Movable = MoveConstructible<T> && MoveAssignable<T>;

	/// @brief Concept that requires that `T` is __NOT__ move constructible or __NOT__ move
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotMovable = !Movable<T>;

	/// @brief Concept that requires that `T` is noexcept move constructible and noexcept move
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptMovable = NoexceptMoveConstructible<T> && NoexceptMoveAssignable<T>;

	/// @brief Concept that requires that `T` is __NOT__ noexcept move constructible or __NOT__
	/// noexcept move assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptNotMovable = !NoexceptMovable<T>;

	/// @brief Concept that requires that `T` is copy constructible and copy assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Copyable = CopyConstructible<T> && CopyAssignable<T>;

	/// @brief Concept that requires that `T` is __NOT__ copy constructible or __NOT__ copy
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotCopyable = !Copyable<T>;

	/// @brief Concept that requires that `T` is noexcept copy constructible and noexcept copy
	/// assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptCopyable = NoexceptCopyConstructible<T> && NoexceptCopyAssignable<T>;

	/// @brief Concept that requires that `T` is __NOT__ noexcept copy constructible or __NOT__
	/// noexcept copy assignable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotNoexceptCopyable = !NoexceptCopyable<T>;

	/// @brief Concept that requires that `T` is default constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept DefaultConstructible = std::is_default_constructible_v<T>;

	/// @brief Concept that requires that `T` is trivially default constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

	/// @brief Concept that requires that `T` is nothrow default constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NoexceptDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is default
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllDefaultConstructible
		= mpl::for_all_types_v<std::is_default_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is trivially default
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllTriviallyDefaultConstructible
		= mpl::for_all_types_v<std::is_trivially_default_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is nothrow default
	/// constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename List>
	concept AllNoexceptDefaultConstructible
		= mpl::for_all_types_v<std::is_nothrow_default_constructible, std::true_type, List>;

	/// @brief Concept that requires `T` is __NOT__ default constructible
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotDefaultConstructible = !DefaultConstructible<T>;

	/// @brief Concept requiring `T` is copyable or movable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept CopyOrMovable = Copyable<T> || Movable<T>;

	/// @brief Concept requiring `T` is **NOT** copyable nor movable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotCopyOrMovable = !CopyOrMovable<T>;

	/// @brief Concept that requires `T` is swappable with `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept Swappable = std::is_swappable_with_v<T, U>;

	/// @brief Concept that requires `T` is __NOT__ swappable with `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept NotSwappable = !Swappable<T, U>;

	/// @brief Concept that requires `T` is noexcept swappable with `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept NoexceptSwappable = std::is_nothrow_swappable_with_v<T, U>;

	/// @brief Concept that requires `T` is __NOT__ noexcept swappable with `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept NotNoexceptSwappable = !NoexceptSwappable<T, U>;

	/// @brief Concept that requires `T` is constructible from `Args` in a constexpr context
	/// EG: requires that `constexpr T(Args... args);`
	///
	/// Due to implementation restrictions, this requires that each of `Args` are
	/// default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename... Args>
	concept ConstexprConstructibleFrom
		= concepts::ConstructibleFrom<T, Args...> && std::bool_constant
		  < type_traits::is_constexpr_constructible<T, Args...>(type_traits::declval<Args>()...)
	> ::value;

	/// @brief Concept that requires `T` is default constructible in a constexpr context
	/// EG: requires that `constexpr T();`
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept ConstexprDefaultConstructible = ConstexprConstructibleFrom<T>;

	/// @brief Concept that requires `T` is copy constructible in a constexpr context
	/// EG: requires that `constexpr T(const T&);`
	///
	/// Due to implementation restrictions, this requires that `T` is default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept ConstexprCopyConstructible = ConstexprConstructibleFrom<T, const T&>;

	/// @brief Concept that requires `T` is move constructible in a constexpr context
	/// EG: requires that `constexpr T(T&&);`
	///
	/// Due to implementation restrictions, this requires that `T` is default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept ConstexprMoveConstructible = ConstexprConstructibleFrom<T, T&&>;

	/// @brief Concept that requires `T` is assignable in a constexpr context from `U`
	/// EG: requires that `constexpr auto operator=(const U&) -> T&;`
	///
	/// Due to implementation restrictions, this requires that `U` is default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U>
	concept ConstexprAssignable
		= concepts::Assignable<T, U> && std::bool_constant
		  < type_traits::is_constexpr_assignable<T>(type_traits::declval<U>())
	> ::value;

	/// @brief Concept that requires `T` is copy assignable in a constexpr context
	/// EG: requires that `constexpr auto operator=(const T&) -> T&;`
	///
	/// Due to implementation restrictions, this requires that `T` is default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept ConstexprCopyAssignable = ConstexprAssignable<T, const T&>;

	/// @brief Concept that requires `T` is move assignable in a constexpr context
	/// EG: requires that `constexpr auto operator=(T&&) -> T&;`
	///
	/// Due to implementation restrictions, this requires that `T` is default-constructible
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept ConstexprMoveAssignable = ConstexprAssignable<T, T&&>;

	/// @brief Alias for `std::semiregular<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept SemiRegular = std::semiregular<T>;

	/// @brief Alias for `! std::semiregular<T>`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept NotSemiRegular = !SemiRegular<T>;

	/// @brief Concept that requires `Func` is invocable with arguments of types `Args`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename Func, typename... Args>
	concept Invocable = std::is_invocable_v<Func, Args...>;

	/// @brief Concept that requires `Func` is invocable with arguments of types `Args`, with the
	/// return type `Return`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename Return, typename Func, typename... Args>
	concept InvocableWithReturn = std::is_invocable_r_v<Return, Func, Args...>;

	/// @brief Concept that requires `T` is allocatable by the allocator `Allocator`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T,
			 typename Allocator = std::allocator<T>,
			 typename U = std::remove_cv_t<std::remove_all_extents_t<T>>>
	concept Allocatable = requires(Allocator alloc) {
		// clang-format off
		concepts::Same<std::decay_t<U>,typename std::allocator_traits<Allocator>::value_type> ||
			concepts::Derived<typename std::allocator_traits<Allocator>::value_type, U>;
		// clang-format on
		std::allocator_traits<Allocator>::allocate(alloc, 1_usize);
	};

	/// @brief Concept that requires `T` is deletable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = std::remove_all_extents_t<T>>
	concept Deletable = sizeof(U) > 0 && !std::is_void_v<U> // NOLINT
						&& (std::is_array_v<T> ?
								requires(U * arr) {
									delete[] arr; // NOLINT
								} :
								requires(U * ptr) {
									delete ptr; // NOLINT
								});

	/// @brief Concept that requires `T` is noexcept deletable
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = std::remove_all_extents_t<T>>
	concept NoexceptDeletable = sizeof(U) > 0 && !std::is_void_v<U> // NOLINT
								&& (std::is_array_v<T> ?
										requires(U * arr) {
											noexcept(delete[] arr); // NOLINT
										} :
										requires(U * ptr) {
											noexcept(delete ptr); // NOLINT
										});

	/// @brief Concept that requires `T` is equality comparable with `U`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T, typename U = T>
	concept EqualityComparable = requires(T lhs, U rhs) {
		lhs == rhs;
	};

	/// @brief Concept that requires that the single-extent removed `T` is equality comparable
	///
	/// `T` is tested as `std::remove_pointer_t<T>` in the case `T` is a pointer type
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept DerefEqualityComparable
		= (Pointer<T> ? EqualityComparable<T> : EqualityComparable<std::remove_pointer_t<T>>);

	/// @brief Concept that requires that the single-extent removed `T` is inequality comparable
	///
	/// `T` is tested as `std::remove_pointer_t<T>` in the case `T` is a pointer type
	///
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept DerefInequalityComparable
		= (Pointer<T> ? InequalityComparable<T> : InequalityComparable<std::remove_pointer_t<T>>);

	/// @brief Concept that requires `T` is implicitly convertible to `std::string` or
	/// `std::string_view`
	/// @ingroup concepts
	/// @headerfile "Hyperion/Concepts.h"
	template<typename T>
	concept Stringable = Convertible<T, std::string> || Convertible<T, std::string_view>;
} // namespace hyperion::concepts
