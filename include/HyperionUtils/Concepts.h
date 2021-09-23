/// @brief This is a small collection of simple Concepts useful for requiring basic attributes of a
/// type
#pragma once

#include <cmath>
#include <concepts>
#include <type_traits>

#include "TypeTraits.h"
#include "mpl/ForAll.h"
#include "mpl/List.h"

namespace hyperion::concepts {

	/// @brief Alias for `std::floating_point<T>`
	template<typename T>
	concept FloatingPoint = std::floating_point<T>;

	/// @brief Alias for `std::integral<T>`
	template<typename T>
	concept Integral = std::integral<T>;

	template<typename T>
	concept UnsignedIntegral = std::unsigned_integral<T>;

	/// @brief Alias for `std::signed_integral<T>`
	template<typename T>
	concept SignedIntegral = std::signed_integral<T>;

	/// @brief Concept that requires the type to be a numeric type
	/// ie: FloatingPoint<T> or Integral<T> is true
	template<typename T>
	concept Numeric = FloatingPoint<T> || Integral<T>;

	/// @brief Concept that requires the type to be a numeric type
	/// ie: FloatingPoint<T> or Integral<T> is true
	template<typename T>
	concept SignedNumeric = FloatingPoint<T> || SignedIntegral<T>;

	/// @brief Concept for `std::is_pointer_v<T>`
	template<typename T>
	concept Pointer = std::is_pointer_v<T>;

	/// @brief Concept for `! std::is_pointer_v<T>`
	template<typename T>
	concept NotPointer = !std::is_pointer_v<T>;

	/// @brief Concept for `std::is_reference_v<T>`
	template<typename T>
	concept Reference = std::is_reference_v<T>;

	/// @brief Concept for `! std::is_reference_v<T>`
	template<typename T>
	concept NotReference = !Reference<T>;

	template<typename T>
	concept RValueReference = std::is_rvalue_reference_v<T>;

	template<typename T>
	concept NotRValueReference = !RValueReference<T>;

	template<typename T>
	concept LValueReference = std::is_lvalue_reference_v<T>;

	template<typename T>
	concept NotLValueReference = !LValueReference<T>;

	/// @brief Concept for `std::is_function_v<T>`
	template<typename T>
	concept Function = std::is_function_v<T>;

	/// @brief Concept for `!std::is_function_v<T>`
	template<typename T>
	concept NotFunction = !Function<T>;

	/// @brief Alias for `std::derived_from<T, U>`
	template<typename Der, typename Base>
	concept Derived = std::derived_from<std::remove_pointer_t<std::decay_t<Der>>,
										std::remove_pointer_t<std::decay_t<Base>>>;

	/// @brief Concept that requires `From` is convertible to `To`
	template<typename From, typename To>
	concept Convertible = std::is_convertible_v<From, To>;

	/// @brief Concept that requires `From` is __not__ convertible to `To`
	template<typename From, typename To>
	concept NotConvertible = !Convertible<From, To>;

	/// @brief Concept that requires `T` to be constructible from the parameter pack `Args`
	template<typename T, typename... Args>
	concept ConstructibleFrom = std::is_constructible_v<T, Args...>;

	/// @brief Concept that requires `T` to be noexcept constructible from the parameter pack `Args`
	template<typename T, typename... Args>
	concept NoexceptConstructibleFrom = std::is_nothrow_constructible_v<T, Args...>;

	/// @brief Concept that requires `T` and `U` to be the same type
	template<typename T, typename U>
	concept Same = std::same_as<T, U>;

	/// @brief Concept that requires `T` and `U` to be the same type
	template<typename T, typename U>
	concept NotSame = !Same<T, U>;

	/// @brief Concept requiring `T` to be inequality comparable to `U`
	/// (`T` has `operator!=` for `U`)
	template<typename T, typename U = T>
	concept InequalityComparable = type_traits::has_not_equal_v<T, U>;

