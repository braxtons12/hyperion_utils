/// @file SomeFWD.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Forward declarations of the `Some` functions for creating an `Option<T>` containing a
/// value
/// @version 0.1
/// @date 2022-8-26
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

namespace hyperion {
	template<typename T>
	class Option;

	/// @brief Creates an `Option<T>` by storing the given `T` in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	///
	/// @param some - The value to store in the `Option<T>`
	///
	/// @return an `Option<T>` containing the given value
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T,
			 typename U = std::conditional_t<concepts::Pointer<std::remove_reference_t<T>>,
											 std::remove_reference_t<T>,
											 T>>
	[[nodiscard]] inline constexpr auto Some(T&& some) noexcept -> Option<U>;

	/// @brief Creates an `Option<T>` by constructing a `T` in place in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	/// @tparam Args - The types of the arguments to pass to `T`'s constructor
	///
	/// @param args - The arguments to pass to `T`'s constructor
	///
	/// @return an `Option<T>` containing a newly constructed `T`
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T, typename... Args>
	requires concepts::ConstructibleFrom<T, Args...>
	[[nodiscard]] inline constexpr auto Some(Args&&... args) noexcept -> Option<T>;

	/// @brief Creates an `Option<T>` by storing the given `T` in the `Option<T>`
	///
	/// @tparam T - The type to store in the option
	///
	/// @param some - The value to store in the `Option<T>`
	///
	/// @return an `Option<T>` containing the given value
	/// @ingroup option
	/// @headerfile "Hyperion/Option.h"
	template<typename T>
	[[nodiscard]] inline constexpr auto
	Some(concepts::Convertible<T> auto&& some) noexcept -> Option<T>;
} // namespace hyperion
