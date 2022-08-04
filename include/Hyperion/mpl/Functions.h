/// @file Functions.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief meta-programming functions to determine if a given meta-condition matches the desired
/// value for various sets of arguments
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
#include <Hyperion/mpl/HasValue.h>
#include <Hyperion/mpl/List.h>

namespace hyperion::mpl {

	namespace detail {
		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename... Types>
		struct any_type_satisfies_impl : std::false_type { };

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename Head,
				 typename... Types>
		requires mpl::HasValue<RequirementType> && mpl::HasValue<ConditionType<Head>>
		struct any_type_satisfies_impl<ConditionType, RequirementType, Head, Types...>
			: std::conditional_t<
				  ConditionType<Head>::value == RequirementType::value,
				  std::true_type,
				  any_type_satisfies_impl<ConditionType, RequirementType, Types...>> {
		};

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename Head,
				 typename... Types>
		struct any_type_satisfies_impl<ConditionType, RequirementType, Head, Types...>
			: std::conditional_t<
				  std::is_same_v<ConditionType<Head>, RequirementType>,
				  std::true_type,
				  any_type_satisfies_impl<ConditionType, RequirementType, Types...>> { };

		template<template<typename> typename ConditionType, typename RequirementType>
		struct any_type_satisfies_impl<ConditionType, RequirementType> : std::false_type { };

		template<template<typename> typename ConditionType, typename RequirementType, typename List>
		struct any_type_satisfies_list;

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename List,
				 typename... Types>
		struct any_type_satisfies_list<ConditionType, RequirementType, List<Types...>>
			: any_type_satisfies_impl<ConditionType, RequirementType, Types...> { };

		static_assert(any_type_satisfies_list<std::is_integral,
											  std::true_type,
											  mpl::list<u8, u32, f32>>::value,
					  "any_type_satisfies failing");
		static_assert(any_type_satisfies_list<std::is_floating_point,
											  std::true_type,
											  mpl::list<u8, u32, f32>>::value,
					  "any_type_satisfies failing");
		static_assert(!any_type_satisfies_list<std::is_floating_point,
											   std::true_type,
											   mpl::list<u8, u32, i32>>::value,
					  "any_type_satisfies failing");
	} // namespace detail

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
	struct any_type_satisfies
		: detail::any_type_satisfies_list<ConditionType, RequirementType, List> {
	};

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
		= any_type_satisfies<ConditionType, RequirementType, List>::value;

