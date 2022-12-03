/// @file Enum.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief An alternative to std::variant with improved compile time and runtime performance and
/// additional features
/// @version 0.1
/// @date 2022-12-02
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

#include <Hyperion/HyperionDef.h>
#include <Hyperion/enum/detail.h>
#include <Hyperion/error/Panic.h>

///	@defgroup enum Enum
/// `Enum<Ts...>` is a sum type similar to `std::variant`, but that more closely models the API
/// surface and ergonomics of sum types from other languages, such as Rust's `enum` type category.
/// To achieve this, it provides an API compatible with `std::variant`, including overloads for
/// static functions and meta-programming types in `std`, along with additional functionality that
/// improves ergonomics and usage vs `std::variant`.
///
/// # Example
/// @code {.cpp}
/// using MyEnum = Enum<u32, u64, f32>;
/// auto my_enum = MyEnum(enum_tag<f32>{}, 42.0_f32);
///
/// const auto index = my_enum.index();
/// HYPERION_ASSERT(index == 2_usize, "");
/// const auto is_variant = my_enum.is_variant(2_usize);
/// HYPERION_ASSERT(is_variant, "");
/// const auto is_variant_templated = my_enum.is_variant<2_usize>();
/// HYPERION_ASSERT(is_variant_templated, "");
/// const auto is_variant_type = my_enum.is_variant<f32>();
/// HYPERION_ASSERT(is_variant_type, "");
/// const auto* val = my_enum.get_if<2_usize>();
/// HYPERION_ASSERT(val != nullptr, "");
/// const auto& val2 = my_enum.get<f32>();
/// HYPERION_ASSERT(val2 == 42.0_f32, "");
///
/// my_enum.match(
///             [](f32 value){ println("enum was an f32"); },
///             [](auto value){ println("enum wasn't an f32"); }
///         );
///
///	enum_inspect(my_enum) {
///		enum_variant(f32, const auto& value) {
///		    HYPERION_ASSERT(value == 42.0_f32, "");
///		    println("enum was an f32");
///		}
///		any_variant() {
///		    println("enum wasn't an f32");
///		}
///	}
///
///	my_enum = 24_u64;
///
/// my_enum.match(
///             [](u64 value){ println("enum was a u64"); },
///             [](auto value){ println("enum wasn't a u64"); }
///         );
///
///	enum_inspect(my_enum) {
///		enum_variant(u64, const auto& value) {
///		    HYPERION_ASSERT(value == 24_u64, "");
///		    println("enum was a u64");
///		}
///		any_variant() {
///		    println("enum wasn't a u64");
///		}
///	}
///
///	const auto* val3 = std::get_if<u64>(my_enum);
///	HYPERION_ASSERT(val3 != nullptr, "");
///	HYPERION_ASSERT(*val3 == 24_u64, "");
///
/// @endcode
/// @headerfile "Hyperion/Enum.h"

namespace hyperion {

	/// @brief Overload wraps a set of callables into a single callable type suitable for passing to
	/// `std::visit` or `Enum<Ts...>::match`
	///
	/// # Example
	/// @code {.cpp}
	/// using MyEnum = Enum<u32, u64, f32>;
	/// auto my_enum = MyEnum(enum_tag<f32>{}, 42.0_f32);
	/// my_enum.match(Overload{
	///                 [](f32 value){ println("enum was an f32"); },
	///                 [](auto value){ println("enum wasn't an f32"); }
	///             });
	/// @endcode
	///
	/// @tparam Callables The types of the callables this `Overload` wraps
	///
	/// @ingroup enum
	/// @headerfile "Hyperion/Enum.h"
	template<typename... Callables>
	struct Overload : Callables... {
		using Callables::operator()...;
	};
	template<typename... Callables>
	Overload(Callables...) -> Overload<Callables...>;

	/// @brief `Enum<Ts...>` is a sum type (a type-safe tagged union) similar to `std::variant`.
	///
	/// `Enum<Ts...>` is a sum type (a type-safe tagged union) similar to `std::variant`,
	/// but that more closely models the API surface and ergonomics of sum types from other
	/// languages, such as Rust's `enum` type category. To achieve this, it provides an API
	/// compatible with `std::variant`, including overloads for static functions and
	/// meta-programming types in `std`, along with additional functionality that improves
	/// ergonomics and usage vs `std::variant`.
	///
	/// # Example
	/// @code {.cpp}
	/// using MyEnum = Enum<u32, u64, f32>;
	/// auto my_enum = MyEnum(enum_tag<f32>{}, 42.0_f32);
	///
	/// const auto index = my_enum.index();
	/// HYPERION_ASSERT(index == 2_usize, "");
	/// const auto is_variant = my_enum.is_variant(2_usize);
	/// HYPERION_ASSERT(is_variant, "");
	/// const auto is_variant_templated = my_enum.is_variant<2_usize>();
	/// HYPERION_ASSERT(is_variant_templated, "");
	/// const auto is_variant_type = my_enum.is_variant<f32>();
	/// HYPERION_ASSERT(is_variant_type, "");
	/// const auto* val = my_enum.get_if<2_usize>();
	/// HYPERION_ASSERT(val != nullptr, "");
	/// const auto& val2 = my_enum.get<f32>();
	/// HYPERION_ASSERT(val2 == 42.0_f32, "");
	///
	/// my_enum.match(
	///             [](f32 value){ println("enum was an f32"); },
	///             [](auto value){ println("enum wasn't an f32"); }
	///         );
	///
	///	enum_inspect(my_enum) {
	///		enum_variant(f32, const auto& value) {
	///		    HYPERION_ASSERT(value == 42.0_f32, "");
	///		    println("enum was an f32");
	///		}
	///		any_variant() {
	///		    println("enum wasn't an f32");
	///		}
	///	}
	///
	///	my_enum = 24_u64;
	///
	/// my_enum.match(
	///             [](u64 value){ println("enum was a u64"); },
	///             [](auto value){ println("enum wasn't a u64"); }
	///         );
	///
	///	enum_inspect(my_enum) {
	///		enum_variant(u64, const auto& value) {
	///		    HYPERION_ASSERT(value == 24_u64, "");
	///		    println("enum was a u64");
	///		}
	///		any_variant() {
	///		    println("enum wasn't a u64");
	///		}
	///	}
	///
	///	const auto* val3 = std::get_if<u64>(my_enum);
	///	HYPERION_ASSERT(val3 != nullptr, "");
	///	HYPERION_ASSERT(*val3 == 24_u64, "");
	///
	/// @endcode
	///
	/// @tparam Types The list of types that can be stored in the Enum
	/// @ingroup enum
	/// @headerfile "Hyperion/Enum.h"
	template<typename... Types>
	class Enum : private detail::EnumUnion<0_usize, Types...> {
	  private:
		/// @brief internal storage management type
		using Union = detail::EnumUnion<0_usize, Types...>;
		/// @brief Contains meta-functions and static members useful for the implementation
		using tags = typename Union::tags;

		/// @brief Forms the lvalue reference type corresponding to the type T
		/// @tparam T  The type to form the lvalue reference type of
		template<typename T>
		using reference = std::add_lvalue_reference_t<T>;

		/// @brief Forms the const lvalue reference type corresponding to the type T
		/// @tparam T  The type to form the const lvalue reference type of
		template<typename T>
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>;

		/// @brief Forms the rvalue reference type corresponding to the type T
		/// @tparam T  The type to form the rvalue reference type of
		template<typename T>
		using rvalue_reference = std::add_rvalue_reference_t<T>;

		/// @brief Forms the const rvalue reference type corresponding to the type T
		/// @tparam T  The type to form the const rvalue reference type of
		template<typename T>
		using const_rvalue_reference = std::add_rvalue_reference_t<std::add_const_t<T>>;

		/// @brief The type of the `Enum` variant at index `Index + 1`
		/// @tparam Index  The current `Enum` variant index
		template<usize Index>
		using next_variant = typename tags::template next_variant<Index>;

	  public:
		/// @brief The type list containing the variant types of this `Enum` instantiation
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		using list = typename tags::list;
		/// @brief The size type used to store the variant index
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		using size_type = typename tags::size_type;
		/// @brief The size of the `list`. The number of `Enum` variants in this instantiation
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		static constexpr auto SIZE = static_cast<size_type>(tags::SIZE);

