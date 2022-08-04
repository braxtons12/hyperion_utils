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
				= std::conditional_t<Index < SIZE - 1_usize, variant<Index>, None>;
		};

		template<usize Index, typename... Types>
		struct EnumUnion;

		// clang-format off

        // manually unroll the union for type list sizes <= 5 to reduce template instantiations

        template<usize Index, typename... Types>
        requires(mpl::size_v<mpl::list<Types...>> == 1_usize)
        // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
            constexpr EnumUnion() noexcept : m_none() {
            }
            constexpr ~EnumUnion() noexcept {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                m_none.~None();
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
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
            constexpr EnumUnion() noexcept : m_none() {
            }
            constexpr ~EnumUnion() noexcept {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                m_none.~None();
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
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
            constexpr EnumUnion() noexcept : m_none() {
            }
            constexpr ~EnumUnion() noexcept {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                m_none.~None();
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
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
            constexpr EnumUnion() noexcept : m_none() {
            }
            constexpr ~EnumUnion() noexcept {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                m_none.~None();
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
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
            constexpr EnumUnion() noexcept : m_none() {
            }
            constexpr ~EnumUnion() noexcept {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                m_none.~None();
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
        struct EnumUnion<Index, Types...> : private virtual EnumTags<Types...> {
			// clang-format on
			constexpr EnumUnion() noexcept : m_none() {
			}
			constexpr ~EnumUnion() noexcept {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
				m_none.~None();
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
