/// @file CallWithIndex.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Metaprogramming functions to call a single meta-function invocable with a
/// runtime-provided index
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

#include <Hyperion/HyperionDef.h>
#include <Hyperion/error/Assert.h>
#include <Hyperion/mpl/Index.h>
#include <Hyperion/mpl/HasValue.h>
#include <cassert>
#include <type_traits>
#include <utility>

namespace hyperion::mpl {

	namespace detail {
		template<usize Size>
		struct call_with_index_impl {
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {
				if(index < Size / 2) {
					return call_with_index_impl<Size / 2>::template call<K>(index, std::forward<F>(func));
				}

				return call_with_index_impl<Size - Size / 2>::template call<K + Size / 2>(
					index - Size / 2,
					std::forward<F>(func));
			}
		};

		template<>
		struct call_with_index_impl<0> { };

		template<>
		struct call_with_index_impl<1> {
			template<usize K, typename F>
			inline static constexpr auto call(usize /** i **/, F&& func) noexcept
				-> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {
				return std::forward<F>(func)(mpl::index<K>());
			}
		};

		template<>
		struct call_with_index_impl<2> {
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<3> {
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<4> {
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<5> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<6> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					case 5: return std::forward<F>(func)(mpl::index<K + 5>()); // NOLINT
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<7> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					case 5: return std::forward<F>(func)(mpl::index<K + 5>()); // NOLINT
					case 6: return std::forward<F>(func)(mpl::index<K + 6>()); // NOLINT
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<8> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					case 5: return std::forward<F>(func)(mpl::index<K + 5>()); // NOLINT
					case 6: return std::forward<F>(func)(mpl::index<K + 6>()); // NOLINT
					case 7: return std::forward<F>(func)(mpl::index<K + 7>()); // NOLINT
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<9> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					case 5: return std::forward<F>(func)(mpl::index<K + 5>()); // NOLINT
					case 6: return std::forward<F>(func)(mpl::index<K + 6>()); // NOLINT
					case 7: return std::forward<F>(func)(mpl::index<K + 7>()); // NOLINT
					case 8: return std::forward<F>(func)(mpl::index<K + 8>()); // NOLINT
					default: HYPERION_UNREACHABLE();
				}
			}
		};

		template<>
		struct call_with_index_impl<10> { // NOLINT
			template<usize K, typename F>
			inline static constexpr auto
			call(usize index, F&& func) noexcept -> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {

				switch(index) {
					case 0: return std::forward<F>(func)(mpl::index<K + 0>());
					case 1: return std::forward<F>(func)(mpl::index<K + 1>());
					case 2: return std::forward<F>(func)(mpl::index<K + 2>());
					case 3: return std::forward<F>(func)(mpl::index<K + 3>());
					case 4: return std::forward<F>(func)(mpl::index<K + 4>());
					case 5: return std::forward<F>(func)(mpl::index<K + 5>()); // NOLINT
					case 6: return std::forward<F>(func)(mpl::index<K + 6>()); // NOLINT
					case 7: return std::forward<F>(func)(mpl::index<K + 7>()); // NOLINT
					case 8: return std::forward<F>(func)(mpl::index<K + 8>()); // NOLINT
					case 9: return std::forward<F>(func)(mpl::index<K + 9>()); // NOLINT
					default: HYPERION_UNREACHABLE();
				}
			}
		};

	} // namespace detail

	IGNORE_UNUSED_TEMPLATES_START
	/// @brief Invokes the function template `f` with an `mpl::index` with the value of `i`
	///
	/// `i` must be strictly less than `Size`
	///
	/// @tparam Size - The number of indices
	/// @tparam F - The type of the function/function object to call
	/// @param index - The index to use as the call parameter
	/// @param func - The function/function object to call
	///
	/// @return The result of invoking `f` with the `mpl::index` of value `i`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/CallWithIndex.h"
	template<usize Size, typename F>
	requires std::is_invocable_v<F, index<0>>
	inline static constexpr auto call_with_index(usize index, F&& func) noexcept
		-> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {
		HYPERION_ASSERT(index < Size, "index must be strictly less than size (index: {}, Size: {})", index, Size);
		return detail::call_with_index_impl<Size>::template call<0>(index, std::forward<F>(func));
	}

	/// @brief Invokes the function template `f` with an `mpl::index` with the value of `i`
	///
	/// `i` must be strictly less than `Size`
	///
	/// @tparam SizeType - The type containing the `static constexpr usize value` representing the
	/// number of indices
	/// @tparam F - The type of the function/function object to call
	/// @param index - The index to use as the call parameter
	/// @param func - The function/function object to call
	///
	/// @return The result of invoking `f` with the `mpl::index` of value `i`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/CallWithIndex.h"
	template<HasValue SizeType, typename F>
	requires std::is_invocable_v<F, index<0>>
	inline static constexpr auto call_with_index(usize index, F&& func) noexcept
		-> decltype(std::declval<F>()(std::declval<mpl::index<0>>())) {
		HYPERION_ASSERT(index < SizeType::Value,
						"index must be strictly less than size (index: {}, Size: {})",
						index,
						SizeType::value);
		return call_with_index<SizeType::value, F>(index, std::forward<F>(func));
	}
	IGNORE_UNUSED_TEMPLATES_STOP

} // namespace hyperion::mpl