	/// @brief  Concept requiring that the `mpl::list`, `List`, contains the type `T`
	template<typename T, typename List>
	concept Contains = mpl::contains_v<T, List>;

	/// @brief Concept that requires that `T` is copy constructible
	template<typename T>
	concept CopyConstructible = std::is_copy_constructible_v<T>;

	/// @brief Concept that requires that `T` is trivially copy constructible
	template<typename T>
	concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// constructible
	template<typename List>
	concept AllCopyConstructible
		= mpl::for_all_types_v<std::is_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// constructible
	template<typename List>
	concept AllTriviallyCopyConstructible
		= mpl::for_all_types_v<std::is_trivially_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is copy assignable
	template<typename T>
	concept CopyAssignable = std::is_copy_assignable_v<T>;

	/// @brief Concept that requires that `T` is copy assignable
	template<typename T>
	concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// assignable
	template<typename List>
	concept AllCopyAssignable = mpl::for_all_types_v<std::is_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is copy
	/// assignable
	template<typename List>
	concept AllTriviallyCopyAssignable
		= mpl::for_all_types_v<std::is_trivially_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept copy constructible
	template<typename T>
	concept NoexceptCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// constructible
	template<typename List>
	concept AllNoexceptCopyConstructible
		= mpl::for_all_types_v<std::is_nothrow_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept copy assignable
	template<typename T>
	concept NoexceptCopyAssignable = std::is_nothrow_copy_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// assignable
	template<typename List>
	concept AllNoexceptCopyAssignable
		= mpl::for_all_types_v<std::is_nothrow_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is move constructible
	template<typename T>
	concept MoveConstructible = std::is_move_constructible_v<T>;

	/// @brief Concept that requires that `T` is __not__ move constructible
	template<typename T>
	concept NotMoveConstructible = !std::is_move_constructible_v<T>;

	/// @brief Concept that requires that `T` is move constructible
	template<typename T>
	concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// constructible
	template<typename List>
	concept AllMoveConstructible
		= mpl::for_all_types_v<std::is_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// constructible
	template<typename List>
	concept AllTriviallyMoveConstructible
		= mpl::for_all_types_v<std::is_trivially_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is move assignable
	template<typename T>
	concept MoveAssignable = std::is_move_assignable_v<T>;

	/// @brief Concept that requires that `T` is __not__ move assignable
	template<typename T>
	concept NotMoveAssignable = !std::is_move_assignable_v<T>;

	/// @brief Concept that requires that `T` is move assignable
	template<typename T>
	concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// assignable
	template<typename List>
	concept AllMoveAssignable = mpl::for_all_types_v<std::is_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is move
	/// assignable
	template<typename List>
	concept AllTriviallyMoveAssignable
		= mpl::for_all_types_v<std::is_trivially_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is assignable from `U`
	template<typename T, typename U>
	concept Assignable = std::is_assignable_v<T, U>;

	/// @brief Concept that requires that `T` is noexcept assignable from `U`
	template<typename T, typename U>
	concept NoexceptAssignable = std::is_nothrow_assignable_v<T, U>;

	/// @brief Concept that requires that `T` is noexcept move constructible
	template<typename T>
	concept NoexceptMoveConstructible = std::is_nothrow_move_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// constructible
	template<typename List>
	concept AllNoexceptMoveConstructible
		= mpl::for_all_types_v<std::is_nothrow_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that `T` is noexcept move assignable
	template<typename T>
	concept NoexceptMoveAssignable = std::is_nothrow_move_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// assignable
	template<typename List>
	concept AllNoexceptMoveAssignable
		= mpl::for_all_types_v<std::is_nothrow_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that `T` is destructible
	template<typename T>
	concept Destructible = std::is_destructible_v<T>;

	/// @brief Concept that requires that `T` is noexcept destructible
	template<typename T>
	concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

