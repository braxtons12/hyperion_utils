/// @file Enum.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief An alternative to std::variant with improved compile time and runtime performance and
/// additional features
/// @version 0.1
/// @date 2022-08-21
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

// the inspection macros declared before the tests break formatting for some reason
// clang-format off

namespace hyperion {

    template<typename... Callables> struct Overload : Callables... {
        using Callables::operator()...;
    };
    template<typename... Callables> Overload(Callables...) -> Overload<Callables...>;

	template<typename... Types>
	class Enum : private detail::EnumUnion<0_usize, Types...> {
	  private:
		using Union = detail::EnumUnion<0_usize, Types...>;
		using tags = typename Union::tags;

		template<typename T>
		using reference = std::add_lvalue_reference_t<T>;

		template<typename T>
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>;

		template<typename T>
		using rvalue_reference = std::add_rvalue_reference_t<T>;

		template<typename T>
		using const_rvalue_reference = std::add_rvalue_reference_t<std::add_const_t<T>>;

		template<usize Index>
		using next_variant = typename tags::template next_variant<Index>;

	  public:
		using list = typename tags::list;
		using size_type = typename tags::size_type;
		static constexpr auto SIZE = static_cast<size_type>(tags::SIZE);

		template<usize Index, std::enable_if_t<(Index < SIZE), bool> = true>
		using variant = typename tags::template variant<Index>;

        template<typename T>
        requires mpl::contains_v<T, list>
        static inline constexpr auto variant_index = mpl::index_of_v<T, list>;

	  private:
		size_type m_current_index = SIZE;

		template<usize Index, typename... Args>
        requires (Index < SIZE)
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

		template<usize Index, typename Arg>
        requires (Index < SIZE)
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
		// default-constructs as the first variant
		constexpr Enum() noexcept(concepts::NoexceptDefaultConstructible<variant<0>>)
            requires concepts::DefaultConstructible<variant<0>>
        {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			construct<0_usize, true>();
			set_index(0_usize);
		}

		// Constructs as the first variant constructible from `value`
		template<typename T>
		// NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
		explicit constexpr Enum(T&& value)
            noexcept(mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_constructible,
				                                             std::true_type,
				                                             list,
				                                             mpl::list<T>>)
            requires(check_constructible<0, decltype(value)>().first)
        {
			construct<check_constructible<0, decltype(value)>().second, true>(std::forward<T>(value));
		}

		// constructs the variant indicated by `T` from `args...`
		template<typename T, typename... Args>
		requires(mpl::contains_v<T, list>
                 && concepts::ConstructibleFrom<T, Args...>)
        explicit constexpr Enum([[maybe_unused]] enum_tag<T> tag, Args&&... args)
            noexcept(concepts::NoexceptConstructibleFrom<T, Args...>)
        {
			construct<variant_index<T>, true>(std::forward<Args>(args)...);
		}

		constexpr Enum(const Enum& value)
            noexcept(mpl::all_types_satisfy_v<std::is_nothrow_copy_constructible,
									          std::true_type,
									          list>)
            requires mpl::all_types_satisfy_v<std::is_copy_constructible, std::true_type, list>
        {
			if(!value.is_valueless()) {
				const auto idx = value.get_index();
				mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
                    this->template construct<
                          check_constructible<0_usize,
                                              decltype(value.get_with_index(index))>().second, true>(
                            value.get_with_index(index));
				});
			}
		}

		constexpr Enum(Enum&& value) noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_move_constructible,
									 std::true_type,
									 list>) requires
			mpl::all_types_satisfy_v<std::is_move_constructible, std::true_type, list> {
			if(!value.is_valueless()) {
				const auto idx = value.get_index();
				mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
                    this->template construct<
                          check_constructible<0_usize,
                                              decltype(std::move(value).get_with_index(index))>().second, true>(
                            std::move(value).get_with_index(index));
				});
			}
		}

