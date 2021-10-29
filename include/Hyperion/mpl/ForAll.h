/// @file ForAll.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief meta-programming functions to determine if a given meta-condition matches the desired
/// value for every element in a list of types
/// @version 0.1
/// @date 2021-10-20
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

	namespace detail {
		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename... Types>
		struct for_all_types_impl : std::true_type { };

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename Head,
				 typename... Types>
		struct for_all_types_impl<ConditionType, RequirementType, Head, Types...>
			: std::conditional_t<
				  !(std::is_same_v<
						ConditionType<Head>,
						RequirementType> || ConditionType<Head>::value == RequirementType::value),
				  std::false_type,
				  for_all_types_impl<ConditionType, RequirementType, Types...>> { };

		template<template<typename> typename ConditionType, typename RequirementType>
		struct for_all_types_impl<ConditionType, RequirementType> : std::true_type { };

		namespace test {
			static_assert(for_all_types_impl<std::is_trivially_destructible,
											 std::true_type,
											 u8,
											 u16,
											 u32,
											 u64>::value,
						  "mpl::for_all implementation failing");
			static_assert(!for_all_types_impl<std::is_trivially_destructible,
											  std::true_type,
											  u8,
											  u16,
											  std::string,
											  u64>::value,
						  "mpl::for_all implementation failing");
		} // namespace test

		template<template<typename> typename ConditionType, typename RequirementType, typename List>
		struct for_all_types_list_impl;

		// clang-format off

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename List,
				 typename... Types>
		struct for_all_types_list_impl<ConditionType, RequirementType, List<Types...>>
			: for_all_types_impl<ConditionType, RequirementType, Types...> { };

		// clang-format on
	} // namespace detail

	/// @brief Meta-programming function to determine that every type `Type` in the `mpl::list`,
	/// `List`, satisfies that the value of `ConditionType<Type>` equals the value of
	/// `RequirementType`.
	/// For example:
	///
	/// @code {.cpp}
	/// for_all_types<std::is_trivially_destructible, std::true_type, mpl::list<u8, u16, u32>>;
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
	struct for_all_types : detail::for_all_types_list_impl<ConditionType, RequirementType, List> {
	};

	/// @brief Value of Meta-programming function `for_all`. Used to determine that every type
	/// "Type" in the `mpl::list`, `List`, satisfies that the value of `ConditionType<Type>` equals
	/// the value of `RequirementType`
	template<template<typename> typename ConditionType, HasValue RequirementType, typename List>
	requires HasValue<ConditionType<mpl::first_t<List>>>
	inline static constexpr bool for_all_types_v
		= for_all_types<ConditionType, RequirementType, List>::value;

	namespace detail {
		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename... Types>
		struct for_all_parameters_impl : std::true_type { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename Head,
				 typename... Types>
		struct for_all_parameters_impl<ConditionType, RequirementType, Evaluatee, Head, Types...>
			: std::conditional_t<
				  !(std::is_same_v<
						ConditionType<Evaluatee, Head>,
						RequirementType> || ConditionType<Evaluatee, Head>::value == RequirementType::value),
				  std::false_type,
				  for_all_parameters_impl<ConditionType, RequirementType, Evaluatee, Types...>> { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee>
		struct for_all_parameters_impl<ConditionType, RequirementType, Evaluatee> : std::true_type {
		};

		namespace test {
			static_assert(
				for_all_parameters_impl<std::is_constructible, std::true_type, umax, u8, u32, u64>::
					value,
				"mpl::for_all_parameters implementation failing");
			static_assert(!for_all_parameters_impl<std::is_constructible,
												   std::true_type,
												   umax,
												   u8,
												   std::string,
												   u64>::value,
						  "mpl::for_all_parameters implementation failing");
		} // namespace test

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename List>
		struct for_all_parameters_list_impl;

		// clang-format off

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 template<typename...>
				 typename List,
				 typename... Types>
		struct for_all_parameters_list_impl<ConditionType, RequirementType, Evaluatee, List<Types...>>
			: for_all_parameters_impl<ConditionType, RequirementType, Evaluatee, Types...> { };
		// clang-format on
	} // namespace detail

	/// @brief Meta-programming function to determine that every type `Type` in the `mpl::list`,
	/// `List`, satisfies that the value of `ConditionType<Evaluatee, Type>` equals the value of
	/// `RequirementType`, where `Evaluatee` is a type that `ConditionType` will compare or evaluate
	/// `Type` against in some way.
	/// For example:
	///
	/// @code {.cpp}
	/// for_all_parameters<std::is_constructible, std::true_type, umax, mpl::list<u8, u32, u64>>
	/// @endcode
	///
	/// will check that `std::is_constructible<umax, T>::value` is true for `T` equal to each
	/// of `u8`, `u32`, `u64`
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam Evaluatee - The type that `ConditionType` should compare or evaluate each type,
	/// `Type`, in `List` against
	/// @tparam List - The `mpl::list` of each `Type` to check `ConditionType` for
	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename List>
	requires HasValue<ConditionType<Evaluatee, mpl::first_t<List>>>
	struct for_all_parameters
		: detail::for_all_parameters_list_impl<ConditionType, RequirementType, Evaluatee, List> {
	};

	/// @brief Value of Meta-programming function `for_all_parameters`.
	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename List>
	requires HasValue<ConditionType<Evaluatee, mpl::first_t<List>>>
	inline static constexpr bool for_all_parameters_v
		= for_all_parameters<ConditionType, RequirementType, Evaluatee, List>::value;

	namespace detail {
		template<template<template<typename, typename...> typename ConditionType,
						  typename RequirementType,
						  typename Evaluatee,
						  typename... Types>
				 typename MetaFunction,
				 template<typename, typename...>
				 typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename... Types>
		struct for_parameters_in_list_captured_impl
			: public MetaFunction<ConditionType, RequirementType, Evaluatee, Types...> { };

		template<template<template<typename, typename...> typename ConditionType,
						  typename RequirementType,
						  typename Evaluatee,
						  typename... Types>
				 typename MetaFunction,
				 template<typename, typename...>
				 typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename List>
		struct for_parameters_in_list_impl;

		template<template<template<typename, typename...> typename ConditionType,
						  typename RequirementType,
						  typename Evaluatee,
						  typename... Types>
				 typename MetaFunction,
				 template<typename, typename...>
				 typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 template<typename...>
				 typename List,
				 typename... Types>
		struct for_parameters_in_list_impl<MetaFunction,
										   ConditionType,
										   RequirementType,
										   Evaluatee,
										   List<Types...>>
			: public for_parameters_in_list_captured_impl<MetaFunction,
														  ConditionType,
														  RequirementType,
														  Evaluatee,
														  Types...> { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename... Types>
		struct for_all_parameters_in_list_impl
			: public std::conditional_t<
				  std::is_same_v<
					  ConditionType<Evaluatee, Types...>,
					  RequirementType> || ConditionType<Evaluatee, Types...>::value == RequirementType::value,
				  std::true_type,
				  std::false_type> { };

		template<typename T>
		struct is_true : std::conditional_t<T::value, std::true_type, std::false_type> { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 template<typename...>
				 typename Lists,
				 typename... List>
		struct for_all_parameter_lists_impl
			: public for_all_types<
				  detail::is_true,
				  std::true_type,
				  mpl::list<for_parameters_in_list_impl<detail::for_all_parameters_in_list_impl,
														ConditionType,
														RequirementType,
														Evaluatee,
														List>...>> { };

	} // namespace detail

	/// @brief Meta-programming function to determine that for every list of types, `Types...` in
	/// `Lists`, the value of `ConditionType<Evaluatee, Types...>` is equal to the value of
	/// `RequirementType`
	///
	/// For example:
	/// @code {.cpp}
	/// for_all_parameter_lists<std::is_constructible, std::true_type, std::string,
	/// 	mpl::list<mpl::list<usize, char>, mpl::list<const char*, usize>>>;
	/// @endcode
	///
	/// will check that `std::is_constructible<std::string, Types...>::value` is true for `Types...`
	/// equal to the list `usize, char` and equal to the list `const char*, usize`.
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
	struct for_all_parameter_lists;

	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 template<typename...>
			 typename List,
			 typename... Lists>
	requires HasValue<ConditionType<Evaluatee, mpl::first_t<mpl::first_t<List<Lists...>>>>>
	struct for_all_parameter_lists<ConditionType, RequirementType, Evaluatee, List<Lists...>>
		: detail::for_all_parameter_lists_impl<ConditionType,
											   RequirementType,
											   Evaluatee,
											   List,
											   Lists...> {
	};

	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename Lists>
	inline static constexpr bool for_all_parameter_lists_v
		= for_all_parameter_lists<ConditionType, RequirementType, Evaluatee, Lists>::value;

	namespace detail::test {
		struct ForAllParameterListsTest1 {
			ForAllParameterListsTest1(u8, u8, u8, u8) { // NOLINT
			}
			ForAllParameterListsTest1(u16, u16, u16, u16) { // NOLINT
			}
			ForAllParameterListsTest1(u32, u32, u32, u32) { // NOLINT
			}
		};

		static_assert(for_all_parameter_lists_v<std::is_constructible,
												std::true_type,
												ForAllParameterListsTest1,
												mpl::list<mpl::list<u8, u8, u8, u8>,
														  mpl::list<u16, u16, u16, u16>,
														  mpl::list<u32, u32, u32, u32>>>,
					  "for_all_parameter_lists failing");

		static_assert(!for_all_parameter_lists_v<std::is_constructible,
												 std::true_type,
												 ForAllParameterListsTest1,
												 mpl::list<mpl::list<u8, u8, u32, u32>,
														   mpl::list<u16, u16, u16, u16>,
														   mpl::list<u32, u32, u8, u8>>>,
					  "for_all_parameter_lists failing");

		static_assert(!for_all_parameter_lists_v<std::is_constructible,
												 std::true_type,
												 ForAllParameterListsTest1,
												 mpl::list<mpl::list<i8, i8, i8, i8>,
														   mpl::list<i16, i16, i16, i16>,
														   mpl::list<i32, i32, i32, i32>>>,
					  "for_all_parameter_lists failing");

		static_assert(for_all_parameter_lists_v<
						  std::is_constructible,
						  std::true_type,
						  std::string,
						  mpl::list<mpl::list<usize, char>, mpl::list<const char*, usize>>>,
					  "for_all_parameter_lists failing");
	} // namespace detail::test
} // namespace hyperion::mpl
