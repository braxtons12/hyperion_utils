/// @file CompressedPair.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief This file includes an
/// [Empty Base Class Optimized](https://en.cppreference.com/w/cpp/language/ebo) pair type
/// @version 0.1
/// @date 2023-01-25
///
/// MIT License
/// @copyright Copyright (c) 2023 Braxton Salyer <braxtonsalyer@gmail.com>
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include <Hyperion/Concepts.h>
#include <Hyperion/memory/EmptyBaseClass.h>

namespace hyperion {

	/// @brief Compressed Pair is an
	/// [Empty Base Class Optimized](https://en.cppreference.com/w/cpp/language/ebo) pair type
	///
	/// @tparam T - the first type stored in the pair
	/// @tparam U - The second type stored in the pair
	///
	/// # Requirements
	/// - `concepts::NotSame<T, U>` because Empty Base Class Optimization can't be performed when
	/// the types are the same, we can't have a compressed pair wrapping two of the same type
	/// @ingroup memory
	template<typename T, typename U>
	requires concepts::NotSame<T, U>
	class CompressedPair : private EmptyBaseClass<T>,
						   private EmptyBaseClass<U> {
	  public:
		/// @brief The first Empty Base Class Optimized type in the pair
		/// @ingroup memory
		using BaseT = EmptyBaseClass<T>;
		/// @brief The second Empty Base Class Optimized type in the pair
		/// @ingroup memory
		using BaseU = EmptyBaseClass<U>;

		/// @brief Constructs an uninitialized `CompressedPair`
		/// @ingroup memory
		constexpr CompressedPair() noexcept(
			concepts::NoexceptConstructibleFrom<BaseT, UnInitTag<T>>&&
				concepts::NoexceptConstructibleFrom<BaseU, UnInitTag<U>>)
			: BaseT(UnInitTag<T>()), BaseU(UnInitTag<U>()) {
		}
		/// @brief Constructs a `CompressedPair` with the `T` constructed from `args` and the `U`
		/// uninitialized
		///
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		///
		/// @param args - The arguments to pass to `T`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, Args...>`: `T` must be constructible from `args` in
		/// order to construct a `CompressedPair` from them
		/// @ingroup memory
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr CompressedPair(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<T, Args...>)
			: BaseT(std::forward<Args>(args)...), BaseU(UnInitTag<U>()) {
		}
		/// @brief Constructs a `CompressedPair` with the `U` constructed from `args` and the `T`
		/// uninitialized
		///
		/// @tparam Args - The types of the arguments to pass to `U`'s constructor
		///
		/// @param args - The arguments to pass to `U`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<U, Args...>`: `U` must be constructible from `args` in
		/// order to construct a `CompressedPair` from them
		/// @ingroup memory
		template<typename... Args>
		requires concepts::ConstructibleFrom<U, Args...>
		explicit constexpr CompressedPair(Args&&... args) noexcept(
			concepts::NoexceptConstructibleFrom<U, Args...>)
			: BaseU(std::forward<Args>(args)...), BaseT(UnInitTag<T>()) {
		}
		/// @brief Constructs a `CompressedPair` from the pair of arguments.
		///
		/// Constructs a `CompressedPair` from the pair of arguments. Constructs the `T` from the
		/// `T_` argument, `t`, and the `U` from the `U_` argument, `u`.
		///
		/// @tparam T_ - The type of the argument to construct the `T` from
		/// @tparam U_ - The type of the argument to construct the `U` from
		///
		/// @param t - The argument to pass to `T`'s constructor
		/// @param u - The argument to pass to `U`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, decltype(t)>`: `T` must be constructible from `t` to
		/// construct a `CompressedPair` from `t` and `u`
		/// - `concepts::ConstructibleFrom<U, decltype(u)>`: `U` must be constructible from `u` to
		/// construct a `CompressedPair` from `t` and `u`
		/// @ingroup memory
		template<typename T_, typename U_>
			constexpr CompressedPair(T_&& t, U_&& u) // NOLINT(readability-identifier-length)
			noexcept(
				concepts::NoexceptConstructibleFrom<BaseT, T_>&&
					concepts::NoexceptConstructibleFrom<
						BaseU,
						U_>) requires(concepts::
												   ConstructibleFrom<
													   T,
													   T_> || concepts::Convertible<T_, T>)
			&& (concepts::ConstructibleFrom<U,
											U_> || concepts::Convertible<U_, U>)
			: BaseT(std::forward<T_>(t)),
		BaseU(std::forward<U_>(u)) {
		}
		/// @brief Constructs a `CompressedPair` from the pair of arguments.
		///
		/// Constructs a `CompressedPair` from the pair of arguments. Leaves the `T` uninitialized,
		/// and constructs the `U` from the `U_` argument, `u`.
		///
		/// @tparam U_ - The type of the argument to construct the `U` from
		///
		/// @param t - The uninit tag signaling to leave the `T` uninitialized
		/// @param u - The argument to pass to `U`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<U, decltype(u)>`: `U` must be constructible from `u` to
		/// construct a `CompressedPair` from `u`
		/// @ingroup memory
		template<typename U_>
		constexpr CompressedPair(UnInitTag<T> t, U_&& u) // NOLINT(readability-identifier-length)
			noexcept(
				concepts::NoexceptConstructibleFrom<BaseT, UnInitTag<T>>&&
					concepts::NoexceptConstructibleFrom<BaseU, U_>) requires
			concepts::ConstructibleFrom<U, U_> : BaseT(t),
														  BaseU(std::forward<U_>(u)) {
		}
		/// @brief Constructs a `CompressedPair` from the pair of arguments.
		///
		/// Constructs a `CompressedPair` from the pair of arguments. Leaves the `U` uninitialized,
		/// and constructs the `T` from the `T_` argument, `t`.
		///
		/// @tparam T_ - The type of the argument to construct the `T` from
		///
		/// @param t - The argument to pass to `T`'s constructor
		/// @param u - The uninit tag signaling to leave the `U` uninitialized
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, decltype(t)>`: `T` must be constructible from `t` to
		/// construct a `CompressedPair` from `t`
		/// @ingroup memory
		template<typename T_>
		constexpr CompressedPair(T_&& t, UnInitTag<U> u) // NOLINT(readability-identifier-length)
			noexcept(
				concepts::NoexceptConstructibleFrom<BaseT, T_>&&
					concepts::NoexceptConstructibleFrom<BaseU, UnInitTag<U>>) requires
			concepts::ConstructibleFrom<T, T_> || concepts::Convertible<T_, T>
			: BaseT(std::forward<T_>(t)),
			  BaseU(u) {
		}
		/// @brief Constructs a `CompressedPair`, leaving both the `T` and `U` uninitialized
		///
		/// @param t - The uninit tag signaling to leave the `T` uninitialized
		/// @param u - The uninit tag signaling to leave the `U` uninitialized
		/// @ingroup memory
		constexpr CompressedPair(UnInitTag<T> t, // NOLINT(readability-identifier-length)
								 UnInitTag<U> u) // NOLINT(readability-identifier-length)
			noexcept(concepts::NoexceptConstructibleFrom<BaseT, UnInitTag<T>>&&
						 concepts::NoexceptConstructibleFrom<BaseU, UnInitTag<U>>)
			: BaseT(t), BaseU(u) {
		}
		/// @brief Constructs a `CompressedPair` from the pair of arguments.
		///
		/// Constructs a `CompressedPair` from the pair of arguments. Default-constructs the `T`,
		/// and constructs the `U` from the `U_` argument, `u`.
		///
		/// @tparam U_ - The type of the argument to construct the `U` from
		///
		/// @param t - The default init tag signaling to default-construct the `T`
		/// @param u - The argument to pass to `U`'s constructor
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<U, decltype(u)>`: `U` must be constructible from `u` to
		/// construct a `CompressedPair` from `u`
		/// @ingroup memory
		template<typename U_>
		constexpr CompressedPair(DefaultInitTag<T> t, // NOLINT(readability-identifier-length)
								 U_&& u)			  // NOLINT(readability-identifier-length)
			noexcept(
				concepts::NoexceptConstructibleFrom<BaseT, DefaultInitTag<T>>&&
					concepts::NoexceptConstructibleFrom<BaseU, U_>) requires
			concepts::ConstructibleFrom<U, U_> : BaseT(t),
														  BaseU(std::forward<U_>(u)) {
		}
		/// @brief Constructs a `CompressedPair` from the pair of arguments.
		///
		/// Constructs a `CompressedPair` from the pair of arguments. Default-constructs the `U`,
		/// and constructs the `T` from the `T_` argument, `t`.
		///
		/// @tparam T_ - The type of the argument to construct the `T` from
		///
		/// @param t - The argument to pass to `T`'s constructor
		/// @param u - The default init tag signaling to default-construct the `U`
		///
		/// # Requirements
		/// - `concepts::ConstructibleFrom<T, decltype(t)>`: `T` must be constructible from `t` to
		/// construct a `CompressedPair` from `t`
		/// @ingroup memory
		template<typename T_>
		constexpr CompressedPair(T_&& t,			  // NOLINT(readability-identifier-length)
								 DefaultInitTag<U> u) // NOLINT(readability-identifier-length)
			noexcept(
				concepts::NoexceptConstructibleFrom<BaseT, T_>&&
					concepts::NoexceptConstructibleFrom<BaseU, DefaultInitTag<U>>) requires
			concepts::ConstructibleFrom<T, T_> || concepts::Convertible<T_, T>
			: BaseT(std::forward<T_>(t)),
			  BaseU(u) {
		}
		/// @brief Constructs a `CompressedPair`, default-constructing both the `T` and `U`
		///
		/// @param t - The default init tag signaling to default-construct the `T`
		/// @param u - The default init tag signaling to default-construct the `U`
		/// @ingroup memory
		constexpr CompressedPair(DefaultInitTag<T> t, // NOLINT(readability-identifier-length)
								 DefaultInitTag<U> u) // NOLINT(readability-identifier-length)
			noexcept(concepts::NoexceptConstructibleFrom<BaseT, DefaultInitTag<T>>&&
						 concepts::NoexceptConstructibleFrom<BaseU, DefaultInitTag<U>>)
			: BaseT(t), BaseU(u) {
		}
		/// @brief Copy-Constructs a `CompressedPair` from the given one
		/// @ingroup memory
		constexpr CompressedPair(const CompressedPair&) noexcept = default;
		/// @brief Move-Constructs a `CompressedPair` from the given one
		/// @ingroup memory
		constexpr CompressedPair(CompressedPair&&) noexcept = default;
		/// @brief Destructs this `CompressedPair`
		/// @ingroup memory
		constexpr ~CompressedPair() noexcept = default;

		/// @brief Copy-Assigns this `CompressedPair` from the given one
		/// @ingroup memory
		constexpr auto operator=(const CompressedPair&) noexcept(
			concepts::NoexceptCopyAssignable<BaseT>&&
				concepts::NoexceptCopyAssignable<BaseU>) -> CompressedPair& = default;
		/// @brief Move-Assigns this `CompressedPair` from the given one
		/// @ingroup memory
		constexpr auto operator=(CompressedPair&&) noexcept(
			concepts::NoexceptMoveAssignable<BaseT>&&
				concepts::NoexceptMoveAssignable<BaseU>) -> CompressedPair& = default;

		/// @brief Returns a reference to const to the wrapped `T`
		///
		/// @return a reference to the wrapped `T`
		/// @ingroup memory
		constexpr auto first() const noexcept -> typename BaseT::const_reference {
			return static_cast<const BaseT&>(*this).get();
		}
		/// @brief Returns a reference to the wrapped `T`
		///
		/// @return a reference to the wrapped `T`
		/// @ingroup memory
		constexpr auto first() noexcept -> typename BaseT::reference {
			return static_cast<BaseT&>(*this).get();
		}

		/// @brief Returns a reference to const to the wrapped `U`
		///
		/// @return a reference to the wrapped `U`
		/// @ingroup memory
		constexpr auto second() const noexcept -> typename BaseU::const_reference {
			return static_cast<const BaseU&>(*this).get();
		}
		/// @brief Returns a reference to the wrapped `U`
		///
		/// @return a reference to the wrapped `U`
		/// @ingroup memory
		constexpr auto second() noexcept -> typename BaseU::reference {
			return static_cast<BaseU&>(*this).get();
		}

		/// @brief Swaps the contents of this `CompressedPair` with the given one
		///
		/// @param pair - The `CompressedPair` to swap contents with
		/// @ingroup memory
		inline constexpr auto swap(CompressedPair& pair) noexcept(
			concepts::NoexceptSwappable<T>&& concepts::NoexceptSwappable<U>)
			-> void requires concepts::Swappable<T> && concepts::Swappable<U> {
			std::swap(first(), pair.first());
			std::swap(second(), pair.second());
		}
	};

	template<typename T, typename U>
	CompressedPair(T, U) -> CompressedPair<T, U>;
	template<typename T, typename U>
	CompressedPair(UnInitTag<T>, U) -> CompressedPair<T, U>;
	template<typename T, typename U>
	CompressedPair(T, UnInitTag<U>) -> CompressedPair<T, U>;
	template<typename T, typename U>
	CompressedPair(UnInitTag<T>, UnInitTag<U>) -> CompressedPair<T, U>;

	/// @brief Swaps the contents of the first `CompressedPair` with the second one
	///
	/// @tparam T - The first type stored in the pairs
	/// @tparam U - The second type stored in the pairs
	///
	/// @param first - The `CompressedPair` to swap contents to
	/// @param second - The `CompressedPair` to swap contents from
	/// @ingroup memory
	template<typename T, typename U>
	requires concepts::Swappable<T> && concepts::Swappable<U>
	inline constexpr auto swap(CompressedPair<T, U>& first, CompressedPair<T, U>& second) noexcept(
		concepts::NoexceptSwappable<T>&& concepts::NoexceptSwappable<U>) -> void {
		first.swap(second);
	}
} // namespace hyperion