	/// @brief Concept that requires that `T` is noexcept destructible
	template<typename T>
	concept NoexceptDestructible = std::is_nothrow_destructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow
	/// destructible
	template<typename List>
	concept AllTriviallyDestructible
		= mpl::for_all_types_v<std::is_trivially_destructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow
	/// destructible
	template<typename List>
	concept AllNoexceptDestructible
		= mpl::for_all_types_v<std::is_nothrow_destructible, std::true_type, List>;

	/// @brief Alias for `std::movable<T>`
	template<typename T>
	concept Movable = MoveConstructible<T> && MoveAssignable<T>;

	/// @brief Alias for `! std::movable<T>`
	template<typename T>
	concept NotMovable = !Movable<T>;

	/// @brief Alias for `std::movable<T>`
	template<typename T>
	concept NoexceptMovable = NoexceptMoveConstructible<T> && NoexceptMoveAssignable<T>;

	/// @brief Alias for `! std::movable<T>`
	template<typename T>
	concept NoexceptNotMovable = !NoexceptMovable<T>;

	/// @brief Alias for `std::copyable<T>`
	template<typename T>
	concept Copyable = CopyConstructible<T> && CopyAssignable<T>;

	/// @brief Alias for `! std::copyable<T>`
	template<typename T>
	concept NotCopyable = !Copyable<T>;

	template<typename T>
	concept NoexceptCopyable = NoexceptCopyConstructible<T> && NoexceptCopyAssignable<T>;

	template<typename T>
	concept NotNoexceptCopyable = !NoexceptCopyable<T>;

	/// @brief Concept that requires that `T` is default constructible
	template<typename T>
	concept DefaultConstructible = std::is_default_constructible_v<T>;

	/// @brief Concept that requires that `T` is trivally default constructible
	template<typename T>
	concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

