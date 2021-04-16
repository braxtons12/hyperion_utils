/// @brief Meta-programming functions to call a single meta-function with a runtime-provided index
#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

#include "../Macros.h"
#include "Callable.h"
#include "IndexType.h"

namespace hyperion::utils::mpl {

	namespace detail {

		/// @brief Concept that requires T is a meta-programming type with a value
		template<typename T>
		concept HasValue = requires() {
			T::value;
		};

		template<usize Size>
		struct call_with_index_impl {
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {
				if(i < Size / 2) {
					return call_with_index_impl<Size / 2>::template call<K>(i, std::forward<F>(f));
				}
				else {
					return call_with_index_impl<Size - Size / 2>::template call<K + Size / 2>(
						i - Size / 2,
						std::forward<F>(f));
				}
			}
		};

		template<>
		struct call_with_index_impl<0> { };

		template<>
		struct call_with_index_impl<1> {
			template<usize K, typename F>
			inline static constexpr auto call(usize /** i **/, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {
				return std::forward<F>(f)(index_type<K>());
			}
		};

		template<>
		struct call_with_index_impl<2> {
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
				}
			}
		};

		template<>
		struct call_with_index_impl<3> {
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
				}
			}
		};

		template<>
		struct call_with_index_impl<4> {
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
				}
			}
		};

		template<>
		struct call_with_index_impl<5> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
				}
			}
		};

		template<>
		struct call_with_index_impl<6> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
					case 5: return std::forward<F>(f)(index_type<K + 5>()); // NOLINT
				}
			}
		};

		template<>
		struct call_with_index_impl<7> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
					case 5: return std::forward<F>(f)(index_type<K + 5>()); // NOLINT
					case 6: return std::forward<F>(f)(index_type<K + 6>()); // NOLINT
				}
			}
		};

		template<>
		struct call_with_index_impl<8> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
					case 5: return std::forward<F>(f)(index_type<K + 5>()); // NOLINT
					case 6: return std::forward<F>(f)(index_type<K + 6>()); // NOLINT
					case 7: return std::forward<F>(f)(index_type<K + 7>()); // NOLINT
				}
			}
		};

		template<>
		struct call_with_index_impl<9> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
					case 5: return std::forward<F>(f)(index_type<K + 5>()); // NOLINT
					case 6: return std::forward<F>(f)(index_type<K + 6>()); // NOLINT
					case 7: return std::forward<F>(f)(index_type<K + 7>()); // NOLINT
					case 8: return std::forward<F>(f)(index_type<K + 8>()); // NOLINT
				}
			}
		};

		template<>
		struct call_with_index_impl<10> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto call(usize i, F&& f) noexcept
				-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

				switch(i) {
					default: HYPERION_UNREACHABLE();
					case 0: return std::forward<F>(f)(index_type<K + 0>());
					case 1: return std::forward<F>(f)(index_type<K + 1>());
					case 2: return std::forward<F>(f)(index_type<K + 2>());
					case 3: return std::forward<F>(f)(index_type<K + 3>());
					case 4: return std::forward<F>(f)(index_type<K + 4>());
					case 5: return std::forward<F>(f)(index_type<K + 5>()); // NOLINT
					case 6: return std::forward<F>(f)(index_type<K + 6>()); // NOLINT
					case 7: return std::forward<F>(f)(index_type<K + 7>()); // NOLINT
					case 8: return std::forward<F>(f)(index_type<K + 8>()); // NOLINT
					case 9: return std::forward<F>(f)(index_type<K + 9>()); // NOLINT
				}
			}
		};

	} // namespace detail

	// clang-format off

	/// @brief Calls the function/fuction object `f` with `mpl::index_type<i>`
	/// `i` must be strictly less than `Size`
	///
	/// @tparam Size - The number of indices
	/// @tparam F - The type of the fuction/function object to call
	/// @param `i` - The index to use as the call parameter
	/// @param `f` - The fuction/function object to call
	template<usize Size, typename F>
	requires(is_callable_with<F, index_type<0>>::value)
	inline static constexpr auto call_with_index(usize i, F&& f) noexcept
		-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

		assert(i < Size);
		return detail::call_with_index_impl<Size>::template call<0>(i, std::forward<F>(f));
	}

	template<detail::HasValue SizeType, typename F>
	requires(is_callable_with<F, index_type<0>>::value)
	inline static constexpr auto call_with_index(usize i, F&& f) noexcept
		-> decltype(std::declval<F>()(std::declval<index_type<0>>())) {

		return call_with_index<SizeType::value, F>(i, std::forward<F>(f));
	}

	// clang-format on
} // namespace hyperion::utils::mpl
