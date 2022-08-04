/// @file Index.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Meta-programming facilities for working with a list of types
/// @version 0.1
/// @date 2022-07-29
///
/// MIT License
/// @copyright Copyright (c) 2022 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
#pragma once

#include <Hyperion/BasicTypes.h>
#include <Hyperion/HyperionDef.h>

IGNORE_UNUSED_VALUES_START
#include <boost/hana.hpp>
IGNORE_UNUSED_VALUES_STOP

namespace hyperion::mpl {

	namespace hana = boost::hana;

	IGNORE_UNUSED_VALUES_START

	/// @brief Basic meta-programming type list
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename... T>
    struct list {
    };

	namespace detail {
		template<typename List>
		struct size_impl;

		template<template<typename...> typename List, typename... Types>
		struct size_impl<List<Types...>> : public std::integral_constant<usize, sizeof...(Types)> {
		};
	} // namespace detail

	/// @brief Used to determine the number of elements in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to query the size of
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	struct size : detail::size_impl<List> {
	};

	/// @brief Used to determine the number of elements in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to query the size of
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static inline constexpr auto size_v = size<List>::value;

	// clang-format off
    namespace detail {
        template<usize N, typename List>
        struct at_unrolled;

#if HYPERION_HAS_TYPE_PACK_ELEMENT
        template<usize N, template<typename...> typename List, typename... Types>
        struct at_unrolled<N, List<Types...>> {
            using type = __type_pack_element<N, Types...>;
        };
#else
        template<usize N, template<typename...> typename List, typename First>
        requires (N == 0_usize)
        struct at_unrolled<N, List<First>> {
            using type = First;
        };

        template<usize N, template<typename...> typename List, typename First, typename Second>
        requires (N <= 1_usize)
        struct at_unrolled<N, List<First, Second>> {
            using type = std::conditional_t<N == 0_usize, First, Second>;
        };

        template<usize N,
                 template<typename...> typename List,
                 typename First,
                 typename Second,
                 typename Third>
        requires (N <= 2_usize)
        struct at_unrolled<N, List<First, Second, Third>> {
            using type = std::conditional_t<N == 0_usize,
                                            First,
                                            std::conditional_t<N == 1_usize, Second, Third>>;
        };

        template<usize N,
                 template<typename...> typename List,
                 typename First,
                 typename Second,
                 typename Third,
                 typename Fourth>
        requires (N <= 3_usize)
        struct at_unrolled<N, List<First, Second, Third, Fourth>> {
            using type = std::conditional_t<N == 0_usize,
                                            First,
                                            std::conditional_t<N == 1_usize,
                                                               Second,
                                                               std::conditional_t<N == 2_usize,
                                                                                  Third,
                                                                                  Fourth>>>;
        };

        template<usize N,
                 template<typename...> typename List,
                 typename First,
                 typename Second,
                 typename Third,
                 typename Fourth,
                 typename Fifth>
        requires (N <= 4_usize)
        struct at_unrolled<N, List<First, Second, Third, Fourth, Fifth>> {
            using type = std::conditional_t<N == 0_usize,
                                            First,
                                            std::conditional_t<N == 1_usize,
                                                               Second,
                                                               std::conditional_t<N == 2_usize,
                                                                                  Third,
                                                                                  std::conditional_t<N == 3_usize,
                                                                                                     Fourth,
                                                                                                     Fifth>>>>;
        };

#endif // HYPERION_HAS_TYPE_PACK_ELEMENT
       
        template<usize N, usize Current, typename List>
        struct at_impl;

        template<usize N, usize Current, template<typename...> typename List, typename Head, typename... Types>
        struct at_impl<N, Current, List<Head, Types...>> {
#if HYPERION_HAS_TYPE_PACK_ELEMENT
            using type = __type_pack_element<N, Head, Types...>;
#else
            using type = std::conditional_t<N == Current,
                                            Head,
                                            typename at_impl<N, Current + 1_usize, List<Types...>>::type>;
#endif //HYPERION_HAS_TYPE_PACK_ELEMENT
        };

