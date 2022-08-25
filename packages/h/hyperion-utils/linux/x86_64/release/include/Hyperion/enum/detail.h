/// @file enum/detail.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Storage implementation details for hyperion::Enum
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
#include <Hyperion/Concepts.h>
#include <Hyperion/MPL.h>
#include <Hyperion/option/None.h>
#include <limits>
#include <new>

namespace hyperion {
	template<typename T>
	struct enum_tag { };

	namespace detail {
		using option::None;

		template<usize Size>
		struct size_type {
			using type = std::conditional_t<
				Size + 1_usize <= std::numeric_limits<u8>::max(),
				u8,
				std::conditional_t<
					Size + 1_usize <= std::numeric_limits<u16>::max(),
					u16,
					std::conditional_t<Size + 1_usize <= std::numeric_limits<u32>::max(),
									   u32,
									   u64>>>;
		};

		template<typename... Types>
		struct EnumTags {
			using list = mpl::list<Types...>;
			using size_type = typename size_type<mpl::size_v<list>>::type;
			static inline constexpr size_type SIZE = mpl::size_v<list>;

			template<usize Index>
			using variant = mpl::at_t<Index, list>;

			template<usize Index>
			using next_variant
				= std::conditional_t<Index < SIZE, variant<Index>, None>;
		};

		template<usize Index, typename... Types>
		struct EnumUnion;

		// clang-format off

        // manually unroll the union for type list sizes <= 5 to reduce template instantiations

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 1_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr EnumUnion() noexcept {
            }
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr ~EnumUnion() noexcept {
               //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
               //m_none.~None();
            }
            using tags = EnumTags<Types...>;
            union {
                typename tags::template variant<0_usize> m_first;
                None m_none = None();
            };
        };

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 2_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr EnumUnion() noexcept {
            }
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr ~EnumUnion() noexcept {
               //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
               //m_none.~None();
            }
            using tags = EnumTags<Types...>;
            union {
                typename tags::template variant<0_usize> m_first;
                typename tags::template variant<1_usize> m_second;
                None m_none = None();
            };
        };

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 3_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr EnumUnion() noexcept {
            }
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr ~EnumUnion() noexcept {
               //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
               //m_none.~None();
            }
            using tags = EnumTags<Types...>;
            union {
                typename tags::template variant<0_usize> m_first;
                typename tags::template variant<1_usize> m_second;
                typename tags::template variant<2_usize> m_third;
                None m_none = None();
            };
        };

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 4_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr EnumUnion() noexcept {
            }
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr ~EnumUnion() noexcept {
               //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
               //m_none.~None();
            }
            using tags = EnumTags<Types...>;
            union {
                typename tags::template variant<0_usize> m_first;
                typename tags::template variant<1_usize> m_second;
                typename tags::template variant<2_usize> m_third;
                typename tags::template variant<3_usize> m_fourth;
                None m_none = None();
            };
        };

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 5_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr EnumUnion() noexcept {
            }
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
            constexpr ~EnumUnion() noexcept {
               //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
               //m_none.~None();
            }
            using tags = EnumTags<Types...>;
            union {
                typename tags::template variant<0_usize> m_first;
                typename tags::template variant<1_usize> m_second;
                typename tags::template variant<2_usize> m_third;
                typename tags::template variant<3_usize> m_fourth;
                typename tags::template variant<4_usize> m_fifth;
                None m_none = None();
            };
        };

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> >= 6_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : public virtual EnumTags<Types...> {
			// clang-format on
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
			constexpr EnumUnion() noexcept {
			}
            // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
			constexpr ~EnumUnion() noexcept {
                //// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                //m_none.~None();
			}
			using tags = EnumTags<Types...>;

			template<usize Idx>
			using next = std::conditional_t<
				concepts::Same<typename tags::template next_variant<Idx>, None>,
				None,
				EnumUnion<Idx, Types...>>;

			union {
				typename tags::template variant<Index> m_current;
				next<Index + 1_usize> m_next;
				None m_none = None();
			};
		};
	} // namespace detail
} // namespace hyperion
