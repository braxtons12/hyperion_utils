/// @file Index.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief basic meta-programming index type
/// @version 0.1
/// @date 2021-10-29
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

#include <Hyperion/BasicTypes.h>

namespace hyperion::mpl {

	/// @brief Basic meta-programming index type
	///
	/// @tparam N - The index
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/Index.h"
	template<usize N>
	struct index : std::integral_constant<usize, N> { };

	/// @brief Value of `mpl::index`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/Index.h"
	template<usize N>
	inline static constexpr usize index_v = index<N>::value;
} // namespace hyperion::mpl