		/// @brief The type of the `Enum` variant at index `Index`
		/// @tparam Index  The variant index to get the corresponding type for
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize Index, std::enable_if_t<(Index < SIZE), bool> = true>
		using variant = typename tags::template variant<Index>;

		/// @brief The variant index associated with the given type.
		/// If T occurs multiple times in `list`, this will be the index corresponding with the
		/// first occurrence of `T`
		/// @tparam T  The type to get the variant index of
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
		static inline constexpr auto variant_index = mpl::index_of_v<T, list>;

	  private:
		/// @brief The index of the current variant type stored in the `Enum`
		size_type m_current_index = SIZE;

		/// @brief A version of `list`, but with all elements `const`
		using list_as_const = mpl::apply_to_list<std::add_const_t, list>;

		/// @brief A version of `list`, but with all elements as lvalue references
		using list_as_ref = mpl::apply_to_list<std::add_lvalue_reference_t, list>;

		/// @brief A version of `list`, but with all elements as const lvalue references
		using list_as_const_ref = mpl::apply_to_list<std::add_lvalue_reference_t, list_as_const>;

		/// @brief Checks if the `Enum` variant at `Index` can be constructed from `Args...`
		/// @tparam Index  The variant index to check
		/// @tparam Args  The types of the arguments to pass to the variant at `Index`'s
		/// constructor
		/// @return Whether the `Enum` variant at `Index` is constructible from an argument list of
		/// types `Args...`
		template<usize Index, typename... Args>
		requires(Index < SIZE)
		static inline consteval auto check_constructible() noexcept -> std::pair<bool, usize> {
			using current = variant<Index>;

			constexpr auto constructible = concepts::ConstructibleFrom<current, Args...>;

			if constexpr(constructible) {
				return {true, Index};
			}

			if constexpr(Index + 1_usize < SIZE) {
				using next_variant = variant<Index + 1_usize>;
				if constexpr(!concepts::Same<next_variant, detail::None>) {
					return check_constructible<Index + 1_usize, Args...>();
				}
			}

			return {false, SIZE};
		}

		/// @brief Checks if the `Enum` variant at `Index` can be assigned from `Args`
		/// @tparam Index  The variant index to check
		/// @tparam Arg  The type to assign the variant at `Index` with
		/// @return Whether the `Enum` variant at `Index` is assignable from an `Arg`
		template<usize Index, typename Arg>
		requires(Index < SIZE)
		static inline consteval auto check_assignable() noexcept -> std::pair<bool, usize> {
			using current = variant<Index>;

			constexpr auto assignable = concepts::Assignable<current, Arg>;

			if constexpr(assignable) {
				return {true, Index};
			}

			if constexpr(Index + 1_usize < SIZE) {
				using next_variant = variant<Index + 1_usize>;
				if constexpr(!concepts::Same<next_variant, detail::None>) {
					return check_assignable<Index + 1_usize, Arg>();
				}
			}

			return {false, SIZE};
		}

	  public:
		/// @brief Default constructs an `Enum`
		/// This will default construct an `Enum` as the first variant (ie the variant at index
		/// zero).
		///
		/// # Requirements
		/// * The variant at index zero must be `DefaultConstructible`
		///
		/// # Exception Safety
		/// * `noexcept` as long as the variant at index zero is `NoexceptDefaultConstructible`,
		/// otherwise can throw any exception throwable by the variant at index zero.
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr Enum() noexcept(concepts::NoexceptDefaultConstructible<variant<0>>)
		requires concepts::DefaultConstructible<variant<0>>
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			construct<0_usize, true>();
			set_index(0_usize);
		}

		/// @brief Constructs an `Enum` as the first variant `ConstructibleFrom` `value`
		///
		/// # Requirements
		/// * A variant of `Enum` must be `ConstructibleFrom` `value`
		///
		/// # Exception Safety
		/// * `noexcept` as long as the constructible variant is `NoexceptConstructibleFrom` `value`
		/// otherwise can throw any exception throwable by the constructible variant.
		///
		/// @param value  The value to construct the `Enum` from
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T>
		explicit constexpr Enum(T&& value) // NOLINT(bugprone-forwarding-reference-overload)
			noexcept(mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_constructible,
															 std::true_type,
															 list,
															 mpl::list<T>>)
		requires(check_constructible<0, decltype(value)>().first)
		{
			construct<check_constructible<0, decltype(value)>().second, true>(
				std::forward<T>(value));
		}

		/// @brief Constructs an `Enum` as the variant `T` from the argument list `args...`
		///
		/// # Requirements
		/// * `T` must be a variant of `Enum`
		/// * `T` must be `ConstructibleFrom` `args...`
		///
		/// # Exception Safety
		/// * `noexcept` as long as `T` is `NoexceptConstructible` from `args...` otherwise can
		/// throw any exception throwable by constructing `T` from `args...`.
		///
		/// @param tag  The tag signalling the variant, `T`, to construct the `Enum` as
		/// @param args  The arguments to pass to `T`'s constructor
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T, typename... Args>
		requires(mpl::contains_v<T, list> && concepts::ConstructibleFrom<T, Args...>)
		explicit constexpr Enum([[maybe_unused]] enum_tag<T> tag, Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>) {
			construct<variant_index<T>, true>(std::forward<Args>(args)...);
		}

		/// @brief Copy Constructs an `Enum` from the given one
		///
		/// # Requirements
		/// * All variants of `Enum` must be `CopyConstructible`
		///
		/// # Exception Safety
		/// * `noexcept` as long as all variants of `Enum` are `NoexceptCopyConstructible`,
		/// otherwise can throw any exception throwable by copy constructing one of the `Enum`
		/// variants.
		///
		/// @param value  The `Enum` to copy-construct an `Enum` from
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr Enum(const Enum& value) noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_copy_constructible, std::true_type, list>)
		requires mpl::all_types_satisfy_v<std::is_copy_constructible, std::true_type, list>
		{
			if(!value.is_valueless()) {
				const auto idx = value.get_index();
				mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
					this->template construct<
						check_constructible<0_usize, decltype(value.get_with_index(index))>()
							.second,
						true>(value.get_with_index(index));
				});
			}
		}

		/// @brief Move Constructs an `Enum` from the given one
		///
		/// # Requirements
		/// * All variants of `Enum` must be `MoveConstructible`
		///
		/// # Exception Safety
		/// * `noexcept` as long as all variants of `Enum` are `NoexceptMoveConstructible`,
		/// otherwise can throw any exception throwable by move constructing one of the `Enum`
		/// variants.
		///
		/// @param value  The `Enum` to move-construct this `Enum` from
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr Enum(Enum&& value) noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_move_constructible, std::true_type, list>)
		requires mpl::all_types_satisfy_v<std::is_move_constructible, std::true_type, list>
		{
			if(!value.is_valueless()) {
				const auto idx = value.get_index();
				mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
					this->template construct<
						check_constructible<0_usize,
											decltype(std::move(value).get_with_index(index))>()
							.second,
						true>(std::move(value).get_with_index(index));
				});
			}
		}

// work around for clang not-yet supporting multiple destructors
#if HYPERION_PLATFORM_COMPILER_CLANG
		/// @brief Destructor
		///
		/// # Triviality
		/// * Trivial if all variants of `Enum` are `TriviallyDestructible`
		///
		/// # Exception Safety
		/// * `noexcept` as long as all variants of `Enum` are `NoexceptDestructible`,
		/// otherwise can throw any exception throwable by destructing one of the `Enum`
		/// variants.
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr ~Enum() noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>) {
			if constexpr(mpl::any_type_satisfies_v<std::is_trivially_destructible,
												   std::false_type,
												   list>)
			{
				destruct(get_index());
			}
		}
#else
		/// @brief Destructor
		///
		/// # Triviality
		/// * Trivial if all variants of `Enum` are `TriviallyDestructible`
		///
		/// # Exception Safety
		/// * `noexcept` as long as all variants of `Enum` are `NoexceptDestructible`,
		/// otherwise can throw any exception throwable by destructing one of the `Enum`
		/// variants.
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr ~Enum() noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
		requires(mpl::any_type_satisfies_v<std::is_trivially_destructible, std::false_type, list>)
		{
			destruct(get_index());
		}
		constexpr ~Enum() noexcept = default;
