/// @file Err.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `Err` wraps an error for implicit conversion into a `Result`
/// @version 0.1
/// @date 2021-10-20
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/Concepts.h>
#include <Hyperion/Error.h>

namespace hyperion {

	/// @brief `Err` wraps and represents and error value and is used to create a `Result` in the
	/// `Err` variant
	///
	/// @tparam E - The error type
	/// @ingroup result
	/// @headerfile "Hyperion/result/Err.h"
	template<typename E = error::SystemError>
	struct Err;

	template<concepts::NotReference E>
	struct Err<E> {
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(
			const E& error) noexcept requires concepts::NoexceptCopyConstructible<E>
			: m_error(error) {
		}
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(E&& error) noexcept requires concepts::NoexceptMoveConstructible<E>
			: m_error(std::move(error)) {
		}
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(concepts::Convertible<E> auto&& error) noexcept // NOLINT
			requires concepts::NotSame<const Err&,
									   decltype(error)> && concepts::NotSame<Err&&, decltype(error)>
			: m_error(std::forward<E>(static_cast<E>(error))) {
		}

		/// @brief Constructs an `Err` by constructing the error value in place in it
		///
		/// @tparam Args - The types of arguments to pass to `E`'s constructor
		/// @param args - The arguments to pass to `E`'s constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<E, Args...>
		explicit constexpr Err(Args&&... args) noexcept : m_error(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr Err(const Err& err) noexcept requires concepts::NoexceptCopyConstructible<E>
		= default;
		/// @brief Move Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr Err(Err&& err) noexcept requires concepts::NoexceptMoveConstructible<E>
		= default;
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr ~Err() noexcept = default;

		inline constexpr operator E() const noexcept { // NOLINT
			return m_error;
		}

		/// @brief Copy Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr auto
		operator=(const Err& err) noexcept -> Err& requires concepts::NoexceptCopyAssignable<E>
		= default;
		/// @brief Move Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr auto
		operator=(Err&& err) noexcept -> Err& requires concepts::NoexceptMoveAssignable<E>
		= default;

		/// @brief The error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		E m_error;
	};

	template<concepts::Reference E>
	struct Err<E> {
		using type = std::remove_reference_t<E>;
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(const type& error) noexcept requires std::is_const_v<E>
			: m_error(error) {
		}
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(type& error) noexcept : m_error(error) {
		}
		/// @brief Constructs an `Err` from the given error value
		///
		/// @param error - The value representing an error
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		explicit constexpr Err(concepts::Convertible<E> auto&& error) noexcept // NOLINT
			requires concepts::NotSame<const Err&,
									   decltype(error)> && concepts::NotSame<Err&&, decltype(error)>
			: m_error(std::forward<E>(static_cast<E>(error))) {
		}
		/// @brief Copy Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr Err(const Err& err) noexcept = default;
		/// @brief Move Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr Err(Err&& err) noexcept : m_error(err.m_error) {
		}
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr ~Err() noexcept = default;

		inline constexpr operator E() const noexcept { // NOLINT
			return m_error;
		}

		/// @brief Copy Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr auto operator=(const Err& err) noexcept -> Err& = default;
		/// @brief Move Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Err.h"
		constexpr auto operator=(Err&& err) noexcept -> Err& {
			m_error = err.m_error;
		}

		/// @brief The error
		std::reference_wrapper<type> m_error;
	};

	/// @brief CTAD Deduction Guide
	template<typename E>
	Err(E) -> Err<E>;
} // namespace hyperion