	namespace detail {
		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename CheckList,
				 typename ArgumentList>
		struct any_type_satisfies_with_arg_list_impl : std::false_type { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename CheckList,
				 template<typename...>
				 typename ArgumentList,
				 typename... CheckTypes,
				 typename... ArgumentTypes>
		struct any_type_satisfies_with_arg_list_impl<ConditionType,
													 RequirementType,
													 CheckList<CheckTypes...>,
													 ArgumentList<ArgumentTypes...>>
			: std::false_type { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename CheckList,
				 template<typename...>
				 typename ArgumentList,
				 typename Head,
				 typename... CheckTypes,
				 typename... ArgumentTypes>
		requires mpl::HasValue<ConditionType<Head, ArgumentTypes...>> && mpl::HasValue<
			RequirementType>
		struct any_type_satisfies_with_arg_list_impl<ConditionType,
													 RequirementType,
													 CheckList<Head, CheckTypes...>,
													 ArgumentList<ArgumentTypes...>>
			: std::conditional_t<
				  ConditionType<Head, ArgumentTypes...>::value == RequirementType::value,
				  std::true_type,
				  any_type_satisfies_with_arg_list_impl<ConditionType,
														RequirementType,
														CheckList<CheckTypes...>,
														ArgumentList<ArgumentTypes...>>> {
		};

		// clang-format off

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename CheckList,
				 template<typename...>
				 typename ArgumentList,
				 typename Head,
				 typename... CheckTypes,
				 typename... ArgumentTypes>
		requires(!mpl::HasValue<ConditionType<Head, ArgumentTypes...>>)
        struct any_type_satisfies_with_arg_list_impl<ConditionType,
												  RequirementType,
												  CheckList<Head, CheckTypes...>,
												  ArgumentList<ArgumentTypes...>>
			// clang-format on
			: std::conditional_t<
				  std::is_same_v<ConditionType<Head, ArgumentTypes...>, RequirementType>,
				  std::true_type,
				  any_type_satisfies_with_arg_list_impl<ConditionType,
														RequirementType,
														CheckList<CheckTypes...>,
														ArgumentList<ArgumentTypes...>>> {
		};

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename CheckList,
				 template<typename...>
				 typename ArgumentList,
				 typename... ArgumentTypes>
		struct any_type_satisfies_with_arg_list_impl<ConditionType,
													 RequirementType,
													 CheckList<>,
													 ArgumentList<ArgumentTypes...>>
			: std::false_type { };

		template<typename T>
		struct atswal_val {
			explicit atswal_val(T _val) : val(_val){};
			T val;
		};

		struct atswal1 {
			explicit atswal1(atswal_val<u8> _value) : value(_value.val) {
			}
			u8 value;
		};

		struct atswal2 {
			explicit atswal2(atswal_val<f32> _value) : value(_value.val) {
			}
			f32 value;
		};

		struct atswal3 {
			explicit atswal3(atswal_val<u32> _value) : value(_value.val) {
			}
			u32 value;
		};

		static_assert(any_type_satisfies_with_arg_list_impl<std::is_constructible,
															std::true_type,
															mpl::list<atswal1, atswal2, atswal3>,
															mpl::list<atswal_val<u8>>>::value,
					  "any_type_satisfies_with_arg_list failing");
		static_assert(any_type_satisfies_with_arg_list_impl<std::is_constructible,
															std::true_type,
															mpl::list<atswal1, atswal2, atswal3>,
															mpl::list<atswal_val<u32>>>::value,
					  "any_type_satisfies_with_arg_list failing");
		static_assert(any_type_satisfies_with_arg_list_impl<std::is_constructible,
															std::true_type,
															mpl::list<atswal1, atswal2, atswal3>,
															mpl::list<atswal_val<f32>>>::value,
					  "any_type_satisfies_with_arg_list failing");
		static_assert(!any_type_satisfies_with_arg_list_impl<std::is_constructible,
															 std::true_type,
															 mpl::list<atswal1, atswal2, atswal3>,
															 mpl::list<atswal_val<u16>>>::value,
					  "any_type_satisfies_with_arg_list failing");
		static_assert(!any_type_satisfies_with_arg_list_impl<std::is_constructible,
															 std::true_type,
															 mpl::list<atswal1, atswal2, atswal3>,
															 mpl::list<atswal_val<i16>>>::value,
					  "any_type_satisfies_with_arg_list failing");
	} // namespace detail

	/// @brief Meta-programming function to determine that, given the `mpl::list` `CheckList`,
	/// containing types `CheckTypes...`, and the `mpl::list` `ArgumentList`, containing types
	/// `ArgumentTypes...`, at least one type, `Type`, in the `CheckList` satisfies that the value of
	/// `ConditionType<Type, ArgumentTypes...>` equals the value of `RequirementType`, where
	/// `RequirementType` is a type that `ConditionType` will compared against.
	/// For example:
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
	/// will check that `std::is_constructible<T, u8>::value` is true for at least one of `T` equal
	/// `u64` and/or `umax`
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam CheckList - The `mpl::list` of each type to check `ConditionType` for
	/// @tparam ArgumentList - The `mpl::list` of argument types to use as the additional arguments
	/// for `ConditionType`
    /// @ingroup mpl
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename CheckList,
			 typename ArgumentList>
	struct any_type_satisfies_with_arg_list
		: detail::any_type_satisfies_with_arg_list_impl<ConditionType,
														RequirementType,
														CheckList,
														ArgumentList> { };

	/// @brief Value of Meta-programming function `any_type_satisfies_with_arg_list`. Used to check
    /// that, given the `mpl::list` `CheckList`, containing types `CheckTypes...`, and the
    /// `mpl::list` `ArgumentList`, containing types `ArgumentTypes...`, at least one type, `Type`,
    /// in the `CheckList` satisfies that the value of `ConditionType<Type, ArgumentTypes...>`
    /// equals the value of `RequirementType`.
    ///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam CheckList - The `mpl::list` of each type to check `ConditionType` for
	/// @tparam ArgumentList - The `mpl::list` of argument types to use as the additional arguments
	/// for `ConditionType`
    /// @ingroup mpl
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename CheckList,
			 typename ArgumentList>
	static inline constexpr auto any_type_satisfies_with_arg_list_v
		= any_type_satisfies_with_arg_list<ConditionType,
										   RequirementType,
										   CheckList,
										   ArgumentList>::value;

