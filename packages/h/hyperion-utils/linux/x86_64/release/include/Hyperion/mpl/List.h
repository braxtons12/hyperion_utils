/// @file Index.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Meta-programming facilities for working with a list of types
/// @version 0.1
/// @date 2022-08-07
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
#include <Hyperion/mpl/Index.h>

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
	using list = hana::tuple<hana::type<T>...>;

	template<typename... T>
	static constexpr auto list_v = hana::tuple_t<T...>;

	/// @brief Used to determine the number of elements in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to query the size of
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static constexpr auto size(List list) noexcept {
		return hana::size(list);
	}

	/// @brief Used to determine the number of elements in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to query the size of
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static inline constexpr auto size_v = decltype(size(List{}))::value;

	/// @brief Used to find the `N`th type in the `mpl::list`, `List`
	///
	/// @tparam N - The index into the list
	/// @tparam List - The `mpl::list` to get a type from
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<usize N, typename List>
	requires(N < size_v<List>) static inline constexpr auto at(List list) noexcept {
		return hana::at_c<N>(list);
	}

	// clang-format on

	/// @brief Used to find the `N`th type in the `mpl::list`, `List`
	///
	/// @tparam N - The index into the list
	/// @tparam List - The `mpl::list` to get a type from
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<usize N, typename List>
	using at_t = typename decltype(at<N>(List{}))::type;

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
	struct first : decltype(at<0>(List{})) { };

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
	struct last : decltype(at < mpl::size_v<List> >= 1_usize ? mpl::size_v<List> - 1_usize :
															   0_usize > (List{})) { };

	/// @brief Alias to `mpl::last::type`. Used to get the last type in the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	using last_t = typename last<List>::type;

	static_assert(std::is_same_v<last_t<list<u8, u16, u32>>, u32>,
				  "mpl::last implementation failing");
	static_assert(std::is_same_v<last_t<list<u32, u16, u8>>, u8>,
				  "mpl::last implementation failing");

	/// @brief Used to determine if the given `mpl::list`, `List`, contains the type `T`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr auto contains(List list) noexcept {
		return hana::integral_c<bool, [list]() noexcept {
			auto found = false;
			hana::for_each(list, [&found](auto val) {
				found = found || hana::equal(val, hana::type_c<T>);
			});
			return found;
		}()>;
	}

	/// @brief Value of `mpl::contains`. Used to determine if the given `mpl::list`, `List`,
	/// contains the given type `T`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr auto contains_v = decltype(contains<T>(List{}))::value;

	static_assert(contains_v<u8, list<u16, u32, u8, u64>>, "mpl::contains implementation failing");
	static_assert(!contains_v<i8, list<u16, u32, u8, u64>>, "mpl::contains implementation failing");

	// clang-format off

	/// @brief Used to get the index of the type `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	requires(contains_v<T, List>)
    static inline constexpr auto index_of(List list) noexcept {
        constexpr auto res = hana::index_if(list, hana::equal_t::to(hana::type_c<T>));
        return mpl::index<res.value()>{};
    }


	/// @brief Value of `mpl::index_of`. Used to determine the index of the type `T`
	/// in the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	requires(contains_v<T, List>)
	inline static constexpr usize index_of_v = decltype(index_of<T>(List{}))::value;

    static_assert(index_of_v<u8, list<u16, u32, u8, u64>> == 2_usize,
                  "mpl::index_of implementation failing");
    static_assert(index_of_v<u32, list<u16, u32, u8, u64>> == 1_usize,
                  "mpl::index_of implementation failing");

	// clang-format on

	/// @brief Used to determine the size of the largest type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to get the largest type for
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static inline constexpr auto max_size_of(List list) noexcept {
		return mpl::index<[list]() noexcept {
			auto size = 0_usize;
			hana::for_each(list, [&size](auto val) noexcept {
				constexpr auto _size = sizeof(typename decltype(hana::decltype_(val))::type);
				size = std::max(_size, size);
			});
			return size;
		}()>{};
	}

	/// @brief Value of `mpl::max_size_of`. Used to determine the size of the largest type in
	/// the `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename List>
	static inline constexpr usize max_size_of_v = decltype(max_size_of(List{}))::value;

	static_assert(max_size_of_v<list<u8, u16, u32, u64>> == 8_usize,
				  "mpl::max_size_of implementation failing");
	static_assert(max_size_of_v<list<i8, i16, bool, f32>> == 4_usize,
				  "mpl::max_size_of implementation failing");

	/// @brief Used to determine the number of occurrences of `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to get the number of occurrences of
	/// @tparam List - The `mpl::list` to search in
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr auto instances_of(List list) noexcept {
		return mpl::index<[list]() noexcept {
			auto num_instances = 0_usize;
			hana::for_each(list, [&num_instances](auto val) noexcept {
				if(hana::equal(hana::type_c<T>, val)) {
					num_instances++;
				}
			});
			return num_instances;
		}()>{};
	}

	/// @brief Value of `instances_of`. Used to determine the number of occurrences of `T` in the
	/// `mpl::list`, `List`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/List.h"
	template<typename T, typename List>
	static inline constexpr usize instances_of_v = decltype(instances_of<T>(List{}))::value;

	static_assert(instances_of_v<u16, list<u16, u32, u16, u64>> == 2_usize,
				  "mpl::instances_of implementation failing");
	static_assert(instances_of_v<u64, list<u16, u32, u16, u64>> == 1_usize,
				  "mpl::instances_of implementation failing");
	static_assert(instances_of_v<i64, list<u16, u32, u16, u64>> == 0_usize,
				  "mpl::instances_of implementation failing");

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
		apply_to_list([[maybe_unused]] List<Elements...> list) noexcept {
			return hana::unpack(list, [](auto... elems) noexcept {
				return mpl::list<T<typename decltype(elems)::type>...>{};
			});
		}

	} // namespace detail

	template<template<typename...> typename T, typename List>
	using apply_to_list = decltype(detail::apply_to_list<T>(std::declval<List>()));

	static_assert(std::same_as<apply_to_list<mpl::list, mpl::list<u8, u16, u32>>,
							   mpl::list<mpl::list<u8>, mpl::list<u16>, mpl::list<u32>>>,
				  "mpl::apply_to_list implementation failing");
	IGNORE_UNUSED_VALUES_STOP
} // namespace hyperion::mpl
