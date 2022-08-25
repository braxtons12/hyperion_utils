/// @file Ok.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief `Ok` wraps the result of a successful operation for implicit conversion into a `Result`
/// @version 0.1
/// @date 2022-06-04
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
/// @brief `Ok` represents a valid or successful value
#pragma once

#include <Hyperion/Concepts.h>
#include <Hyperion/option/None.h>

namespace hyperion {

	/// @brief `Ok` wraps and represents and the value of a successful operation and is used to
	/// create a `Result` in the `Ok` variant
	///
	/// @tparam T - The type of the value
	/// @ingroup result
	/// @headerfile "Hyperion/result/Ok.h"
	template<typename T = option::None>
	struct Ok;

	template<concepts::NotReference T>
	struct Ok<T> {
		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(const T& ok) noexcept // NOLINT(readability-identifier-length)
		requires concepts::NoexceptCopyConstructible<T>
		: m_ok(ok) {
		}
		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(T&& ok) noexcept // NOLINT(readability-identifier-length)
		requires concepts::NoexceptMoveConstructible<T>
		: m_ok(std::move(ok)) {
		}
		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(concepts::Convertible<T> auto&& ok) noexcept // NOLINT
		requires concepts::NotSame<const Ok&, decltype(ok)> && concepts::NotSame<Ok&&, decltype(ok)>
		: m_ok(std::forward<T>(static_cast<T>(ok))) {
		}

		/// @brief Constructs an `Ok` by constructing the `T` in place in it
		///
		/// @tparam Args - The types of the arguments to pass to `T`'s constructor
		/// @param args - The arguments to pass to `T`'s constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		template<typename... Args>
		requires concepts::ConstructibleFrom<T, Args...>
		explicit constexpr Ok(Args&&... args) noexcept : m_ok(std::forward<Args>(args)...) {
		}
		/// @brief Copy Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr Ok(const Ok&) noexcept
		requires concepts::NoexceptCopyConstructible<T>
		= default;
		/// @brief Move Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr Ok(Ok&&) noexcept
		requires concepts::NoexceptMoveConstructible<T>
		= default;
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr ~Ok() noexcept = default;

		inline constexpr operator T() const noexcept { // NOLINT
			return m_ok;
		}

		/// @brief Copy Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr auto operator=(const Ok&) noexcept -> Ok&
		requires concepts::NoexceptCopyAssignable<T>
		= default;
		/// @brief Move Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr auto operator=(Ok&&) noexcept -> Ok&
		requires concepts::NoexceptMoveAssignable<T>
		= default;

		/// @brief The value representing success
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		T m_ok;
	};

	template<concepts::Reference T>
	struct Ok<T> {
		using type = std::remove_reference_t<T>;

		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(const T& ok) noexcept // NOLINT(readability-identifier-length)
		requires std::is_const_v<T>
		: m_ok(ok) {
		}
		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(T& ok) noexcept // NOLINT(readability-identifier-length)
			: m_ok(ok) {
		}
		/// @brief Constructs an `Ok` from the given T
		///
		/// @param ok - The successful value
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		explicit constexpr Ok(concepts::Convertible<T> auto&& ok) noexcept // NOLINT
		requires concepts::NotSame<const Ok&, decltype(ok)> && concepts::NotSame<Ok&&, decltype(ok)>
		: m_ok(std::forward<T>(static_cast<T>(ok))) {
		}

		/// @brief Copy Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr Ok(const Ok&) noexcept = default;
		/// @brief Move Constructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr Ok(Ok&& ok) noexcept // NOLINT(readability-identifier-length)
			: m_ok(ok.m_ok) {
		}
		/// @brief Destructor
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr ~Ok() noexcept = default;

		inline constexpr operator T() const noexcept { // NOLINT
			return m_ok;
		}
		/// @brief Copy Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr auto operator=(const Ok&) noexcept -> Ok& = default;
		/// @brief Move Assignment Operator
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		constexpr auto operator=(Ok&& ok) noexcept // NOLINT(readability-identifier-length)
			-> Ok& {
			m_ok = ok.m_ok;
		}

		/// @brief The value representing success
		/// @ingroup result
		/// @headerfile "Hyperion/result/Ok.h"
		std::reference_wrapper<type> m_ok;
	};

	/// @brief CTAD Deduction Guide
	template<typename T>
	Ok(T) -> Ok<T>;
} // namespace hyperion
