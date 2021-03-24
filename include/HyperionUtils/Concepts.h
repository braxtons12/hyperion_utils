#pragma once

#include <cmath>
#include <concepts>
#include <type_traits>

#include "Error.h"
#include "TypeTraits.h"

namespace hyperion::utils::concepts {

	/// @brief Alias for `std::floating_point<T>`
	template<typename T>
	concept FloatingPoint = std::floating_point<T>;

	/// @brief Alias for `std::integral<T>`
	template<typename T>
	concept Integral = std::integral<T>;

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

	/// @brief Alias for `std::movable<T>`
	template<typename T>
	concept Movable = std::movable<T>;

	/// @brief Alias for `! std::movable<T>`
	template<typename T>
	concept NotMovable = !Movable<T>;

	/// @brief Alias for `std::copyable<T>`
	template<typename T>
	concept Copyable = std::copyable<T> || Numeric<T>;

	/// @brief Alias for `! std::copyable<T>`
	template<typename T>
	concept NotCopyable = !Copyable<T>;

	/// @brief Alias for `std::default_initializable<T>`
	template<typename T>
	concept DefaultConstructible = std::default_initializable<T>;

	/// @brief Alias for `! std::default_initializable<T>`
	template<typename T>
	concept NotDefaultConstructible = !DefaultConstructible<T>;

	/// @brief Concept requiring T is copyable or movable
	template<typename T>
	concept CopyOrMovable = Copyable<T> || Movable<T>;

	/// @brief Concept requiring T is **NOT** copyable nor movable
	template<typename T>
	concept NotCopyOrMovable = !CopyOrMovable<T>;

	/// @brief Alias for `std::semiregular<T>`
	template<typename T>
	concept SemiRegular = std::semiregular<T>;

	/// @brief Alias for `! std::semiregular<T>`
	template<typename T>
	concept NotSemiRegular = !SemiRegular<T>;

	/// @brief Alias for `std::derived_from<T, U>`
	template<typename Der, typename Base>
	concept Derived = std::derived_from<std::remove_pointer_t<std::decay_t<Der>>,
										std::remove_pointer_t<std::decay_t<Base>>>;

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

	/// @brief Concept that requires `T` and `U` to be the same type
	template<typename T, typename U>
	concept Same = std::is_same_v<T, U>;

	/// @brief Concept requiring `T` to be inequality comparable to `U`
	/// (`T` has `operator!=` for `U`)
	template<typename T, typename U = T>
	concept InequalityComparable = type_traits::has_not_equal_v<T, U>;
} // namespace hyperion::utils::concepts
