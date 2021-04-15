#pragma once

#include "../BasicTypes.h"
#include "List.h"

namespace hyperion::utils::mpl {

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

		static_assert(
			for_all_types_impl<std::is_trivially_destructible, std::true_type, u8, u16, u32, u64>::
				value,
			"mpl::for_all implementation failing");
		static_assert(!for_all_types_impl<std::is_trivially_destructible,
										  std::true_type,
										  u8,
										  u16,
										  std::string,
										  u64>::value,
					  "mpl::for_all implementation failing");

		template<template<typename> typename ConditionType, typename RequirementType, typename List>
		struct for_all_types_list_impl;

		// clang-format off

		template<template<typename> typename ConditionType,
				 typename RequirementType,
				 template<typename...>
				 typename List,
				 typename... Types>
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct for_all_types_list_impl<ConditionType, RequirementType, List<Types...>>
			: for_all_types_impl<ConditionType, RequirementType, Types...> { };

		// clang-format on
	} // namespace detail

	/// @brief Type Trait to determine that every type "Type" in the `mpl::list`, `List`, satisfies
	/// that the value of `ConditionType<Type>` equals the value of `RequirementType`
	///
	/// @tparam ConditionType - The condition to check
	/// @tparam RequirementType - The required value of the condition
	/// @tparam List - The `mpl::list` of types to check the condition for
	template<template<typename> typename ConditionType, typename RequirementType, typename List>
	struct for_all_types : detail::for_all_types_list_impl<ConditionType, RequirementType, List> {
	};

	/// @brief Value of Type Trait `for_all`. Used to determine that every type "Type" in the
	/// `mpl::list`, `List`, satisfies that the value of `ConditionType<Type>` equals the value of
	/// `RequirementType`
	template<template<typename> typename ConditionType, typename RequirementType, typename List>
	inline static constexpr bool for_all_types_v
		= for_all_types<ConditionType, RequirementType, List>::value;

	namespace detail {
		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename... Types>
		struct for_all_params_impl : std::true_type { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename Head,
				 typename... Types>
		struct for_all_params_impl<ConditionType, RequirementType, Evaluatee, Head, Types...>
			: std::conditional_t<
				  !(std::is_same_v<
						ConditionType<Evaluatee, Head>,
						RequirementType> || ConditionType<Evaluatee, Head>::value == RequirementType::value),
				  std::false_type,
				  for_all_params_impl<ConditionType, RequirementType, Evaluatee, Types...>> { };

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee>
		struct for_all_params_impl<ConditionType, RequirementType, Evaluatee> : std::true_type { };

		static_assert(
			for_all_params_impl<std::is_constructible, std::true_type, umax, u8, u32, u64>::value,
			"mpl::for_all_params implementation failing");
		static_assert(!for_all_params_impl<std::is_constructible,
										   std::true_type,
										   umax,
										   u8,
										   std::string,
										   u64>::value,
					  "mpl::for_all_params implementation failing");

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 typename List>
		struct for_all_params_list_impl;

		// clang-format off

		template<template<typename, typename...> typename ConditionType,
				 typename RequirementType,
				 typename Evaluatee,
				 template<typename...>
				 typename List,
				 typename... Types>
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct for_all_params_list_impl<ConditionType, RequirementType, Evaluatee, List<Types...>>
			: for_all_params_impl<ConditionType, RequirementType, Evaluatee, Types...> { };
		// clang-format on
	} // namespace detail

	/// @brief Type Trait to determine that every type "Type" in the `mpl::list`, `List`, satisfies
	/// that the value of `ConditionType<Evaluatee, Type>` equals the value of `RequirementType`,
	/// where `Evaluatee` is a type that can perform some operation on "Type" For example:
	//
	/// 	for_all_params<std::is_constructible, std::true_type, umax, mpl::list<u8, u32, u64>>
	///
	/// will check that `std::is_constructible<umax, Args>::value` is true for `Args` equal to each
	/// of `u8`, `u32`, `u64`
	///
	/// @tparam ConditionType - The condition to check for each
	/// combination of `<Evaluatee, Type>`
	/// @tparam RequirementType - The required value of `ConditionType`
	/// @tparam Evaluatee - The type that should perform some operator on "Type"
	/// @tparam List - The `mpl::list` of each "Type" to check `ConditionType` for
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename Evaluatee,
			 typename List>
	struct for_all_params
		: detail::for_all_params_list_impl<ConditionType, RequirementType, Evaluatee, List> { };

	/// @brief Value of Type Trait `for_all_params`. Used to determine that every type "Type" in
	/// the `mpl::list`, `List`, satisfies that the value of `ConditionType<Evaluatee, Type>` equals
	/// the value of `RequirementType`, where `Evaluatee` is a type that can perform some operation
	/// on "Type". For example:
	//
	/// 	for_all_params_v<std::is_constructible, std::true_type, umax, mpl::list<u8, u32, u64>>
	///
	/// will check that `std::is_constructible<umax, Args>::value` is true for `Args` equal to each
	/// of `u8`, `u32`, `u64`
	template<template<typename, typename...> typename ConditionType,
			 typename RequirementType,
			 typename Evaluatee,
			 typename List>
	inline static constexpr bool for_all_params_v
		= for_all_params<ConditionType, RequirementType, Evaluatee, List>::value;
} // namespace hyperion::utils::mpl