// work around for clang not-yet supporting multiple destructors
#if HYPERION_PLATFORM_COMPILER_CLANG
        constexpr ~Enum()
            noexcept(mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
        {
            if constexpr(mpl::any_type_satisfies_v<std::is_trivially_destructible,
                                                   std::false_type,
                                                   list>)
            {
                destruct(get_index());
            }
        }
#else

        // destructor to use if __any__ type in `list` is __not__ trivially destructible
		constexpr ~Enum()
            noexcept(mpl::all_types_satisfy_v<std::is_nothrow_destructible, std::true_type, list>)
            requires(mpl::any_type_satisfies_v<std::is_trivially_destructible, std::false_type, list>)
		{
            destruct(get_index());
		}
        // destructor to use if all types in `list` are trivially destructible
        // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
        constexpr ~Enum() noexcept = default;
#endif


		constexpr auto operator=(const Enum& value) noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_copy_constructible, std::true_type, list>)
			-> Enum& requires
			mpl::all_types_satisfy_v<std::is_copy_assignable, std::true_type, list> {
			if(this == &value) {
				return *this;
			}

			const auto idx = value.get_index();
			mpl::call_with_index<SIZE>(idx, [this, &value](auto index) noexcept {
				this->template assign<check_assignable<0_usize,
                                              decltype(value.get_with_index(index))>().second, true>(
                      value.get_with_index(index));
			});

			return *this;
		}

		constexpr auto operator=(Enum&& value) noexcept(
			mpl::all_types_satisfy_v<std::is_nothrow_move_constructible, std::true_type, list>)
			-> Enum& requires
			mpl::all_types_satisfy_v<std::is_move_constructible, std::true_type, list> {
			if(this == &value) {
				return *this;
			}

			const auto idx = value.get_index();
			mpl::call_with_index<SIZE>(idx, [this, val = std::move(value)](auto index) noexcept {
				this->template assign<check_assignable<0_usize,
                                              decltype(std::move(val).get_with_index(index))>().second, true>(
                      std::move(val).get_with_index(index));
			});

			return *this;
		}

		template<typename T>
		requires mpl::any_type_satisfies_with_arg_list_v<std::is_assignable,
														 std::true_type,
														 list,
														 mpl::list<T>>
        // NOLINTNEXTLINE
		constexpr auto operator=(T&& value) noexcept(
			mpl::any_type_satisfies_with_arg_list_v<std::is_nothrow_assignable,
													std::true_type,
													list,
													mpl::list<T>>) -> Enum& requires concepts::NotSame<std::remove_cvref_t<decltype(value)>, Enum> {
			this->template assign<check_assignable<0_usize, decltype(value)>().second, true>(
                    std::forward<T>(value));
			return *this;
		}

        template<usize N, typename... Args>
        requires concepts::ConstructibleFrom<variant<N>, Args...>
        constexpr auto emplace(Args&& ...args)
            noexcept(concepts::NoexceptConstructibleFrom<variant<N>, Args...>) -> void {
            if(!is_valueless()) {
                destruct(get_index());
            }

            construct<N, true>(std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        requires concepts::ConstructibleFrom<T, Args...>
        constexpr auto emplace(Args&& ...args)
            noexcept(concepts::NoexceptConstructibleFrom<T, Args...>) -> void {
            if(!is_valueless()) {
                destruct(get_index());
            }

            construct<variant_index<T>, true>(std::forward<Args>(args)...);
        }

		[[nodiscard]] inline constexpr auto get_index() const noexcept -> size_type {
			return m_current_index;
		}

		[[nodiscard]] inline constexpr auto index() const noexcept -> size_type {
			return m_current_index;
		}

		[[nodiscard]] inline constexpr auto is_variant(usize index) const noexcept -> bool {
			return m_current_index == index;
		}

		// clang-format off

        template<usize Index>
        requires (Index < SIZE)
        [[nodiscard]] inline constexpr auto is_variant() const noexcept
            -> bool {
            return m_current_index == Index;
        }

        template<typename T>
        requires mpl::contains_v<T, list> && (variant_index<T> < SIZE)
        [[nodiscard]] inline constexpr auto is_variant() const noexcept -> bool {
            return m_current_index == variant_index<T>;
        }

        [[nodiscard]] inline constexpr auto is_valueless() const noexcept -> bool {
            return m_current_index == SIZE;
        }

      private:
        using list_as_const = mpl::apply_to_list<std::add_const_t, list>;
        using list_as_ref = mpl::apply_to_list<std::add_lvalue_reference_t, list>;
        using list_as_const_ref = mpl::apply_to_list<std::add_lvalue_reference_t, list_as_const>;
      public:

		template<typename F>
		requires (mpl::all_lists_satisfy_for_type_v<std::is_invocable,
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
			return mpl::call_with_index<SIZE>(
                        get_index(),
                        [this, func = std::forward<F>(function)](auto index) mutable
                            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
                                                                       std::true_type,
                                                                       F,
                                                                       mpl::apply_to_list<
                                                                           mpl::list,
                                                                           list>>)
                        {
                            return std::forward<F>(func)(this->get_with_index(index));
                        });
		}

		template<typename F>
		requires (mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
                                                   F,
												   mpl::apply_to_list<mpl::list, list>>
                 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
                                                      std::true_type,
                                                      F,
                                                      mpl::apply_to_list<mpl::list, list_as_ref>>)
		[[nodiscard]] inline constexpr auto match(F&& function)
            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
											           std::true_type,
                                                       F,
											           mpl::apply_to_list<mpl::list, list>>)
        {
			return mpl::call_with_index<SIZE>(
                        get_index(),
                        [this, func = std::forward<F>(function)](auto index) mutable
                            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
                                                                       std::true_type,
                                                                       F,
                                                                       mpl::apply_to_list<
                                                                           mpl::list,
                                                                           list>>)
                        {
                            return std::forward<F>(func)(this->get_with_index(index));
                        });
		}

      private:
        template<typename T>
        using make_list_of_lists = mpl::apply_to_list<mpl::list, T>;
      public:
		template<typename... F>
		requires (mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
												   decltype(Overload{std::declval<F>()...}),
												   make_list_of_lists<list>>
                 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
                                                      std::true_type,
												      decltype(Overload{std::declval<F>()...}),
                                                      make_list_of_lists<list_as_const_ref>>)
		[[nodiscard]] inline constexpr auto match(F&& ...functions) const
            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
											           std::true_type,
												       decltype(Overload{std::declval<F>()...}),
											           mpl::apply_to_list<mpl::list, list>>)
        {
			return mpl::call_with_index<SIZE>(
                        get_index(),
                        [this, overload = Overload{std::forward<F>(functions)...}](auto index) mutable
                            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
                                                                       std::true_type,
												                       decltype(Overload{std::declval<F>()...}),
                                                                       mpl::apply_to_list<
                                                                           mpl::list,
                                                                           list>>)
                        {
                            return std::move(overload)(this->get_with_index(index));
                        });
		}

		template<typename... F>
		requires (mpl::all_lists_satisfy_for_type_v<std::is_invocable,
												   std::true_type,
												   decltype(Overload{std::declval<F>()...}),
												   mpl::apply_to_list<mpl::list, list>>
                 || mpl::all_lists_satisfy_for_type_v<std::is_invocable,
                                                      std::true_type,
												      decltype(Overload{std::declval<F>()...}),
                                                      mpl::apply_to_list<mpl::list, list_as_ref>>)
		[[nodiscard]] inline constexpr auto match(F&& ...functions)
            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
											           std::true_type,
												       decltype(Overload{std::declval<F>()...}),
											           mpl::apply_to_list<mpl::list, list>>)
        {
			return mpl::call_with_index<SIZE>(
                        get_index(),
                        [this, overload = Overload{std::forward<F>(functions)...}](auto index) mutable
                            noexcept(mpl::all_lists_satisfy_for_type_v<std::is_nothrow_invocable,
                                                                       std::true_type,
												                       decltype(Overload{std::declval<F>()...}),
                                                                       mpl::apply_to_list<
                                                                           mpl::list,
                                                                           list>>)
                        {
                            return std::move(overload)(this->get_with_index(index));
                        });
		}

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

        template<usize N>
        requires(N < SIZE)
        [[nodiscard]] inline constexpr auto get() const&& noexcept -> const_rvalue_reference<variant<N>> {
            if(!is_variant<N>()) {
                panic("get<{}>() called on variant when it currently holds variant {}",
                      N,
                      get_index());
            }

            return get_impl<N>(std::move(*this));
        }

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get() & noexcept -> reference<T> {
            if(!is_variant<T>()) {
                panic("get<T>() called on variant for T equal to index {} when it currently holds variant {}",
                      variant_index<T>,
                      get_index());
            }

			return get_impl<T>(*this);
		}

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get() const& noexcept -> const_reference<T> {
            if(!is_variant<T>()) {
                panic("get<T>() called on variant for T equal to index {} when it currently holds variant {}",
                      variant_index<T>,
                      get_index());
            }

			return get_impl<T>(*this);
		}

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get() && noexcept -> rvalue_reference<T> {
            if(!is_variant<T>()) {
                panic("get<T>() called on variant for T equal to index {} when it currently holds variant {}",
                      variant_index<T>,
                      get_index());
            }

			return get_impl<T>(std::move(*this));
		}

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get() const&& noexcept -> const_rvalue_reference<T> {
            if(!is_variant<T>()) {
                panic("get<T>() called on variant for T equal to index {} when it currently holds variant {}",
                      variant_index<T>,
                      get_index());
            }

			return get_impl<T>(std::move(*this));
		}

        template<usize N>
        requires(N < SIZE)
        [[nodiscard]] inline constexpr auto get_if() noexcept -> std::add_pointer_t<variant<N>> {
            if(!is_variant<N>()) {
                return nullptr;
            }

            return &(get_impl<N>(*this));
        }

        template<usize N>
        requires(N < SIZE)
        [[nodiscard]] inline constexpr auto get_if() const noexcept
            -> std::add_pointer_t<std::add_const_t<variant<N>>> {
            if(!is_variant<N>()) {
                return nullptr;
            }

            return &(get_impl<N>(*this));
        }

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get_if() noexcept -> std::add_pointer_t<T> {
            if(!is_variant<T>()) {
                return nullptr;
            }

			return &(get_impl<T>(*this));
		}

		template<typename T>
		requires mpl::contains_v<T, list>
		[[nodiscard]] inline constexpr auto get_if() const noexcept
            -> std::add_pointer_t<std::add_const_t<T>> {
            if(!is_variant<T>()) {
                return nullptr;
            }

			return &(get_impl<T>(*this));
		}

	  private:
		inline constexpr auto set_index(size_type index) noexcept -> void {
			m_current_index = index;
		}

		template<typename T, typename U>
		using get_correct_ref_qual = std::conditional_t<std::is_rvalue_reference_v<U>,
														std::add_rvalue_reference_t<T>,
														std::add_lvalue_reference_t<T>>;

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
			// NOLINTNEXTLINE(readability-identifier-length)
			constexpr auto N = mpl::index_of_v<T, list>;
			return get_impl<N>(std::forward<Union>(self));
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
        requires (Index < SIZE)
        struct do_destruct<Index> {
            static constexpr bool value = (std::is_class_v<variant<Index>>
                                             || (std::is_union_v<variant<Index>>
                                                 && concepts::Destructible<variant<Index>>))
                                            && !std::is_trivially_destructible_v<variant<Index>>;
        };

        template<usize Index>
        static constexpr bool do_destruct_v = do_destruct<Index>::value;

        template<usize Index>
        inline constexpr auto destruct_with_index([[maybe_unused]] mpl::index<Index> index)
            noexcept(concepts::NoexceptDestructible<variant<Index>>)
        {
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
                            if constexpr(do_destruct_v<2_usize>)
                            {
                                  // NOLINTNEXTLINE
                                  this->m_third.~variant();
                            }
                            break;
                        case 3_usize:
                            if constexpr(do_destruct_v<3_usize>)
                            {
                                  // NOLINTNEXTLINE
                                  this->m_fourth.~variant();
                            }
                            break;
                        case 4_usize:
                            if constexpr(do_destruct_v<4_usize>)
                            {
                                  using variant = variant<4_usize>;
                                  // NOLINTNEXTLINE
                                  this->m_fifth.~variant();
                            }
                            break;
                        default:
                            HYPERION_UNREACHABLE();
                    }
                }
                else {
                    mpl::call_with_index(current_index, [this](auto index) {
                        destruct_with_index(index);
                    });
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

					std::construct_at(std::addressof(get_impl<Index>(*this)), std::forward<Args>(args)...);
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
				// bail if we're only supposed to attempt construction at the current
				// index and the current index wasn't constructible
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
                    const auto assignable_but_this_variant_not_constructed
                        = get_index() != Index;

					if(assignable_but_this_variant_not_constructed) {
                        destruct(get_index());

						if constexpr(concepts::ConstructibleFrom<current, decltype(to_assign)>) {
                            std::construct_at(std::addressof(get_impl<Index>(*this)), std::forward<U>(to_assign));
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
                // we shouldn't be able to get here
                throw;
            }

			if constexpr(Index + 1_usize < SIZE) {
				using next_variant = next_variant<Index + 1_usize>;
				if constexpr(!concepts::Same<next_variant, detail::None>) {
					return assign<Index + 1_usize>(std::forward<U>(to_assign));
				}
			}
		}
	};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define inspect(variant)                                                        \
    IGNORE_RESERVED_IDENTIFIERS_START                                           \
    for(auto __variant = &(variant); __variant != nullptr; __variant = nullptr) \
    IGNORE_RESERVED_IDENTIFIERS_STOP                                            \
        switch (__variant->get_index())

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define decomposed(Type, ...)                                                     \
    break;                                                                        \
    case mpl::index_of_v<Type, std::remove_cvref_t<decltype(*__variant)>::list>:  \
        /** NOLINT(bugprone-macro-parentheses) **/                                \
        /** NOLINTNEXTLINE(bugprone-macro-parentheses) **/                        \
        for(__VA_ARGS__ __VA_OPT__(= __variant->template get<Type>());            \
            __variant != nullptr; __variant = nullptr)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define any_variant()                                                             \
    break;                                                                        \
    default:                                                                      \
        /** NOLINT(bugprone-macro-parentheses) **/                                \
        /** NOLINTNEXTLINE(bugprone-macro-parentheses) **/                        \
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
                inspect(val) {
                    decomposed(TestStruct1, const auto& [valusize, valf64]) {
                        CHECK_EQ(valusize, 2_usize);
                        CHECK_LT(valf64, 1.01_f64);
                        CHECK_GT(valf64, 0.99_f64);
                    }
                    decomposed(TestStruct2) {
                        // we shouldn't get here
                        CHECK_FALSE(true);
                    }
                }
            }

            SUBCASE("MatchWithOverload") {
                val.match(
                    Overload {
                        [](const TestStruct1& val1) noexcept -> void {
                            CHECK_EQ(val1.val1, 2_usize);
                            CHECK_LT(val1.val2, 1.01_f64);
                            CHECK_GT(val1.val2, 0.99_f64);
                        },
                        []([[maybe_unused]] const TestStruct2& val2) noexcept -> void {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                    }
                );
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
                    inspect(val) {
                        decomposed(TestStruct1) {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                        decomposed(TestStruct2, const auto& [vali64, valstr]) {
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
                        }
                    );
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
                    inspect(val) {
                        decomposed(TestStruct1) {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                        decomposed(TestStruct2, const auto& [vali64, valstr]) {
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
                        }
                    );
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
                    inspect(val) {
                        decomposed(TestStruct1) {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                        decomposed(TestStruct2, const auto& [vali64, valstr]) {
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
                        }
                    );
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
                    inspect(val) {
                        decomposed(TestStruct1) {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                        decomposed(TestStruct2, const auto& [vali64, valstr]) {
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
                        }
                    );
                }
            }
		}

		struct TestStruct3 {
			TestStruct3(usize* _val1, f64 _val2) noexcept : val1(_val1), val2(_val2) {
                (*val1) += 1_usize;
			}
            TestStruct3(const TestStruct3& test) noexcept :val1(test.val1), val2(test.val2){
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
            TestStruct4(const TestStruct4& test) noexcept :val1(test.val1), val2(test.val2){
                (*val1) += 1_i64;
            }
            TestStruct4(TestStruct4&& test) noexcept :val1(test.val1), val2(std::move(test.val2)){ 
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
                    inspect(val) {
                        decomposed(TestStruct3, const auto& [valusizeptr, valf64]) {
                            CHECK_EQ((*valusizeptr), 1_usize);
                            CHECK_LT(valf64, 1.01_f64);
                            CHECK_GT(valf64, 0.99_f64);
                        }
                        decomposed(TestStruct4) {
                            // we shouldn't get here
                            CHECK_FALSE(true);
                        }
                    }
                }

                SUBCASE("MatchWithOverload") {
                    val.match(
                        Overload {
                            [](const TestStruct3& val1) noexcept -> void {
                                CHECK_EQ((*val1.val1), 1_usize);
                                CHECK_LT(val1.val2, 1.01_f64);
                                CHECK_GT(val1.val2, 0.99_f64);
                            },
                            []([[maybe_unused]] const TestStruct4& val2) noexcept -> void {
                                // we shouldn't get here
                                CHECK_FALSE(true);
                            }
                        }
                    );
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
                        inspect(val) {
                            decomposed(TestStruct3) {
                                // we shouldn't get here
                                CHECK_FALSE(true);
                            }
                            decomposed(TestStruct4, const auto& [vali64ptr, valstr]) {
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
                            }
                        );
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
                        inspect(val) {
                            decomposed(TestStruct3) {
                                // we shouldn't get here
                                CHECK_FALSE(true);
                            }
                            decomposed(TestStruct4, const auto& [vali64ptr, valstr]) {
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
                            }
                        );
                    }
                }

                SUBCASE("Copy") {
                    {
                        auto _val = test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
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
                        inspect(val) {
                            decomposed(TestStruct3) {
                                // we shouldn't get here
                                CHECK_FALSE(true);
                            }
                            decomposed(TestStruct4, const auto& [vali64ptr, valstr]) {
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
                            }
                        );
                    }
                }

                SUBCASE("Move") {
                    {
                        auto _val = test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
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
                        inspect(val) {
                            decomposed(TestStruct3) {
                                // we shouldn't get here
                                CHECK_FALSE(true);
                            }
                            decomposed(TestStruct4, const auto& [vali64ptr, valstr]) {
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
                            }
                        );
                    }
                }
            }

            CHECK_EQ(struct3_instances, 0_usize);
            CHECK_EQ(struct4_instances, 0_i64);
		}
	}
} // namespace hyperion
