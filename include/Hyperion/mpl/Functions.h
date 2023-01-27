/// @file Functions.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief meta-programming functions to determine if a given meta-condition matches the desired
/// value for various sets of arguments
/// @version 0.1
/// @date 2022-11-12
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
#include <Hyperion/mpl/HasValue.h>
#include <Hyperion/mpl/List.h>

namespace hyperion::mpl {

	/// @brief Meta-programming function to determine that at least one `Type` in the `mpl::list`,
	/// `List`, satisfies that the value of `ConditionType<Type>` equals the value of
	/// `RequirementType`.
	/// For example:
	///
	/// @code {.cpp}
	/// static_assert(
	/// 	any_type_satisfies<std::is_trivially_destructible,
	/// 				 std::true_type,
	/// 				 mpl::list<u8, u16, u32>>
	/// 	::value);
	/// @endcode
	///
	/// Checks that `std::is_trivially_destructible<T>` is true for at least one of `u8`,
	/// `u16`, or `u32`
	///
	/// @tparam ConditionType - The condition to check
	/// @tparam RequirementType - The required value of the condition
	/// @tparam List - The `mpl::list` of types to check the condition for
	/// @ingroup mpl
	template<template<typename> typename ConditionType, typename RequirementType, typename List>
	requires mpl::HasValue<ConditionType<mpl::first_t<List>>>
	static inline constexpr auto any_type_satisfies([[maybe_unused]] List list) {
		return hana::bool_c<hana::fold_left(List{}, false, [](bool state, auto value) noexcept {
			return state
				   || (RequirementType::value
					   == ConditionType<typename decltype(value)::type>::value);
		})>;
	}

	/// @brief Value of Meta-programming function `any_type_satisfies`. Used to determine that at
	/// least one type "Type" in the `mpl::list`, `List`, satisfies that the value of
	/// `ConditionType<Type>` equals the value of `RequirementType`
	/// @tparam ConditionType - The condition to check
	/// @tparam RequirementType - The required value of the condition
	/// @tparam List - The `mpl::list` of types to check the condition for
	/// @ingroup mpl
	template<template<typename> typename ConditionType, typename RequirementType, typename List>
	requires mpl::HasValue<ConditionType<mpl::first_t<List>>>
	static inline constexpr auto any_type_satisfies_v
		= hana::value_of(any_type_satisfies<ConditionType, RequirementType>(List{}));

	static_assert(any_type_satisfies_v<std::is_integral, std::true_type, mpl::list<u8, f32, f64>>,
				  "mpl::any_type_satisfies implementation failing");
	static_assert(
		!any_type_satisfies_v<std::is_floating_point, std::true_type, mpl::list<u8, i32, usize>>,
		"mpl::any_type_satisfies implementation failing");

	/// @brief Meta-programming function to determine that, given the `mpl::list` `CheckList`,
	/// containing types `CheckTypes...`, and the `mpl::list` `ArgumentList`, containing types
	/// `ArgumentTypes...`, at least one type, `Type`, in the `CheckList` satisfies that the
	/// value of `ConditionType<Type, ArgumentTypes...>` equals the value of `RequirementType`,
	/// where `RequirementType` is a type that `ConditionType` will compared against. For
	/// example:
	///
	/// @code {.cpp}
	/// static_assert(
	/// 	any_type_satisfies_with_arg_list<std::is_constructible,
	/// 								     std::true_type,
	/// 								     mpl::list<u64, umax>,
	/// 								     mpl::list<u8>
	///     >::value);
	/// @endcode
	///
	/// will check that `std::is_constructible<T, u8>::value` is true for at least one of `T`
	/// equal `u64` and/or `umax`
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam CheckList - The `mpl::list` of each type to check `ConditionType` for
	/// @tparam ArgList - The `mpl::list` of argument types to use as the additional
	/// arguments for `ConditionType`
	/// @ingroup mpl
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename CheckList,
			 typename ArgList>
	static inline constexpr auto
	any_type_satisfies_with_arg_list([[maybe_unused]] CheckList _list,
									 [[maybe_unused]] ArgList arglist) noexcept {
		constexpr auto satisfies = [](auto val) noexcept {
			constexpr auto satisfies_impl = [](auto... vals) {
				return RequirementType::value
					   == ConditionType<typename decltype(val)::type,
										typename decltype(vals)::type...>::value;
			};

			return hana::unpack(ArgList{}, satisfies_impl);
		};

		return hana::bool_c<hana::any_of(CheckList{}, satisfies)>;
	}

