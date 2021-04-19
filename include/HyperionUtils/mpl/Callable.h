/// @brief Set of metaprogramming functions to determine if a (normal) function can be called on the
/// with a parameter of the given type
#pragma once

#include "../BasicTypes.h"
#include "ForAll.h"

namespace hyperion::mpl {

	namespace detail {
		template<typename T, typename Param, typename = std::void_t<>>
		struct is_callable_with_impl : std::false_type { };

		template<typename T, typename Param>
		struct is_callable_with_impl<
			T,
			Param,
			std::void_t<decltype(std::declval<T>()(std::declval<Param>()))>> : std::true_type { };

		struct dummy_functor {
			inline auto operator()(u8 val) noexcept -> u8 {
				return val;
			}
		};

		static_assert(is_callable_with_impl<dummy_functor, u8>::value,
					  "mpl::is_callable_with implementation failing");
		static_assert(!is_callable_with_impl<dummy_functor, std::string>::value,
					  "mpl::is_callable_with implementation failing");
		static_assert(!is_callable_with_impl<dummy_functor, u8*>::value,
					  "mpl::is_callable_with implementation failing");
	} // namespace detail

	/// @brief Meta-programming function to determine if `T` is a function or function object
	/// callable with a one parameter of the type `Param`
	///
	/// @tparam T - The function/function object
	/// @tparam Param - The type of the parameter to check for callability for
	template<typename T, typename Param>
	struct is_callable_with : detail::is_callable_with_impl<T, Param> { };

	/// @brief Value of Meta-programming function `is_callable`. Used to determine if `T` is a
	/// function or function object callable with one parameter of the type `Param`
	template<typename T, typename Param>
	inline static constexpr bool is_callable_with_v = is_callable_with<T, Param>::value;

	/// @brief Meta-programming function to determine if `T` is a function or function object
	/// callable with one parameter taking the form of each of the types in the `mpl::list`, `List`
	///
	/// @tparam T - The function/function object
	/// @tparam List - The `mpl::list` of types of parameters to check for callability for
	template<typename T, typename List>
	struct is_callable_with_all : for_all_params<is_callable_with, std::true_type, T, List> { };

	/// @brief Value of Meta-programming function `is_callable_with_all`. Used to determine if `T`
	/// is a function or function object callable with one parameter taking the form of each of the
	/// types in the `mpl::list`, `List`
	template<typename T, typename List>
	inline static constexpr bool is_callable_with_all_v = is_callable_with_all<T, List>::value;
} // namespace hyperion::mpl