	/// @brief Concept that requires that `T` is nothrow default constructible
	template<typename T>
	concept NoexceptDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is default
	/// constructible
	template<typename List>
	concept AllDefaultConstructible
		= mpl::for_all_types_v<std::is_default_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is trivially default
	/// constructible
	template<typename List>
	concept AllTriviallyDefaultConstructible
		= mpl::for_all_types_v<std::is_trivially_default_constructible, std::true_type, List>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List` is nothrow default
	/// constructible
	template<typename List>
	concept AllNoexceptDefaultConstructible
		= mpl::for_all_types_v<std::is_nothrow_default_constructible, std::true_type, List>;

	/// @brief Alias for `! std::default_initializable<T>`
	template<typename T>
	concept NotDefaultConstructible = !DefaultConstructible<T>;

	/// @brief Concept requiring `T` is copyable or movable
	template<typename T>
	concept CopyOrMovable = Copyable<T> || Movable<T>;

	/// @brief Concept requiring `T` is **NOT** copyable nor movable
	template<typename T>
	concept NotCopyOrMovable = !CopyOrMovable<T>;

	template<typename T, typename U = T>
	concept Swappable = std::is_swappable_with_v<T, U>;

	template<typename T, typename U = T>
	concept NotSwappable = !Swappable<T, U>;

	template<typename T, typename U = T>
	concept NoexceptSwappable = std::is_nothrow_swappable_with_v<T, U>;

	template<typename T, typename U = T>
	concept NotNoexceptSwappable = !NoexceptSwappable<T, U>;

	/// @brief Concept that requires `T` is constructible from `Args` in a constexpr context
	/// EG: requires that `constexpr T(Args... args);`
	template<typename T, typename... Args>
	concept ConstexprConstructibleFrom
		= concepts::ConstructibleFrom<T, Args...> && std::bool_constant
		  < type_traits::is_constexpr_constructible<T, Args...>(type_traits::declval<Args>()...)
	> ::value;

	/// @brief Concept that requires `T` is default constructible in a constexpr context
	/// EG: requires that `constexpr T();`
	template<typename T>
	concept ConstexprDefaultConstructible = ConstexprConstructibleFrom<T>;

	/// @brief Concept that requires `T` is copy constructible in a constexpr context
	/// EG: requires that `constexpr T(const T&);`
	template<typename T>
	concept ConstexprCopyConstructible = ConstexprConstructibleFrom<T, const T&>;

	/// @brief Concept that requires `T` is move constructible in a constexpr context
	/// EG: requires that `constexpr T(T&&);`
	template<typename T>
	concept ConstexprMoveConstructible = ConstexprConstructibleFrom<T, T&&>;

	template<typename T, typename U>
	concept ConstexprAssignable
		= concepts::Assignable<T, U> && std::bool_constant
		  < type_traits::is_constexpr_assignable<T>(type_traits::declval<U>())
	> ::value;

	/// @brief Concept that requires `T` is copy assignable in a constexpr context
	/// EG: requires that `constexpr auto operator=(const T&) -> T&;`
	template<typename T>
	concept ConstexprCopyAssignable = ConstexprAssignable<T, const T&>;

	/// @brief Concept that requires `T` is move assignable in a constexpr context
	/// EG: requires that `constexpr auto operator=(T&&) -> T&;`
	template<typename T>
	concept ConstexprMoveAssignable = ConstexprAssignable<T, T&&>;

	/// @brief Alias for `std::semiregular<T>`
	template<typename T>
	concept SemiRegular = std::semiregular<T>;

	/// @brief Alias for `! std::semiregular<T>`
	template<typename T>
	concept NotSemiRegular = !SemiRegular<T>;

	/// @brief Alias for `std::is_invocable_v<T, Args...>`
	template<typename T, typename... Args>
	concept Invocable = std::is_invocable_v<T, Args...>;

	/// @brief Alias for `std::is_invocable_r_v<Return, T, Args...>`
	template<typename Return, typename T, typename... Args>
	concept InvocableR = std::is_invocable_r_v<Return, T, Args...>;

	/// @brief Concept that requires that `Func` is invocable for `Arg` or `const Arg&` with the
	/// given return type `Return`
	template<typename Return, typename Func, typename Arg>
	concept InvocableRConst = InvocableR<Return, Func, Arg> || InvocableR<Return, Func, const Arg&>;

	/// @brief Concept that requires that `Func` is invocable for `Arg`, `Arg&`, `const Arg&`, or
	/// `Arg&&` with the given return type `Return`
	template<typename Return, typename Func, typename Arg>
	concept InvocableRMut
		= InvocableR<Return, Func, Arg> || InvocableR<Return, Func, Arg&> || InvocableR<Return,
																						Func,
																						Arg&&>;

	/// @brief Concept that determines if `T` is allocatable by `Allocator`
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

	template<typename T, typename U = std::remove_all_extents_t<T>>
	concept Deletable = sizeof(U) > 0 && !std::is_void_v<U> // NOLINT
						&& (std::is_array_v<T> ?
								requires(U * t) {
									delete[] t; // NOLINT
								} :
								  requires(U * t) {
									delete t; // NOLINT
								});

	template<typename T, typename U = std::remove_all_extents_t<T>>
	concept NoexceptDeletable = sizeof(U) > 0 && !std::is_void_v<U> // NOLINT
								&& (std::is_array_v<T> ?
										requires(U * t) {
											noexcept(delete[] t); // NOLINT
										} :
										  requires(U * t) {
											noexcept(delete t); // NOLINT
										});

	template<typename T, typename U = T>
	concept EqualityComparable = requires(T t, U u) {
		t == u;
	};

	template<typename T>
	concept DerefEqualityComparable
		= (Pointer<T> ? EqualityComparable<T> : EqualityComparable<std::remove_pointer_t<T>>);

	template<typename T>
	concept DerefInequalityComparable
		= (Pointer<T> ? InequalityComparable<T> : InequalityComparable<std::remove_pointer_t<T>>);
} // namespace hyperion::concepts
