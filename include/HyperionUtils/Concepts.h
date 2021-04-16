/// @brief This is a small collection of simple Concepts useful for requiring basic attributes of a
/// type
#pragma once

#include <cmath>
#include <concepts>
#include <type_traits>

#include "Error.h"
#include "TypeTraits.h"
#include "mpl/ForAll.h"
#include "mpl/List.h"

namespace hyperion::utils::concepts {

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
	concept NotPointer = !Pointer<T>;

	/// @brief Concept for `std::is_reference_v<T>`
	template<typename T>
	concept Reference = std::is_reference_v<T>;

	/// @brief Concept for `! std::is_reference_v<T>`
	template<typename T>
	concept NotReference = !Reference<T>;

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

	/// @brief Concept that requires `E` to be an `ErrorType`
	/// (aka derived from `hyperion::utils::Error`)
	template<typename E>
	concept ErrorType = Derived<E, Error>;

	/// @brief Concept that requires `E` is not an `ErrorType`
	/// (aka __NOT__ derived from `hyperion::utils::Error`)
	template<typename E>
	concept NotErrorType = !ErrorType<E>;

	/// @brief Concept that requires `T` to be constructible from the parameter pack `Args`
	template<typename T, typename... Args>
	concept ConstructibleFrom = requires(Args&&... args) {
		T{args...};
	};

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

	/// @brief Concept that requires that T is copy constructible
	template<typename T>
	concept CopyConstructible = std::is_copy_constructible_v<T>;

	/// @brief Concept that requires that T is trivially copy constructible
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

	/// @brief Concept that requires that T is copy assignable
	template<typename T>
	concept CopyAssignable = std::is_copy_assignable_v<T>;

	/// @brief Concept that requires that T is copy assignable
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

	/// @brief Concept that requires that T is noexcept copy constructible
	template<typename T>
	concept NoexceptCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// constructible
	template<typename List>
	concept AllNoexceptCopyConstructible
		= mpl::for_all_types_v<std::is_nothrow_copy_constructible, std::true_type, List>;

	/// @brief Concept that requires that T is noexcept copy assignable
	template<typename T>
	concept NoexceptCopyAssignable = std::is_nothrow_copy_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow copy
	/// assignable
	template<typename List>
	concept AllNoexceptCopyAssignable
		= mpl::for_all_types_v<std::is_nothrow_copy_assignable, std::true_type, List>;

	/// @brief Concept that requires that T is move constructible
	template<typename T>
	concept MoveConstructible = std::is_move_constructible_v<T>;

	/// @brief Concept that requires that T is move constructible
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

	/// @brief Concept that requires that T is move assignable
	template<typename T>
	concept MoveAssignable = std::is_move_assignable_v<T>;

	/// @brief Concept that requires that T is move assignable
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

	/// @brief Concept that requires that T is noexcept move constructible
	template<typename T>
	concept NoexceptMoveConstructible = std::is_nothrow_move_constructible_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// constructible
	template<typename List>
	concept AllNoexceptMoveConstructible
		= mpl::for_all_types_v<std::is_nothrow_move_constructible, std::true_type, List>;

	/// @brief Concept that requires that T is noexcept move assignable
	template<typename T>
	concept NoexceptMoveAssignable = std::is_nothrow_move_assignable_v<T>;

	/// @brief Concept that requires that every type in the `mpl::list`, `List`, is nothrow move
	/// assignable
	template<typename List>
	concept AllNoexceptMoveAssignable
		= mpl::for_all_types_v<std::is_nothrow_move_assignable, std::true_type, List>;

	/// @brief Concept that requires that T is noexcept destructible
	template<typename T>
	concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

	/// @brief Concept that requires that T is noexcept destructible
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

	/// @brief Alias for `std::copyable<T>`
	template<typename T>
	concept Copyable = CopyConstructible<T> && CopyAssignable<T>;

	/// @brief Alias for `! std::copyable<T>`
	template<typename T>
	concept NotCopyable = !Copyable<T>;

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

	/// @brief Concept requiring T is copyable or movable
	template<typename T>
	concept CopyOrMovable = Copyable<T> || Movable<T>;

	/// @brief Concept requiring T is **NOT** copyable nor movable
	template<typename T>
	concept NotCopyOrMovable = !CopyOrMovable<T>;

	/// @brief Concept that is the disjunction of most of the requirements for `std::semiregular`
	/// Requires that that type be at least one of:
	/// * copyable
	/// * movable
	/// * a reference
	/// * a pointer
	template<typename T>
	concept Passable = CopyOrMovable<T> || Reference<T> || Pointer<T>;

	/// @brief Concept that is the negation of `Passable<T>`
	/// Requires that the type is **NONE** of:
	/// * copyable
	/// * movable
	/// * a reference
	/// * a pointer
	template<typename T>
	concept NotPassable = !Passable<T>;

	/// @brief Alias for `std::semiregular<T>`
	template<typename T>
	concept SemiRegular = std::semiregular<T>;

	/// @brief Alias for `! std::semiregular<T>`
	template<typename T>
	concept NotSemiRegular = !SemiRegular<T>;

} // namespace hyperion::utils::concepts
