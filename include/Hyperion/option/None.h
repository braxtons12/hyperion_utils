/// @file None.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Type representing a null, missing, or invalid value
/// @version 0.1
/// @date 2021-10-19
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

namespace hyperion::option {

	/// @brief `None` represents a null, missing, or invalid value
	/// @ingroup option
	/// @headerfile "Hyperion/option/None.h"
	struct None {
		/// @brief Default Constructor
		/// @ingroup option
		constexpr None() noexcept = default;
		/// @brief Copy Constructor
		/// @ingroup option
		constexpr None(const None& none) noexcept = default;
		/// @brief Move Constructor
		/// @ingroup option
		constexpr None(None&& none) noexcept = default;
		/// @brief Destructor
		/// @ingroup option
		constexpr ~None() noexcept = default;

		/// @brief Copy Assignment Operator
		/// @ingroup option
		constexpr auto operator=(const None& none) noexcept -> None& = default;
		/// @brief Move Assignment Operator
		/// @ingroup option
		constexpr auto operator=(None&& none) noexcept -> None& = default;
	};

	/// @brief Static `None` to use to represent any null value
	/// @ingroup option
	static constexpr None none;
} // namespace hyperion::option