	/// @brief Value of Meta-programming function `any_type_satisfies_with_arg_list`. Used to
	/// check that, given the `mpl::list` `CheckList`, containing types `CheckTypes...`, and the
	/// `mpl::list` `ArgumentList`, containing types `ArgumentTypes...`, at least one type,
	/// `Type`, in the `CheckList` satisfies that the value of `ConditionType<Type,
	/// ArgumentTypes...>` equals the value of `RequirementType`.
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam CheckList - The `mpl::list` of each type to check `ConditionType` for
	/// @tparam ArgList - The `mpl::list` of argument types to use as the additional
	/// arguments for `ConditionType`
	/// @ingroup mpl
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename CheckList,
			 typename ArgList>
	static inline constexpr auto any_type_satisfies_with_arg_list_v = hana::value_of(
		any_type_satisfies_with_arg_list<ConditionType, RequirementType>(CheckList{}, ArgList{}));

	namespace detail {
		class atswal_test {
		  public:
			atswal_test(i32, f32);
			i32 first;
			f32 second;
		};

		static_assert(any_type_satisfies_with_arg_list_v<std::is_constructible,
														 std::true_type,
														 mpl::list<f32, usize, atswal_test>,
														 mpl::list<i32, f32>>,
					  "mpl::any_type_satisfies_with_arg_list implementation failing");
		static_assert(!any_type_satisfies_with_arg_list_v<std::is_constructible,
														  std::true_type,
														  mpl::list<f32, usize>,
														  mpl::list<i32, f32>>,
					  "mpl::any_type_satisfies_with_arg_list implementation failing");
	} // namespace detail

	/// @brief Meta-programming function to determine that every type `Type` in the `mpl::list`,
	/// `List`, satisfies that the value of `ConditionType<Type>` equals the value of
	/// `RequirementType`.
	/// For example:
	///
	/// @code {.cpp}
	/// all_types_satisfy<std::is_trivially_destructible, std::true_type, mpl::list<u8, u16,
	/// u32>>;
	/// @endcode
	///
	/// Checks that `std::is_trivially_destructible<T>` is true for each of `T` equal to `u8`,
	/// `u16`, and `u32`
	///
	/// @tparam ConditionType - The condition to check
	/// @tparam RequirementType - The required value of the condition
	/// @tparam List - The `mpl::list` of types to check the condition for
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/ForAll.h"
	template<template<typename> typename ConditionType, HasValue RequirementType, typename List>
	requires HasValue<ConditionType<mpl::first_t<List>>>
	static inline constexpr auto all_types_satisfy([[maybe_unused]] List list) noexcept {
		return hana::bool_c<hana::fold_left(List{}, true, [](bool state, auto value) noexcept {
			return state
				   && (RequirementType::value
					   == ConditionType<typename decltype(value)::type>::value);
		})>;
	}

	/// @brief Value of Meta-programming function `for_all`. Used to determine that every type
	/// "Type" in the `mpl::list`, `List`, satisfies that the value of `ConditionType<Type>`
	/// equals the value of `RequirementType`
	template<template<typename> typename ConditionType, HasValue RequirementType, typename List>
	requires HasValue<ConditionType<mpl::first_t<List>>>
	static inline constexpr bool all_types_satisfy_v
		= hana::value_of(all_types_satisfy<ConditionType, RequirementType>(List{}));

	static_assert(
		all_types_satisfy_v<std::is_default_constructible, std::true_type, mpl::list<u8, u32, f32>>,
		"mpl::all_types_satisfy implementation failing");
	static_assert(!all_types_satisfy_v<std::is_integral, std::true_type, mpl::list<u8, u32, f32>>,
				  "mpl::all_types_satisfy implementation failing");