	namespace detail {
		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename... Types>
		struct all_types_satisfy_impl : std::true_type { };

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 typename Head,
				 typename... Types>
		struct all_types_satisfy_impl<ConditionType, RequirementType, Head, Types...>
			: std::conditional_t<
				  !(std::is_same_v<
						ConditionType<Head>,
						RequirementType> || ConditionType<Head>::value == RequirementType::value),
				  std::false_type,
				  all_types_satisfy_impl<ConditionType, RequirementType, Types...>> { };

		template<template<typename> typename ConditionType, typename RequirementType>
		struct all_types_satisfy_impl<ConditionType, RequirementType> : std::true_type { };

		namespace test {
			static_assert(all_types_satisfy_impl<std::is_trivially_destructible,
											 std::true_type,
											 u8,
											 u16,
											 u32,
											 u64>::value,
						  "mpl::for_all implementation failing");
			static_assert(!all_types_satisfy_impl<std::is_trivially_destructible,
											  std::true_type,
											  u8,
											  u16,
											  std::string,
											  u64>::value,
						  "mpl::for_all implementation failing");
		} // namespace test

		template<template<typename> typename ConditionType, typename RequirementType, typename List>
		struct all_types_satisfy_list_impl;

		// clang-format off

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename List,
				 typename... Types>
		struct all_types_satisfy_list_impl<ConditionType, RequirementType, List<Types...>>
			: all_types_satisfy_impl<ConditionType, RequirementType, Types...> { };

		// clang-format on
	} // namespace detail

	/// @brief Meta-programming function to determine that every type `Type` in the `mpl::list`,
	/// `List`, satisfies that the value of `ConditionType<Type>` equals the value of
	/// `RequirementType`.
	/// For example:
	///
	/// @code {.cpp}
	/// all_types_satisfy<std::is_trivially_destructible, std::true_type, mpl::list<u8, u16, u32>>;
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
	struct all_types_satisfy : detail::all_types_satisfy_list_impl<ConditionType, RequirementType, List> {
	};

	/// @brief Value of Meta-programming function `for_all`. Used to determine that every type
	/// "Type" in the `mpl::list`, `List`, satisfies that the value of `ConditionType<Type>` equals
	/// the value of `RequirementType`
	template<template<typename> typename ConditionType, HasValue RequirementType, typename List>
	requires HasValue<ConditionType<mpl::first_t<List>>>
	static inline constexpr bool all_types_satisfy_v
		= all_types_satisfy<ConditionType, RequirementType, List>::value;

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
		struct all_lists_satisfy_for_type_impl
			: public all_types_satisfy<
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
	/// all_lists_satisfy_for_type<std::is_constructible, std::true_type, std::string,
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
	struct all_lists_satisfy_for_type;

	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 template<typename...>
			 typename List,
			 typename... Lists>
	requires HasValue<ConditionType<Evaluatee, mpl::first_t<mpl::first_t<List<Lists...>>>>>
	struct all_lists_satisfy_for_type<ConditionType, RequirementType, Evaluatee, List<Lists...>>
		: detail::all_lists_satisfy_for_type_impl<ConditionType,
											   RequirementType,
											   Evaluatee,
											   List,
											   Lists...> {
	};

	template<template<typename, typename...> typename ConditionType,
			 HasValue RequirementType,
			 typename Evaluatee,
			 typename Lists>
	static inline constexpr bool all_lists_satisfy_for_type_v
		= all_lists_satisfy_for_type<ConditionType, RequirementType, Evaluatee, Lists>::value;

	namespace detail::test {
		struct AllListsSatisfyTest1 {
			AllListsSatisfyTest1(u8, u8, u8, u8) { // NOLINT
			}
			AllListsSatisfyTest1(u16, u16, u16, u16) { // NOLINT
			}
			AllListsSatisfyTest1(u32, u32, u32, u32) { // NOLINT
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
	} // namespace detail::test
} // namespace hyperion::mpl
