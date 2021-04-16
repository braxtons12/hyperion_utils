/// @brief Collection of basic meta-programming functions for querying a list of types
#pragma once

#include "../BasicTypes.h"

namespace hyperion::utils::mpl {

	template<typename... T>
	struct list { };

	namespace detail {
		template<typename T, typename... Types>
		struct contains_impl : std::true_type { };

		template<typename T, typename Head, typename... Types>
		struct contains_impl<T, Head, Types...> : std::conditional_t<std::is_same_v<T, Head>,
																	 std::true_type,
																	 contains_impl<T, Types...>> {
		};

		template<typename T>
		struct contains_impl<T> : std::false_type { };

		static_assert(contains_impl<u8, u16, u8, u64>::value,
					  "mpl::contains implementation failing");
		static_assert(!contains_impl<u32, u16, u8, u64>::value,
					  "mpl::contains implementation failing");

		template<typename T, typename List>
		struct contains_list_impl;

		// clang-format off

		template<typename T, template<typename...> typename List, typename... Types>
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct contains_list_impl<T, List<Types...>> : contains_impl<T, Types...> { };

		// clang-format on
	} // namespace detail

	/// @brief Used to determine if the given `mpl::list`, `List`, contains the type `T`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	template<typename T, typename List>
	struct contains : detail::contains_list_impl<T, List> { };

	/// @brief Value of `mpl::contains`. Used to determine if the given `mpl::list`, `List`,
	/// contains the given type `T`
	template<typename T, typename List>
	static inline constexpr auto contains_v = contains<T, List>::value;

	namespace detail {
		struct not_found { };

		template<usize I, usize Current, usize Size, typename... Types>
		struct at_impl {
			static_assert(I < Size);
		};

		template<usize I, usize Current, usize Size, typename Head, typename... Types>
		struct at_impl<I, Current, Size, Head, Types...> {
			static_assert(I < Size);
#ifdef HYPERION_HAS_TYPE_PACK_ELEMENT
			using type = __type_pack_element<I, Head, Types...>;
#else
			using type = std::conditional_t<I == Current,
											Head,
											typename at_impl<I, Current + 1, Size, Types...>::type>;
#endif
		};

		/// we should never reach this case, as the static_assert in the other specializations
		/// should trigger before hand
		template<usize I, usize Current, usize Size>
		struct at_impl<I, Size, Current> {
			static_assert(I < Size);
			using type = not_found;
		};

		static_assert(std::is_same_v<u8, at_impl<0, 0, 4, u8, u16, u32, u64>::type>,
					  "mpl::at implemenation failing");
		static_assert(std::is_same_v<u16, at_impl<1, 0, 4, u8, u16, u32, u64>::type>,
					  "mpl::at implementation failing");
		static_assert(std::is_same_v<u32, at_impl<2, 0, 4, u8, u16, u32, u64>::type>,
					  "mpl::at implementation failing");
		static_assert(std::is_same_v<u64, at_impl<3, 0, 4, u8, u16, u32, u64>::type>,
					  "mpl::at implementation failing");

		template<usize I, typename List>
		struct at_list_impl;

		// clang-format off

		template<usize I, template<typename...> typename List, typename... Types>
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct at_list_impl<I, List<Types...>> : at_impl<I, 0, sizeof...(Types), Types...> { };

		// clang-format on
	} // namespace detail

	/// @brief Used to find the `I`th type in the `mpl::list`, `List`
	///
	/// @tparam I - The index in the parameter pack to get the type of
	/// @tparam List - The `mpl::list` to search in
	template<usize I, typename List>
	struct at : detail::at_list_impl<I, List> { };

	/// @brief Alias to `mpl::at<I, List>::type`. Used to get the `I`th type in the `mpl::list`,
	/// `List`
	///
	/// @tparam I - The index in the parameter pack to get the type of
	/// @tparam List - The `mpl::list` to search in
	template<usize I, typename List>
	using at_t = typename at<I, List>::type;

	static_assert(std::is_same_v<u32, at_t<2, list<u8, u16, u32, u64>>>,
				  "mpl::at implementation failing");
	static_assert(std::is_same_v<u64, at_t<3, list<u8, u16, u32, u64>>>,
				  "mpl::at implementation failing");

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
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct index_of_list_impl<T, List<Types...>> : index_of_impl<0, T, Types...> { };

		// clang-format on
	} // namespace detail

	// clang-format off

	/// @brief Used to get the index of the type `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to search for
	/// @tparam List - The `mpl::list` to search in
	template<typename T, typename List>
	requires(contains_v<T, List>)
	struct index_of : detail::index_of_list_impl<T, List> {
	};

	/// @brief Value of `mpl::index_of`. Used to determine the index of the type `T`
	/// in the `mpl::list`, `List`
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
		requires(std::is_same_v<List<Types...>, list<Types...>>)
		struct max_size_list_impl<List<Types...>> : max_size_impl<0, Types...> {
		};

		// clang-format on
	} // namespace detail

	/// @brief Used to determine the size of the largest type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to get the largest type for
	template<typename List>
	struct max_size_of : detail::max_size_list_impl<List> { };

	/// @brief Value of `mpl::max_size_of`. Used to determine the size of the largest type in
	/// the `mpl::list`, `List`
	template<typename List>
	inline static constexpr usize max_size_of_v = max_size_of<List>::value;

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

	/// @brief Used to determine the number of occurences of `T` in the `mpl::list`, `List`
	///
	/// @tparam T - The type to get the number of occurences of
	/// @tparam List - The `mpl::list` to search in
	template<typename T, typename List>
	struct instances_of : detail::instances_of_list_impl<T, List> { };

	/// @brief Value of `instances_of`. Used to determine the number of occurences of `T` in the
	/// `mpl::list`, `List`
	template<typename T, typename List>
	inline static constexpr usize instances_of_v = instances_of<T, List>::value;

	/// @brief Used to get the first type in the `mpl::list`, `List`
	///
	/// @tparam List - The `mpl::list` to search in
	template<typename List>
	struct first : at<0, List> { };

	/// @brief Alias to `mpl::first::type`. Used to get the first type in the `mpl::list`, `List`
	template<typename List>
	using first_t = typename first<List>::type;

	static_assert(std::is_same_v<first_t<list<u8, u16, u32>>, u8>,
				  "mpl::first implementation failing");
	static_assert(std::is_same_v<first_t<list<u32, u16, u8>>, u32>,
				  "mpl::first implementation failing");
} // namespace hyperion::utils::mpl