#endif
		// clang-format off

		/// @brief Copy-assigns this `Enum` from the given one
		///
		/// # Requirements
		/// * All variants of `Enum` must be `CopyAssignable`
		///
		/// # Exception Safety
		/// * `noexcept` as long as:
		///     - The all variants of this `Enum` are `NoexceptCopyAssignable` from `value`,
		///     otherwise can throw any exception throwable by copy-assigning one of the `Enum`
		///     variants.
		///     - All variants of this `Enum` are `NoexceptDestructible`
		/// * If the `noexcept` requirements are not met and an exception is thrown in this
		/// operator, then this `Enum` will be in the "valueless-by-exception" state
		///
		/// @param value  The `Enum` to copy-assign this `Enum` from
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr auto operator=(const Enum& value)
            noexcept(
			    mpl::all_types_satisfy_v<std::is_nothrow_copy_constructible, std::true_type, list>
                && mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
			-> Enum&
		requires mpl::all_types_satisfy_v<std::is_copy_assignable, std::true_type, list>
		{
			// clang-format on
			if(this == &value) {
				return *this;
			}

			const auto idx = value.get_index();
			mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
				this->template assign<
					check_assignable<0_usize, decltype(value.get_with_index(index))>().second,
					true>(value.get_with_index(index));
			});

			return *this;
		}
		// clang-format off

		/// @brief Move-assigns this `Enum` from the given one
		///
		/// # Requirements
		/// * All variants of `Enum` must be `MoveAssignable`
		///
		/// # Exception Safety
		/// * `noexcept` as long as:
		///     - The all variants of this `Enum` are `NoexceptMoveAssignable` from `value`,
		///     otherwise can throw any exception throwable by move-assigning one of the `Enum`
		///     variants.
		///     - All variants of this `Enum` are `NoexceptDestructible`
		/// * If the `noexcept` requirements are not met and an exception is thrown in this
		/// operator, then this `Enum` will be in the "valueless-by-exception" state
		///
		/// @param value  The `Enum` to move-assign this `Enum` from
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		constexpr auto operator=(Enum&& value)
            noexcept(
                mpl::all_types_satisfy_v<std::is_nothrow_move_constructible, std::true_type, list>
                && mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
			-> Enum&
        requires mpl::all_types_satisfy_v<std::is_move_constructible, std::true_type, list>
		{
			// clang-format on
			if(this == &value) {
				return *this;
			}

			const auto idx = value.get_index();
			mpl::call_with_index<SIZE>(idx, [this, val = std::move(value)](auto index) noexcept {
				this->template assign<
					check_assignable<0_usize, decltype(std::move(val).get_with_index(index))>()
						.second,
					true>(std::move(val).get_with_index(index));
			});

			return *this;
		}
		// clang-format off

		/// @brief Assigns `value` to the first `Enum` variant of this `Enum` assignable from
        /// `value`
		///
		/// # Requirements
		/// * A variant of `Enum` must be `Assignable` from `value`
		///
		/// # Exception Safety
		/// * `noexcept` as long as:
		///     - The assignable variant of this `Enum` is `NoexceptAssignable` from `value`,
		///     otherwise can throw any exception throwable by assigning the assignable variant
		///     from `value`
		///     - All variants of this `Enum` are `NoexceptDestructible`
		/// * If the `noexcept` requirements are not met and an exception is thrown in this
		/// operator, then this `Enum` will be in the "valueless-by-exception" state
		///
		/// @param value  The value to assign this `Enum` from
		///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::any_type_satisfies_with_arg_list_v<std::is_assignable,
														 std::true_type,
														 list,
														 mpl::list<T>>
                 && concepts::NotSame<std::remove_cvref_t<T>, Enum>
        constexpr auto operator=(T&& value) // NOLINT
			noexcept(mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_assignable,
															 std::true_type,
															 list,
															 mpl::list<T>>
                    && mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
		    -> Enum&
		{
			// clang-format on
			this->template assign<check_assignable<0_usize, decltype(value)>().second, true>(
				std::forward<T>(value));
			return *this;
		}
		// clang-format off

		/// @brief Re-constructs this `Enum` as the variant at index `N`, from `args...`
		///
		/// # Requirements
        /// * `N` is a valid variant index for this `Enum` (`N < SIZE`)
		/// * The variant at index `N` must be `ConstructibleFrom` `args...`
		///
		/// # Exception Safety
		/// * `noexcept` as long as:
		///     - The variant at index `N` of this `Enum` is `NoexceptConstructibleFrom`` `args...`,
		///     otherwise can throw any exception throwable by constructing the variant at index `N`
		///     from `args...`
		///     - All variants of this `Enum` are `NoexceptDestructible`
		/// * If the `noexcept` requirements are not met and an exception is thrown in this
		/// function, then this `Enum` will be in the "valueless-by-exception" state
        ///
        /// @param args  The arguments to construct the variant at index `N` from
        ///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N, typename... Args>
		requires concepts::ConstructibleFrom<variant<N>, Args...> && (N < SIZE)
		constexpr auto emplace(Args&&... args)
            noexcept(concepts::NoexceptConstructibleFrom<variant<N>, Args...>
                    && mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
			-> void 
        {
			// clang-format on
			if(!is_valueless()) {
				destruct(get_index());
			}

			construct<N, true>(std::forward<Args>(args)...);
		}
		// clang-format off

		/// @brief Re-constructs this `Enum` as the variant `T`, from `args...`
		///
		/// # Requirements
		/// * `T` is a variant of this `Enum`
		/// * `T` is `ConstructibleFrom` `args...`
		///
		/// # Exception Safety
		/// * `noexcept` as long as:
		///     - `T` is `NoexceptConstructibleFrom`` `args...`, otherwise can throw any exception
		///     throwable by constructing a `T` from `args...`
		///     - All variants of this `Enum` are `NoexceptDestructible`
		/// * If the `noexcept` requirements are not met and an exception is thrown in this
		/// function, then this `Enum` will be in the "valueless-by-exception" state
        ///
        /// @param args  The arguments to construct the variant, `T`, from
        ///
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T, typename... Args>
		requires concepts::ConstructibleFrom<T, Args...> && mpl::contains_v<T, list>
        constexpr auto emplace(Args&&... args)
            noexcept(concepts::NoexceptConstructibleFrom<T, Args...>
                    && mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
            -> void
        {
			// clang-format on
			if(!is_valueless()) {
				destruct(get_index());
			}

			construct<variant_index<T>, true>(std::forward<Args>(args)...);
		}

		/// @brief Returns the index associated with the `Enum` variant this `Enum` currently is
		/// @return The index associated with the variant this currently is
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		[[nodiscard]] inline constexpr auto get_index() const noexcept -> size_type {
			return m_current_index;
		}

		/// @brief Returns the index associated with the `Enum` variant this `Enum` currently is
		/// @return The index associated with the variant this currently is
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		[[nodiscard]] inline constexpr auto index() const noexcept -> size_type {
			return m_current_index;
		}

		/// @brief Returns whether this `Enum` is currently the variant associated with `index`
		/// @param index  The index to check
		/// @return Whether this `Enum` is the variant associated with `index`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		[[nodiscard]] inline constexpr auto is_variant(usize index) const noexcept -> bool {
			return m_current_index == index;
		}

		/// @brief Returns whether this `Enum` is currently the variant associated with `Index`
		///
		/// # Requirements
		/// * `Index` must be a valid variant index (`Index < SIZE`)
		///
		/// @tparam Index  The index to check
		/// @return Whether this `Enum` is the variant associated with `Index`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize Index>
		requires(Index < SIZE)
		[[nodiscard]] inline constexpr auto is_variant() const noexcept -> bool {
			return m_current_index == Index;
		}
		// clang-format off

		/// @brief Returns whether this `Enum` is currently the variant `T`
		///
		/// # Requirements
		/// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants this `Enum` can take
		///
		/// @tparam T  The variant to check
		/// @return Whether this `Enum` is the variant `T`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list> && (mpl::instances_of_v<T, list> == 1_usize)
				 && (variant_index<T> < SIZE)
        [[nodiscard]] inline constexpr auto is_variant() const noexcept -> bool {
			// clang-format on
			return m_current_index == variant_index<T>;
		}

		/// @brief Returns whether this `Enum` is currently valueless (whether the `Enum` has been
		/// made valueless due to an exception thrown during an assignment or `emplace` call)
		/// @return Whether this `Enum` is valueless-by-exception
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		[[nodiscard]] inline constexpr auto is_valueless() const noexcept -> bool {
			return m_current_index == SIZE;
		}
		// clang-format off

        /// @brief Invokes the callable `function` with the current variant of this `Enum`
        ///
        /// Equivalent to invoking `function` as if in 
        /// `std::forward<F>(function)(this->get<this->index()>())` (ignoring that `this->index()`
        /// is not a constant expression and thus can't be used as a template parameter)
        ///
        /// # Requirements
        /// * `function` must be invokable with every variant of this `Enum`. The easiest ways to
        /// satisfy this are to use a generic lambda or to pass in an `Overload` wrapping multiple
        /// callables.
        /// * The return type of `function` must be the same for every variant of this `Enum`
        ///
        /// # Exception Safety
        /// * `noexcept` if `function` is `noexcept` for every variant of this `Enum`, otherwise
        /// can throw any exception throwable by `function`.
        ///
        /// # Example
        /// @code {.cpp}
        /// using MyEnum = Enum<std::string, u32, std::vector<u8>>;
        /// auto my_enum = MyEnum(enum_tag<std::string>{}, "MyEnum");
        ///
        /// my_enum.match(Overload{
        ///                 [](auto value){ println("Not string"); },
        ///                 [](std::string value){ println("std::string: {}", value); }
        ///              });
        /// @endcode
        ///
        /// @param function  The callable to invoke with the current variant of this `Enum`
        ///
        /// @return The result of invoking function as if in
        /// `std::forward<F>(function)(this->get<this->index()>())`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename F>
		requires(mpl::all_lists_satisfy_for_type_v<std::is_invocable,
											       std::true_type,
											       F,
											       mpl::apply_to_list<mpl::list, list>>
			     || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												      std::true_type,
												      F,
												      mpl::apply_to_list<mpl::list, list_as_const_ref>>)
		[[nodiscard]] inline constexpr auto match(F&& function) const
			noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
													   std::true_type,
													   F,
													   mpl::apply_to_list<mpl::list, list>>)
        {
			// clang-format on
			return mpl::call_with_index<SIZE>(
				get_index(),
				[this, func = std::forward<F>(function)](auto index) mutable noexcept(
					mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
													  std::true_type,
													  F,
													  mpl::apply_to_list<mpl::list, list>>) {
					return std::forward<F>(func)(this->get_with_index(index));
				});
		}

		/// @brief Invokes the callable `function` with the current variant of this `Enum`
		///
		/// Equivalent to invoking `function` as if in
		/// `std::forward<F>(function)(this->get<this->index()>())` (ignoring that `this->index()`
		/// is not a constant expression and thus can't be used as a template parameter)
		///
		/// # Requirements
		/// * `function` must be invokable with every variant of this `Enum`. The easiest ways to
		/// satisfy this are to use a generic lambda or to pass in an `Overload` wrapping multiple
		/// callables.
		/// * The return type of `function` must be the same for every variant of this `Enum`
		///
		/// # Exception Safety
		/// * `noexcept` if `function` is `noexcept` for every variant of this `Enum`, otherwise
		/// can throw any exception throwable by `function`.
		///
		/// # Example
		/// @code {.cpp}
		/// using MyEnum = Enum<std::string, u32, std::vector<u8>>;
		/// auto my_enum = MyEnum(enum_tag<std::string>{}, "MyEnum");
		///
		/// my_enum.match(Overload{
		///                 [](auto value){ println("Not string"); },
		///                 [](std::string value){ println("std::string: {}", value); }
		///              });
		/// @endcode
		///
		/// @param function  The callable to invoke with the current variant of this `Enum`
		///
		/// @return The result of invoking function as if in
		/// `std::forward<F>(function)(this->get<this->index()>())`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename F>
		requires(mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
												   F,
												   mpl::apply_to_list<mpl::list, list>>
				 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
													  std::true_type,
													  F,
													  mpl::apply_to_list<mpl::list, list_as_ref>>)
		[[nodiscard]] inline constexpr auto match(F&& function) noexcept(
			mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
											  std::true_type,
											  F,
											  mpl::apply_to_list<mpl::list, list>>) {
			return mpl::call_with_index<SIZE>(
				get_index(),
				[this, func = std::forward<F>(function)](auto index) mutable noexcept(
					mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
													  std::true_type,
													  F,
													  mpl::apply_to_list<mpl::list, list>>) {
					return std::forward<F>(func)(this->get_with_index(index));
				});
		}

	  private:
		/// @brief Converts `list` into a list of single-element lists, each containing the
		/// variant contained in `list` at that respective index
		template<typename T>
		using make_list_of_lists = mpl::apply_to_list<mpl::list, T>;

	  public:
		/// @brief Invokes the callable from the list of callables, `functions...`, most
		/// specifically invokable with the current variant of this `Enum`.
		///
		/// By most specific, it is meant that a callable whose parameter most stringently matches
		/// the current variant of this `Enum` will be the one invoked. For example, if this `Enum`
		/// is an instance of `Enum<u32, f32, i32>`, this `Enum` is currently the `f32` variant,
		/// and `functions...` consists of one lambda taking a generic parameter
		/// (ie `[](auto value){ ... }`) and one taking an `f32` (ie `[](f32 value){ ... }`),
		/// the lambda taking an `f32` will be chosen.
		///
		/// Equivalent to invoking the chosen callable, `callable`, as if in
		/// `std::forward<F>(callable)(this->get<this->index()>())` (ignoring that `this->index()`
		/// is not a constant expression and thus can't be used as a template parameter)
		///
		/// Equivalent to calling the single-argument version of `match` with an `Overload` wrapping
		/// the list of callables in `functions...`, ie:
		/// `this->match(Overload{std::forward<F>(functions)...})`
		///
		/// # Requirements
		/// * `functions...` must contain callables that collectively can be invoked with every
		/// variant of this `Enum`.
		/// * The return type of each callable in `functions...` must be the same.
		///
		/// # Exception Safety
		/// * `noexcept` if every callable in `functions...` is `noexcept` when invoked with its
		/// associated `Enum` variant(s), otherwise can throw any exception throwable by the
		/// callables in `functions...`.
		///
		/// # Example
		/// @code {.cpp}
		/// using MyEnum = Enum<std::string, u32, std::vector<u8>>;
		/// auto my_enum = MyEnum(enum_tag<std::string>{}, "MyEnum");
		///
		/// my_enum.match([](auto value){ println("Not string"); },
		///               [](std::string value){ println("std::string: {}", value); });
		/// @endcode
		///
		/// @param functions  The callables to invoke with the current variant of this `Enum`
		///
		/// @return The result of invoking the chsoen callable as if in
		/// `std::forward<F>(callable)(this->get<this->index()>())`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename... F>
		requires(mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
												   decltype(Overload{std::declval<F>()...}),
												   make_list_of_lists<list>>
				 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
													  std::true_type,
													  decltype(Overload{std::declval<F>()...}),
													  make_list_of_lists<list_as_const_ref>>)
		[[nodiscard]] inline constexpr auto match(F&&... functions) const
			noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
													   std::true_type,
													   decltype(Overload{std::declval<F>()...}),
													   mpl::apply_to_list<mpl::list, list>>) {
			return mpl::call_with_index<SIZE>(
				get_index(),
				[this, overload = Overload{std::forward<F>(functions)...}](
					auto index) mutable noexcept(mpl::
													 all_lists_satisfy_for_type_v<
														 std::is_nothrow_invocable,
														 std::true_type,
														 decltype(Overload{std::declval<F>()...}),
														 mpl::apply_to_list<mpl::list, list>>) {
					return std::move(overload)(this->get_with_index(index));
				});
		}

		/// @brief Invokes the callable from the list of callables, `functions...`, most
		/// specifically invokable with the current variant of this `Enum`.
		///
		/// By most specific, it is meant that a callable whose parameter most stringently matches
		/// the current variant of this `Enum` will be the one invoked. For example, if this `Enum`
		/// is an instance of `Enum<u32, f32, i32>`, this `Enum` is currently the `f32` variant,
		/// and `functions...` consists of one lambda taking a generic parameter
		/// (ie `[](auto value){ ... }`) and one taking an `f32` (ie `[](f32 value){ ... }`),
		/// the lambda taking an `f32` will be chosen.
		///
		/// Equivalent to invoking the chosen callable, `callable`, as if in
		/// `std::forward<F>(callable)(this->get<this->index()>())` (ignoring that `this->index()`
		/// is not a constant expression and thus can't be used as a template parameter)
		///
		/// Equivalent to calling the single-argument version of `match` with an `Overload` wrapping
		/// the list of callables in `functions...`, ie:
		/// `this->match(Overload{std::forward<F>(functions)...})`
		///
		/// # Requirements
		/// * `functions...` must contain callables that collectively can be invoked with every
		/// variant of this `Enum`.
		/// * The return type of each callable in `functions...` must be the same.
		///
		/// # Exception Safety
		/// * `noexcept` if every callable in `functions...` is `noexcept` when invoked with its
		/// associated `Enum` variant(s), otherwise can throw any exception throwable by the
		/// callables in `functions...`.
		///
		/// # Example
		/// @code {.cpp}
		/// using MyEnum = Enum<std::string, u32, std::vector<u8>>;
		/// auto my_enum = MyEnum(enum_tag<std::string>{}, "MyEnum");
		///
		/// my_enum.match([](auto value){ println("Not string"); },
		///               [](std::string value){ println("std::string: {}", value); });
		/// @endcode
		///
		/// @param functions  The callables to invoke with the current variant of this `Enum`
		///
		/// @return The result of invoking the chsoen callable as if in
		/// `std::forward<F>(callable)(this->get<this->index()>())`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<typename... F>
		requires(mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
												   decltype(Overload{std::declval<F>()...}),
												   mpl::apply_to_list<mpl::list, list>>
				 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
													  std::true_type,
													  decltype(Overload{std::declval<F>()...}),
													  mpl::apply_to_list<mpl::list, list_as_ref>>)
		[[nodiscard]] inline constexpr auto match(F&&... functions) noexcept(
			mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
											  std::true_type,
											  decltype(Overload{std::declval<F>()...}),
											  mpl::apply_to_list<mpl::list, list>>) {
			return mpl::call_with_index<SIZE>(
				get_index(),
				[this, overload = Overload{std::forward<F>(functions)...}](
					auto index) mutable noexcept(mpl::
													 all_lists_satisfy_for_type_v<
														 std::is_nothrow_invocable,
														 std::true_type,
														 decltype(Overload{std::declval<F>()...}),
														 mpl::apply_to_list<mpl::list, list>>) {
					return std::move(overload)(this->get_with_index(index));
				});
		}

		/// @brief Returns a reference to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a reference to it, otherwise
		/// invokes a `panic`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A reference to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get() & noexcept -> reference<variant<N>> {
			if(!is_variant<N>()) {
				panic("get<{}>() called on variant when it currently holds variant {}",
					  N,
					  get_index());
			}

			return get_impl<N>(*this);
		}

		/// @brief Returns a reference to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a reference to it, otherwise
		/// invokes a `panic`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A reference to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get() const& noexcept -> const_reference<variant<N>> {
			if(!is_variant<N>()) {
				panic("get<{}>() called on variant when it currently holds variant {}",
					  N,
					  get_index());
			}

			return get_impl<N>(*this);
		}

		/// @brief Returns a reference to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a reference to it, otherwise
		/// invokes a `panic`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A reference to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get() && noexcept -> rvalue_reference<variant<N>> {
			if(!is_variant<N>()) {
				panic("get<{}>() called on variant when it currently holds variant {}",
					  N,
					  get_index());
			}

			return get_impl<N>(std::move(*this));
		}

		/// @brief Returns a reference to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a reference to it, otherwise
		/// invokes a `panic`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A reference to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get() const&& noexcept
			-> const_rvalue_reference<variant<N>> {
			if(!is_variant<N>()) {
				panic("get<{}>() called on variant when it currently holds variant {}",
					  N,
					  get_index());
			}

			return get_impl<N>(std::move(*this));
		}
		// clang-format off

        /// @brief Returns a reference to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a reference to it, otherwise
        /// invokes a `panic`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A reference to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
        [[nodiscard]] inline constexpr auto get() & noexcept -> reference<T> {
			// clang-format on
			if(!is_variant<T>()) {
				panic("get<T>() called on variant for T equal to index {} when it currently holds "
					  "variant {}",
					  variant_index<T>,
					  get_index());
			}

			return get_impl<T>(*this);
		}
		// clang-format off

        /// @brief Returns a reference to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a reference to it, otherwise
        /// invokes a `panic`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A reference to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
		[[nodiscard]] inline constexpr auto get() const& noexcept -> const_reference<T> {
			// clang-format on
			if(!is_variant<T>()) {
				panic("get<T>() called on variant for T equal to index {} when it currently holds "
					  "variant {}",
					  variant_index<T>,
					  get_index());
			}

			return get_impl<T>(*this);
		}
		// clang-format off

        /// @brief Returns a reference to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a reference to it, otherwise
        /// invokes a `panic`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A reference to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
		[[nodiscard]] inline constexpr auto get() && noexcept -> rvalue_reference<T> {
			// clang-format on
			if(!is_variant<T>()) {
				panic("get<T>() called on variant for T equal to index {} when it currently holds "
					  "variant {}",
					  variant_index<T>,
					  get_index());
			}

			return get_impl<T>(std::move(*this));
		}
		// clang-format off

        /// @brief Returns a reference to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a reference to it, otherwise
        /// invokes a `panic`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A reference to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
		[[nodiscard]] inline constexpr auto get() const&& noexcept -> const_rvalue_reference<T> {
			// clang-format on
			if(!is_variant<T>()) {
				panic("get<T>() called on variant for T equal to index {} when it currently holds "
					  "variant {}",
					  variant_index<T>,
					  get_index());
			}

			return get_impl<T>(std::move(*this));
		}

		/// @brief Returns a pointer to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a ointer to it, otherwise
		/// returns `nullptr`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A pointer to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get_if() noexcept -> std::add_pointer_t<variant<N>> {
			if(!is_variant<N>()) {
				return nullptr;
			}

			return &(get_impl<N>(*this));
		}

		/// @brief Returns a pointer to the variant corresponding with index `N`
		///
		/// If this is currently the variant at index `N`, returns a ointer to it, otherwise
		/// returns `nullptr`
		///
		/// # Requirements
		/// * `N` must be a valid variant index (`N < SIZE`)
		///
		/// @return A pointer to the variant at index `N`
		/// @ingroup enum
		/// @headerfile "Hyperion/Enum.h"
		template<usize N>
		requires(N < SIZE)
		[[nodiscard]] inline constexpr auto get_if() const noexcept
			-> std::add_pointer_t<std::add_const_t<variant<N>>> {
			if(!is_variant<N>()) {
				return nullptr;
			}

			return &(get_impl<N>(*this));
		}
		// clang-format off

        /// @brief Returns a pointer to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a pointer to it, otherwise returns
        /// `nullptr`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A pointer to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
		[[nodiscard]] inline constexpr auto get_if() noexcept -> std::add_pointer_t<T> {
			// clang-format on
			if(!is_variant<T>()) {
				return nullptr;
			}

			return &(get_impl<T>(*this));
		}
		// clang-format off

        /// @brief Returns a pointer to the variant `T`
        ///
        /// If this is currently the variant `T`, returns a pointer to it, otherwise returns
        /// `nullptr`
        ///
        /// # Requirements
        /// * `T` must be a variant of this `Enum`
        /// * `T` must only occur once in the list of possible variants of this `Enum`
        ///
        /// @return A pointer to the variant `T`
        /// @ingroup enum
        /// @headerfile "Hyperion/Enum.h"
		template<typename T>
		requires mpl::contains_v<T, list>
				 && (mpl::instances_of_v<T, list> == 1_usize)
		[[nodiscard]] inline constexpr auto get_if() const noexcept
            -> std::add_pointer_t<std::add_const_t<T>> {
			// clang-format on
			if(!is_variant<T>()) {
				return nullptr;
			}

			return &(get_impl<T>(*this));
		}

	  private:
		inline constexpr auto set_index(size_type index) noexcept -> void {
			m_current_index = index;
		}

		/// @brief Returns `T` with the same ref-qualification as `U`
		template<typename T, typename U>
		using get_correct_ref_qual = std::conditional_t<std::is_rvalue_reference_v<U>,
														std::add_rvalue_reference_t<T>,
														std::add_lvalue_reference_t<T>>;

		/// @brief Returns `T` with the same `const` and ref qualification as `U`
		template<typename T, typename U>
		using get_ret = std::conditional_t<std::is_const_v<std::remove_reference_t<U>>,
										   get_correct_ref_qual<std::add_const_t<T>, U>,
										   get_correct_ref_qual<T, U>>;

		template<usize N, typename Union>
		[[nodiscard]] static inline constexpr auto
		get_impl(Union&& self) noexcept -> get_ret<variant<N>, decltype(self)> {
			if constexpr(SIZE <= 5_usize) {
				if constexpr(N == 0_usize) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_first;
				}
				else if constexpr(N == 1_usize) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_second;
				}
				else if constexpr(N == 2_usize) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_third;
				}
				else if constexpr(N == 3_usize) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_fourth;
				}
				else if constexpr(N == 4_usize) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_fifth;
				}
			}
			else {
				if constexpr(N == Enum::current_index) {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return std::forward<Union>(self).m_current;
				}
				else {
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
					return get_impl(std::forward<Union>(self).m_next);
					// return std::forward<Union>(self).m_next.template get<N>();
				}
			}
		}

		template<typename T, typename Union>
		requires mpl::contains_v<T, list>
		[[nodiscard]] static inline constexpr auto
		get_impl(Union&& self) noexcept -> get_ret<T, decltype(self)> {
			constexpr auto Index = mpl::index_of_v<T, list>;
			return get_impl<Index>(std::forward<Union>(self));
		}

		template<usize N>
		[[nodiscard]] inline constexpr auto
		get_with_index([[maybe_unused]] mpl::index<N> index) & noexcept -> reference<variant<N>> {
			return this->template get<N>();
		}

		template<usize N>
		[[nodiscard]] inline constexpr auto
		get_with_index([[maybe_unused]] mpl::index<N> index) const& noexcept
			-> const_reference<variant<N>> {
			return this->template get<N>();
		}

		template<usize N>
		[[nodiscard]] inline constexpr auto
		get_with_index([[maybe_unused]] mpl::index<N> index) && noexcept
			-> rvalue_reference<variant<N>> {
			return std::move(*this).template get<N>();
		}

		template<usize N>
		[[nodiscard]] inline constexpr auto
		get_with_index([[maybe_unused]] mpl::index<N> index) const&& noexcept
			-> const_rvalue_reference<variant<N>> {
			return std::move(*this).template get<N>();
		}

		template<usize Index>
		struct do_destruct {
			static constexpr bool value = false;
		};

		template<usize Index>
		requires(Index < SIZE)
		struct do_destruct<Index> {
			static constexpr bool value
				= (std::is_class_v<variant<Index>>
				   || (std::is_union_v<variant<Index>> && concepts::Destructible<variant<Index>>))
				  && !std::is_trivially_destructible_v<variant<Index>>;
		};

		template<usize Index>
		static constexpr bool do_destruct_v = do_destruct<Index>::value;

		template<usize Index>
		inline constexpr auto
		destruct_with_index([[maybe_unused]] mpl::index<Index> index) noexcept(
			concepts::NoexceptDestructible<variant<Index>>) {
			if constexpr(do_destruct_v<Index>) {
				get_impl<Index>(*this).~variant<Index>();
			}
		}

		inline constexpr auto
		// NOLINTNEXTLINE(readability-function-cognitive-complexity
		destruct(usize current_index) noexcept(concepts::AllNoexceptDestructible<list>) -> void {
			if(!is_valueless()) {
				if constexpr(SIZE <= 5_usize) {
					switch(current_index) {
						case 0_usize:
							if constexpr(do_destruct_v<0_usize>) {
								using variant = variant<0_usize>;
								// NOLINTNEXTLINE
								this->m_first.~variant();
							}
							break;
						case 1_usize:
							if constexpr(do_destruct_v<1_usize>) {
								using variant = variant<1_usize>;
								// NOLINTNEXTLINE
								this->m_second.~variant();
							}
							break;
						case 2_usize:
							if constexpr(do_destruct_v<2_usize>) {
								// NOLINTNEXTLINE
								this->m_third.~variant();
							}
							break;
						case 3_usize:
							if constexpr(do_destruct_v<3_usize>) {
								// NOLINTNEXTLINE
								this->m_fourth.~variant();
							}
							break;
						case 4_usize:
							if constexpr(do_destruct_v<4_usize>) {
								using variant = variant<4_usize>;
								// NOLINTNEXTLINE
								this->m_fifth.~variant();
							}
							break;
						default: HYPERION_UNREACHABLE();
					}
				}
				else {
					mpl::call_with_index(current_index,
										 [this](auto index) { destruct_with_index(index); });
				}
				set_index(SIZE);
			}
		}

		template<usize Index, bool ONLY_IF_THIS_INDEX = false, typename... Args>
		inline constexpr auto construct(Args&&... args) noexcept(
			mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_constructible,
													std::true_type,
													list,
													mpl::list<Args...>>) -> void {
			using current = variant<Index>;

			constexpr auto constructible = concepts::ConstructibleFrom<current, Args...>;
			if constexpr(ONLY_IF_THIS_INDEX) {
				static_assert(constructible);
			}

			if constexpr(constructible) {
				try {
					if(!is_valueless()) {
						destruct(get_index());
					}

					std::construct_at(std::addressof(get_impl<Index>(*this)),
									  std::forward<Args>(args)...);
					set_index(Index);
					return;
				}
				catch([[maybe_unused]] const std::exception& e) {
					set_index(SIZE);
					throw;
				}
				catch(...) {
					set_index(SIZE);
					throw;
				}
			}

			if constexpr(ONLY_IF_THIS_INDEX) {
				// shouldn't be able to happen, per the static assert above, but weirder things have
				// happened
				panic("Attempt to force construction of index {0} of an Enum with a set of "
					  "arguments that the type at index {0} cannot be constructed from",
					  Index);
				throw;
			}

			if constexpr(Index + 1_usize < SIZE) {
				using next_variant = next_variant<Index + 1_usize>;
				if constexpr(!concepts::Same<next_variant, detail::None>) {
					return construct<Index + 1_usize>(std::forward<Args>(args)...);
				}
			}
		}

		template<usize Index, bool ONLY_THIS_INDEX = false, typename U>
		inline constexpr auto assign(U&& to_assign) noexcept(
			mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_assignable,
													std::true_type,
													list,
													mpl::list<U>>) -> void {
			using current = variant<Index>;
			constexpr auto assignable = std::is_assignable_v<current, decltype(to_assign)>;

			if constexpr(ONLY_THIS_INDEX) {
				static_assert(assignable);
			}

			if constexpr(assignable) {
				try {
					const auto assignable_but_this_variant_not_constructed = get_index() != Index;

					if(assignable_but_this_variant_not_constructed) {
						destruct(get_index());

						if constexpr(concepts::ConstructibleFrom<current, decltype(to_assign)>) {
							std::construct_at(std::addressof(get_impl<Index>(*this)),
											  std::forward<U>(to_assign));
							set_index(Index);
							return;
						}
						else if constexpr(concepts::DefaultConstructible<current>) {
							std::construct_at(std::addressof(get_impl<Index>(*this)));
						}
					}

					get_impl<Index>(*this) = std::forward<U>(to_assign);
					set_index(Index);
					return;
				}
				catch([[maybe_unused]] const std::exception& e) {
					set_index(SIZE);
					throw;
				}
				catch(...) {
					set_index(SIZE);
					throw;
				}
			}

			if constexpr(ONLY_THIS_INDEX) {
				// shouldn't be able to happen, per the static assert above, but weirder things have
				// happened
				panic("Attempt to force-assign to index {0} of an Enum with a value that the type "
					  "at index {0} cannot be assigned with",
					  Index);
			}

			if constexpr(Index + 1_usize < SIZE) {
				using next_variant = next_variant<Index + 1_usize>;
				if constexpr(!concepts::Same<next_variant, detail::None>) {
					return assign<Index + 1_usize>(std::forward<U>(to_assign));
				}
			}
		}
	};
} // namespace hyperion