	/// @brief Meta-programming function to determine that for every list of types, `Types...`
	/// in `Lists`, the value of `ConditionType<Evaluatee, Types...>` is equal to the value of
	/// `RequirementType`
	///
	/// For example:
	/// @code {.cpp}
	/// all_lists_satisfy_for_type<std::is_constructible, std::true_type, std::string,
	/// 	mpl::list<mpl::list<usize, char>, mpl::list<const char*, usize>>>;
	/// @endcode
	///
	/// will check that `std::is_constructible<std::string, Types...>::value` is true for
	/// `Types...` equal to the list `usize, char` and equal to the list `const char*, usize`.
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Types...>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam Evaluatee - The type that `ConditionType` should compare or evaluate each type,
	/// `Type`, in `List` against
	/// @tparam Lists - The `mpl::list` of each `mpl::list` of types to pass to `ConditionType`
	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename Lists>
	static inline constexpr auto all_lists_satisfy_for_type([[maybe_unused]] Lists lists) noexcept {
		return hana::bool_c<hana::fold_left(Lists{}, true, [](bool state, auto list) noexcept {
			constexpr auto satisfies = [](auto... types) noexcept {
				return RequirementType::value
					   == ConditionType<Evaluatee, typename decltype(types)::type...>::value;
			};

			using list_t = typename decltype(list)::type;
			return state && hana::unpack(list_t{}, satisfies);
		})>;
	}

	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename Lists>
	static inline constexpr bool all_lists_satisfy_for_type_v = hana::value_of(
		all_lists_satisfy_for_type<ConditionType, RequirementType, Evaluatee>(Lists{}));

	namespace detail::test {
		struct AllListsSatisfyTest1 {
			AllListsSatisfyTest1(u8, u8, u8, u8) { // NOLINT
			}
			AllListsSatisfyTest1(u16, u16, u16, u16) { // NOLINT
			}
			AllListsSatisfyTest1(u32, u32, u32, u32) { // NOLINT
			}
		};

		struct AllListsSatisfyTest2 {
			// NOLINTNEXTLINE
			AllListsSatisfyTest2(u8) {
			}
			// NOLINTNEXTLINE
			AllListsSatisfyTest2(f32) {
			}
			// NOLINTNEXTLINE
			AllListsSatisfyTest2(i64) {
			}
		};

		static_assert(all_lists_satisfy_for_type_v<std::is_constructible,
												   std::true_type,
												   AllListsSatisfyTest1,
												   mpl::list<mpl::list<u8, u8, u8, u8>,
															 mpl::list<u16, u16, u16, u16>,
															 mpl::list<u32, u32, u32, u32>>>,
					  "all_lists_satisfy_for_type failing");

		static_assert(!all_lists_satisfy_for_type_v<std::is_constructible,
													std::true_type,
													AllListsSatisfyTest1,
													mpl::list<mpl::list<u8, u8, u32, u32>,
															  mpl::list<u16, u16, u16, u16>,
															  mpl::list<u32, u32, u8, u8>>>,
					  "all_lists_satisfy_for_type failing");

		static_assert(!all_lists_satisfy_for_type_v<std::is_constructible,
													std::true_type,
													AllListsSatisfyTest1,
													mpl::list<mpl::list<i8, i8, i8, i8>,
															  mpl::list<i16, i16, i16, i16>,
															  mpl::list<i32, i32, i32, i32>>>,
					  "all_lists_satisfy_for_type failing");

		static_assert(all_lists_satisfy_for_type_v<
						  std::is_constructible,
						  std::true_type,
						  std::string,
						  mpl::list<mpl::list<usize, char>, mpl::list<const char*, usize>>>,
					  "all_lists_satisfy_for_type failing");

		static_assert(
			all_lists_satisfy_for_type_v<std::is_constructible,
										 std::true_type,
										 AllListsSatisfyTest2,
										 mpl::apply_to_list<mpl::list, mpl::list<u8, f32, i64>>>,
			"mpl::all_lists_satisfy_for_type implementation failing");
	} // namespace detail::test
} // namespace hyperion::mpl