        template<usize N, usize Current, template<typename...> typename List>
        struct at_impl<N, Current, List<>> {
            using type = void;
        };
    } // namespace detail

	/// @brief Used to find the `N`th type in the `mpl::list`, `List`
	///
	/// @tparam N - The index into the list
	/// @tparam List - The `mpl::list` to get a type from
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<usize N, typename List>
    requires (N < size_v<List>)
	struct at {
        private:
            static inline constexpr auto size = size_v<List>;
        public:
        using type = std::conditional_t<size <= 5_usize,
                                        typename detail::at_unrolled<N, List>::type,
                                        typename detail::at_impl<N, 0_usize, List>::type>;
    };
	// clang-format on

	/// @brief Used to find the `N`th type in the `mpl::list`, `List`
	///
	/// @tparam N - The index into the list
	/// @tparam List - The `mpl::list` to get a type from
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<usize N, typename List>
	using at_t = typename at<N, List>::type;

	static_assert(std::is_same_v<u32, at_t<2, list<u8, u16, u32, u64>>>,
				  "mpl::at implementation failing");
	static_assert(std::is_same_v<u64, at_t<3, list<u8, u16, u32, u64>>>,
				  "mpl::at implementation failing");

	/// @brief Used to get the first type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	struct first : at<0, List> { };

	/// @brief Alias to `mpl::first::type`. Used to get the first type in the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	using first_t = typename first<List>::type;

	static_assert(std::is_same_v<first_t<list<u8, u16, u32>>, u8>,
				  "mpl::first implementation failing");
	static_assert(std::is_same_v<first_t<list<u32, u16, u8>>, u32>,
				  "mpl::first implementation failing");