#if !defined(_GLIBCXX_VARIANT) && !defined(_LIBCPP_VARIANT) && !defined(_VARIANT_)
	#include <variant>
#endif

// wire up hyperion::Enum<Types...> to support access like std::variant<Types...>,
// except for std::visit

template<std::size_t I, typename... Ts>
requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_alternative<I, hyperion::Enum<Ts...>> {
	using type = typename hyperion::Enum<Ts...>::template variant<I>;
};

template<std::size_t I, typename... Ts>
requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_alternative<I, const hyperion::Enum<Ts...>> {
	using type = std::add_const_t<typename hyperion::Enum<Ts...>::template variant<I>>;
};

template<std::size_t I, typename... Ts>
requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_alternative<I, volatile hyperion::Enum<Ts...>> {
	using type = std::add_volatile_t<typename hyperion::Enum<Ts...>::template variant<I>>;
};

template<std::size_t I, typename... Ts>
requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_alternative<I, const volatile hyperion::Enum<Ts...>> {
	using type = std::add_cv_t<typename hyperion::Enum<Ts...>::template variant<I>>;
};

template<typename... Ts>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_size<hyperion::Enum<Ts...>> {
	static constexpr auto value = hyperion::Enum<Ts...>::SIZE;
};

template<typename... Ts>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_size<const hyperion::Enum<Ts...>> {
	static constexpr auto value = hyperion::Enum<Ts...>::SIZE;
};