	/// @brief Used to get the last type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	struct last : at<mpl::size_v<List> >= 1_usize ? mpl::size_v<List> - 1_usize : 0_usize, List> {
	};

	/// @brief Alias to `mpl::last::type`. Used to get the last type in the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	using last_t = typename last<List>::type;

	static_assert(std::is_same_v<last_t<list<u8, u16, u32>>, u32>,
				  "mpl::last implementation failing");
	static_assert(std::is_same_v<last_t<list<u32, u16, u8>>, u8>,
				  "mpl::last implementation failing");

	namespace detail {
		template<typename T, typename List>
		struct contains_unrolled;
		;

		// clang-format off

		template<typename T, typename List>
		requires(mpl::size_v<List> == 1_usize)
        struct contains_unrolled<T, List>
			: std::conditional_t <std::is_same_v<T, mpl::first_t<List>>, std::true_type, std::false_type> {
			// clang-format on
		};

		// clang-format off

		template<typename T, typename List>
		requires(mpl::size_v<List> == 2_usize)
        struct contains_unrolled<T, List>
            : std::conditional_t<
				  std::is_same_v<T, mpl::at_t<0_usize, List>>
                  || std::is_same_v<T, mpl::at_t<1_usize, List>>,
				  std::true_type,
				  std::false_type> {
			// clang-format on
		};

		// clang-format off

		template<typename T, typename List>
		requires(mpl::size_v<List> == 3_usize)
        struct contains_unrolled<T, List>
            : std::conditional_t<
				  std::is_same_v<T, mpl::at_t<0_usize, List>>
                  || std::is_same_v<T, mpl::at_t<1_usize, List>>
				  || std::is_same_v<T, mpl::at_t<2_usize, List>>,
				  std::true_type,
				  std::false_type> {
			// clang-format on
		};

		// clang-format off

		template<typename T, typename List>
		requires(mpl::size_v<List> == 4_usize)
        struct contains_unrolled<T, List>
            : std::conditional_t<
				  std::is_same_v<T, mpl::at_t<0_usize, List>>
                  || std::is_same_v<T, mpl::at_t<1_usize, List>>
				  || std::is_same_v<T, mpl::at_t<2_usize, List>>
                  || std::is_same_v<T, mpl::at_t<3_usize, List>>,
				  std::true_type,
				  std::false_type> {
			// clang-format on
		};

		// clang-format off

		template<typename T, typename List>
		requires(mpl::size_v<List> == 5_usize)
        struct contains_unrolled<T, List>
            : std::conditional_t<
				  std::is_same_v<T, mpl::at_t<0_usize, List>>
                  || std::is_same_v<T, mpl::at_t<1_usize, List>>
				  || std::is_same_v<T, mpl::at_t<2_usize, List>>
                  || std::is_same_v<T, mpl::at_t<3_usize, List>>
                  || std::is_same_v<T, mpl::at_t<4_usize, List>>,
				  std::true_type,
				  std::false_type> {
			// clang-format on
		};

		// clang-format off

		template<typename T, typename... Types>
		requires(mpl::size_v<mpl::list<Types...>> >= 6_usize)
        struct contains_impl : public std::true_type {
			// clang-format on
		};

		template<typename T, typename Head, typename... Types>
		struct contains_impl<T, Head, Types...> : std::conditional_t<std::is_same_v<T, Head>,
																	 std::true_type,
																	 contains_impl<T, Types...>> {
		};

		template<typename T>
		struct contains_impl<T> : std::false_type { };

		template<typename T, typename List>
		struct contains_list;

		// clang-format off

		template<typename T, template<typename...> typename List, typename... Types>
        requires (mpl::size_v<List<Types...>> <= 5_usize)
		struct contains_list<T, List<Types...>> : contains_unrolled<T, List<Types...>> {
        };

		template<typename T, template<typename...> typename List, typename... Types>
        requires (mpl::size_v<List<Types...>> >= 6_usize)
		struct contains_list<T, List<Types...>> : contains_impl<T, Types...> {
        };

		// clang-format on

		static_assert(contains_list<u8, mpl::list<u8, u16, u8, u64>>::value,
					  "mpl::contains implementation failing");
		static_assert(!contains_list<u32, mpl::list<u16, u8, u64>>::value,
					  "mpl::contains implementation failing");

	} // namespace detail

	/// @brief Used to determine if the given `mpl::list`, `List`, contains the type `T`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	struct contains : detail::contains_list<T, List> { };

	/// @brief Value of `mpl::contains`. Used to determine if the given `mpl::list`, `List`,
	/// contains the given type `T`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr auto contains_v = contains<T, List>::value;

	namespace detail {
		template<usize N, typename T, typename... Types>
		struct index_of_impl : std::integral_constant<usize, N> { };

		template<usize N, typename T, typename Head, typename... Types>
		struct index_of_impl<N, T, Head, Types...>
			: std::conditional_t<std::is_same_v<Head, T>,
								 std::integral_constant<usize, N>,
								 index_of_impl<N + 1, T, Types...>> { };

		template<usize N, typename T>
		struct index_of_impl<N, T> : std::integral_constant<usize, N> { };

		static_assert(2 == index_of_impl<0, u8, u16, u32, u8>::value,
					  "mpl::index_of implementation failing");
		static_assert(0 == index_of_impl<0, u16, u16, u32, u8>::value,
					  "mpl::index_of implementation failing");

		template<typename T, typename List>
		struct index_of_list_impl;

		// clang-format off

		template<typename T, template<typename...> typename List, typename... Types>
		struct index_of_list_impl<T, List<Types...>> : index_of_impl<0, T, Types...> { };

		// clang-format on
	} // namespace detail

	// clang-format off

	/// @brief Used to get the index of the type `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	requires(contains_v<T, List>)
	struct index_of : detail::index_of_list_impl<T, List> {
	};

	/// @brief Value of `mpl::index_of`. Used to determine the index of the type `T`
	/// in the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	requires(contains_v<T, List>)
	inline static constexpr usize index_of_v = index_of<T, List>::value;
	// clang-format on

	namespace detail {
		template<usize N, typename... Types>
		struct max_size_impl : std::integral_constant<usize, 0> { };

		template<usize N, typename Head, typename... Types>
		struct max_size_impl<N, Head, Types...>
			: std::integral_constant<
				  usize,
				  max_size_impl<(sizeof(Head) > N ? sizeof(Head) : N), Types...>::value> { };

		template<usize N>
		struct max_size_impl<N> : std::integral_constant<usize, N> { };

		static_assert(4 == max_size_impl<0, u8, u16, byte, u32>::value,
					  "mpl::max_size implementation failing");
		static_assert(2 == max_size_impl<0, u8, u16, byte>::value,
					  "mpl::max_size implementation failing");
		static_assert(8 == max_size_impl<0, u8, u64, byte>::value, // NOLINT
					  "mpl::max_size implementation failing");

		template<typename List>
		struct max_size_list_impl;

		// clang-format off

		template<template<typename...> typename List, typename... Types>
		struct max_size_list_impl<List<Types...>> : max_size_impl<0, Types...> {
		};

		// clang-format on
	} // namespace detail

	/// @brief Used to determine the size of the largest type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to get the largest type for
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	struct max_size_of : detail::max_size_list_impl<List> { };

	/// @brief Value of `mpl::max_size_of`. Used to determine the size of the largest type in
	/// the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static inline constexpr usize max_size_of_v = max_size_of<List>::value;

	namespace detail {
		template<usize Instances, typename T, typename... Types>
		struct instances_of_impl : std::integral_constant<usize, 0> { };

		template<usize Instances, typename T, typename Head, typename... Types>
		struct instances_of_impl<Instances, T, Head, Types...>
			: std::conditional_t<std::is_same_v<T, Head>,
								 instances_of_impl<Instances + 1, T, Types...>,
								 instances_of_impl<Instances, T, Types...>> { };

		template<usize Instances, typename T>
		struct instances_of_impl<Instances, T> : std::integral_constant<usize, Instances> { };

		static_assert(instances_of_impl<0, u8, u8, u16, u32, u64>::value == 1,
					  "mpl::instances_of implementation failing");
		static_assert(instances_of_impl<0, u16, u8, u16, u32, u64>::value == 1,
					  "mpl::instances_of implementation failing");
		static_assert(instances_of_impl<0, u8, u8, u8, u32, u64>::value == 2,
					  "mpl::instances_of implementation failing");
		static_assert(instances_of_impl<0, i8, u8, u8, u32, u64>::value == 0,
					  "mpl::instances_of implementation failing");

		template<typename T, typename List>
		struct instances_of_list_impl;

		template<typename T, template<typename...> typename List, typename... Types>
		struct instances_of_list_impl<T, List<Types...>> : instances_of_impl<0, T, Types...> { };
	} // namespace detail

	/// @brief Used to determine the number of occurrences of `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to get the number of occurrences of
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	struct instances_of : detail::instances_of_list_impl<T, List> { };

	/// @brief Value of `instances_of`. Used to determine the number of occurrences of `T` in the
	/// `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr usize instances_of_v = instances_of<T, List>::value;

	namespace detail {
		// template<template<typename> typename T,
		//		 template<typename... Elements>
		//		 typename List,
		//		 typename... Elements>
		// static inline constexpr auto apply_to_list([[maybe_unused]] List<Elements...>&& list)
		// noexcept { 	return mpl::list<T<Elements>...>{};
		// }

		template<template<typename...> typename T,
				 template<typename... Elements>
				 typename List,
				 typename... Elements>
		static inline constexpr auto
		apply_to_list([[maybe_unused]] List<Elements...>&& list) noexcept {
			return mpl::list<T<Elements>...>{};
		}

	} // namespace detail

	template<template<typename...> typename T, typename List>
	using apply_to_list = decltype(detail::apply_to_list<T>(std::declval<List>()));

	IGNORE_UNUSED_VALUES_STOP
} // namespace hyperion::mpl