template<typename... Ts>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_size<volatile hyperion::Enum<Ts...>> {
	static constexpr auto value = hyperion::Enum<Ts...>::SIZE;
};

template<typename... Ts>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct std::variant_size<const volatile hyperion::Enum<Ts...>> {
	static constexpr auto value = hyperion::Enum<Ts...>::SIZE;
};

// NOLINTNEXTLINE(cert-dcl58-cpp)
namespace std {
	template<typename T, typename... Ts>
	requires hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto holds_alternative(const hyperion::Enum<Ts...>& _enum) noexcept -> bool {
		return _enum.template is_variant<T>();
	}

	/// @defgroup std_getI std::get
	/// Overloads of `std::get<I>` for `hyperion::Enum`
	/// @headerfile "Hyperion/Enum.h"
	///
	/// @defgroup std_getT std::get
	/// Overloads of `std::get<T>` for `hyperion::Enum`
	/// @headerfile "Hyperion/Enum.h"

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(const hyperion::Enum<Ts...>& _enum)
		-> const std::variant_alternative_t<I, hyperion::Enum<Ts...>>& {
		if(!_enum.template is_variant<I>()) {
			throw std::bad_variant_access();
		}

		return _enum.template get<I>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(const hyperion::Enum<Ts...>&& _enum)
		-> const std::variant_alternative_t<I, hyperion::Enum<Ts...>>&& {
		if(!_enum.template is_variant<I>()) {
			throw std::bad_variant_access();
		}

		return std::move(_enum).template get<I>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(hyperion::Enum<Ts...>& _enum)
		-> std::variant_alternative_t<I, hyperion::Enum<Ts...>>& {
		if(!_enum.template is_variant<I>()) {
			throw std::bad_variant_access();
		}

		return _enum.template get<I>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(hyperion::Enum<Ts...>&& _enum)
		-> std::variant_alternative_t<I, hyperion::Enum<Ts...>>&& {
		if(!_enum.template is_variant<I>()) {
			throw std::bad_variant_access();
		}

		return std::move(_enum).template get<I>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(const hyperion::Enum<Ts...>& _enum) -> const T& {
		if(!_enum.template is_variant<T>()) {
			throw std::bad_variant_access();
		}

		return _enum.template get<T>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(const hyperion::Enum<Ts...>&& _enum) -> const T&& {
		if(!_enum.template is_variant<T>()) {
			throw std::bad_variant_access();
		}

		return std::move(_enum).template get<T>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(hyperion::Enum<Ts...>& _enum) -> T& {
		if(!_enum.template is_variant<T>()) {
			throw std::bad_variant_access();
		}

		return _enum.template get<T>();
	}

	/// @brief overload `std::get` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get(hyperion::Enum<Ts...>&& _enum) -> T&& {
		if(!_enum.template is_variant<T>()) {
			throw std::bad_variant_access();
		}

		return std::move(_enum).template get<T>();
	}

	/// @brief overload `std::get_if` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get_if(const hyperion::Enum<Ts...>* _enum) noexcept
		-> std::add_pointer_t<std::add_const_t<variant_alternative_t<I, hyperion::Enum<Ts...>>>> {
		return _enum->template get_if<I>();
	}

	/// @brief overload `std::get_if` for `hyperion::Enum`
	/// @ingroup std_getI
	template<size_t I, typename... Ts>
	requires(I < std::variant_size_v<hyperion::Enum<Ts...>>)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get_if(hyperion::Enum<Ts...>* _enum) noexcept
		-> std::add_pointer_t<variant_alternative_t<I, hyperion::Enum<Ts...>>> {
		return _enum->template get_if<I>();
	}

	/// @brief overload `std::get_if` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get_if(const hyperion::Enum<Ts...>* _enum) noexcept
		-> std::add_pointer_t<std::add_const_t<T>> {
		return _enum->template get_if<T>();
	}

	/// @brief overload `std::get_if` for `hyperion::Enum`
	/// @ingroup std_getT
	template<typename T, typename... Ts>
	requires(hyperion::mpl::contains_v<T, hyperion::mpl::list<Ts...>>
			 && hyperion::mpl::instances_of_v<T, hyperion::mpl::list<Ts...>> == 1)
	// NOLINTNEXTLINE(cert-dcl58-cpp)
	inline constexpr auto get_if(hyperion::Enum<Ts...>* _enum) noexcept -> std::add_pointer_t<T> {
		return _enum->template get_if<T>();
	}
} // namespace std

namespace hyperion {

/// @brief Does pattern matching on the given `hyperion::Enum`, `_enum`
///
/// Pattern-matches the given `Enum`, potentially extracting the stored value and potentially
/// decomposing that value into its constituent parts. Control flow is similar to that of a `switch`
/// statement, but does not allow for fallthrough. Similar to Rust's `match` construct.
///
/// # Example
/// @code {.cpp}
/// using MyEnum = Enum<u32, f32, std::string>;
/// auto my_enum = MyEnum(enum_tag<std::string>{}, "MyEnum");
///
///	enum_inspect(my_enum) {
///		enum_variant(std::string, const auto& value) {
///		    println("enum was a std::string: {}", value);
///		}
///		any_variant() {
///		    println("enum wasn't a std::string");
///		}
///	}
/// @endcode
///
/// @param _enum  The `Enum` to pattern match on
///
/// @ingroup enum
/// @headerfile "Hyperion/Enum.h"
#define enum_inspect(_enum) /** NOLINT(cppcoreguidelines-macro-usage) **/     \
	IGNORE_RESERVED_IDENTIFIERS_START                                         \
	for(auto __variant = &(_enum); __variant != nullptr; __variant = nullptr) \
		IGNORE_RESERVED_IDENTIFIERS_STOP                                      \
	switch(__variant->get_index())

/// @brief Defines one branch in an `enum_inspect` statement.
///
/// Used to match on one `Enum` variant type, optionally extracting the value of the variant from
/// the `Enum`. If extracting the value of the variant from the `Enum`, structured binding
/// decomposition is supported.
///
/// @param Type  The `Enum` variant to match against
/// @param ...  The (optional) variable declaration or structured binding decomposition to extract
/// the variant value to
///
/// @see enum_inspect
/// @ingroup enum
/// @headerfile "Hyperion/Enum.h"
#define enum_variant(Type, ...) /** NOLINT(cppcoreguidelines-macro-usage) **/                \
	break;                                                                                   \
	case mpl::index_of_v<Type, std::remove_cvref_t<decltype(*__variant)>::list>:             \
		/** NOLINT(bugprone-macro-parentheses) **/                                           \
		/** NOLINTNEXTLINE(bugprone-macro-parentheses) **/                                   \
		for(__VA_ARGS__ __VA_OPT__(= __variant->template get<Type>()); __variant != nullptr; \
			__variant = nullptr)

/// @brief Defines a catch-all branch in an `enum_inspect` statement.
///
/// Used to match against any `Enum` variant type not explicitly matched against with an
/// `enum_variant` branch.
///
/// @see enum_inspect
/// @ingroup enum
/// @headerfile "Hyperion/Enum.h"
#define any_variant() /** NOLINT(cppcoreguidelines-macro-usage) **/ \
	break;                                                          \
	default:                                                        \
		/** NOLINT(bugprone-macro-parentheses) **/                  \
		/** NOLINTNEXTLINE(bugprone-macro-parentheses) **/          \
		for(; __variant != nullptr; __variant = nullptr)

	// NOLINTNEXTLINE(cert-dcl59-cpp, google-build-namespaces, modernize-use-trailing-return-type)
	TEST_SUITE("Enum") {
		struct TestStruct1 {
			TestStruct1(usize _val1, f64 _val2) noexcept : val1(_val1), val2(_val2) {
			}
			usize val1;
			f64 val2;
		};

		struct TestStruct2 {
			// NOLINTNEXTLINE(modernize-pass-by-value)
			TestStruct2(i64 _val1, const std::string& _val2) noexcept : val1(_val1), val2(_val2) {
			}
			i64 val1;
			std::string val2;
		};

		TEST_CASE("VerificationSet1") {
			using test_enum = Enum<TestStruct1, TestStruct2>;

			test_enum val(enum_tag<TestStruct1>{}, 2_usize, 1.0_f64);

			CHECK(val.is_variant(0_usize));
			CHECK_EQ(val.get_index(), 0_usize);
			CHECK_FALSE(val.is_valueless());

			const auto& current = val.get<TestStruct1>();
			CHECK_EQ(current.val1, 2_usize);
			CHECK_LT(current.val2, 1.01_f64);
			CHECK_GT(current.val2, 0.99_f64);
			CHECK_NE(val.get_if<0_usize>(), nullptr);
			CHECK_EQ(val.get_if<1_usize>(), nullptr);

			SUBCASE("Inspection") {
				enum_inspect(val) {
					enum_variant(TestStruct1, const auto& [valusize, valf64]) {
						CHECK_EQ(valusize, 2_usize);
						CHECK_LT(valf64, 1.01_f64);
						CHECK_GT(valf64, 0.99_f64);
					}
					enum_variant(TestStruct2) {
						// we shouldn't get here
						CHECK_FALSE(true);
					}
				}
			}

			SUBCASE("MatchWithOverload") {
				val.match(Overload{[](const TestStruct1& val1) noexcept -> void {
									   CHECK_EQ(val1.val1, 2_usize);
									   CHECK_LT(val1.val2, 1.01_f64);
									   CHECK_GT(val1.val2, 0.99_f64);
								   },
								   []([[maybe_unused]] const TestStruct2& val2) noexcept -> void {
									   // we shouldn't get here
									   CHECK_FALSE(true);
								   }});
			}

			SUBCASE("Assignment") {
				val = TestStruct2(3_i32, "TestString");

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Emplacement") {
				val.emplace<TestStruct2>(3_i32, "TestString");

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Copy") {
				auto _val = test_enum(enum_tag<TestStruct2>{}, 3_i32, "TestString");
				val = _val;

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Move") {
				auto _val = test_enum(enum_tag<TestStruct2>{}, 3_i32, "TestString");
				val = std::move(_val);

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}
		}

		struct TestStruct3 {
			TestStruct3(usize* _val1, f64 _val2) noexcept : val1(_val1), val2(_val2) {
				(*val1) += 1_usize;
			}
			TestStruct3(const TestStruct3& test) noexcept : val1(test.val1), val2(test.val2) {
				(*val1) += 1_usize;
			}
			TestStruct3(TestStruct3&& test) noexcept : val1(test.val1), val2(test.val2) {
				test.val1 = nullptr;
			}
			~TestStruct3() noexcept {
				if(val1 != nullptr) {
					(*val1) -= 1_usize;
				}
			}

			auto operator=(const TestStruct3&) noexcept -> TestStruct3& = default;
			auto operator=(TestStruct3&& test) noexcept -> TestStruct3& = default;

			usize* val1;
			f64 val2;
		};

		struct TestStruct4 {
			// NOLINTNEXTLINE(modernize-pass-by-value)
			TestStruct4(i64* _val1, const std::string& _val2) noexcept : val1(_val1), val2(_val2) {
				(*val1) += 1_i64;
			}
			TestStruct4(const TestStruct4& test) noexcept : val1(test.val1), val2(test.val2) {
				(*val1) += 1_i64;
			}
			TestStruct4(TestStruct4&& test) noexcept : val1(test.val1), val2(std::move(test.val2)) {
				test.val1 = nullptr;
			}
			~TestStruct4() noexcept {
				if(val1 != nullptr) {
					(*val1) -= 1_i64;
				}
			}
			auto operator=(const TestStruct4&) noexcept -> TestStruct4& = default;
			auto operator=(TestStruct4&& test) noexcept -> TestStruct4& = default;

			i64* val1;
			std::string val2;
		};

		// case to check that constructors and destructors are run the expected number of times
		TEST_CASE("VerificationSet2") {
			using test_enum = Enum<TestStruct3, TestStruct4>;

			auto struct3_instances = 0_usize;
			auto struct4_instances = 0_i64;

			SUBCASE("NumDestructorsRun") {
				auto val = test_enum(enum_tag<TestStruct3>{}, &struct3_instances, 1.0_f64);

				CHECK(val.is_variant(0_usize));
				CHECK_EQ(val.get_index(), 0_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& current = val.get<TestStruct3>();
				CHECK_EQ((*current.val1), 1_usize);
				CHECK_EQ((*current.val1), struct3_instances);
				CHECK_EQ(struct4_instances, 0_i64);
				CHECK_LT(current.val2, 1.01_f64);
				CHECK_GT(current.val2, 0.99_f64);
				CHECK_NE(val.get_if<0_usize>(), nullptr);
				CHECK_EQ(val.get_if<1_usize>(), nullptr);

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct3, const auto& [valusizeptr, valf64]) {
							CHECK_EQ((*valusizeptr), 1_usize);
							CHECK_LT(valf64, 1.01_f64);
							CHECK_GT(valf64, 0.99_f64);
						}
						enum_variant(TestStruct4) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
					}
				}

				SUBCASE("MatchWithOverload") {
					val.match(
						Overload{[](const TestStruct3& val1) noexcept -> void {
									 CHECK_EQ((*val1.val1), 1_usize);
									 CHECK_LT(val1.val2, 1.01_f64);
									 CHECK_GT(val1.val2, 0.99_f64);
								 },
								 []([[maybe_unused]] const TestStruct4& val2) noexcept -> void {
									 // we shouldn't get here
									 CHECK_FALSE(true);
								 }});
				}

				SUBCASE("Assignment") {
					{
						val = TestStruct4(&struct4_instances, "TestString");
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Emplacement") {
					val.emplace<TestStruct4>(&struct4_instances, "TestString");

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Copy") {
					{
						auto _val
							= test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
						val = _val;
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Move") {
					{
						auto _val
							= test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
						test_enum _val2(std::move(_val));
						val = std::move(_val2);
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}
			}

			CHECK_EQ(struct3_instances, 0_usize);
			CHECK_EQ(struct4_instances, 0_i64);
		}
	}
} // namespace hyperion
