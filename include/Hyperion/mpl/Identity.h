/// @file Identity.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Type trait returning the type given to it
/// @version 0.1
/// @date 2023-01-26
///
/// MIT License
/// @copyright Copyright (c) 2023 Braxton Salyer <braxtonsalyer@gmail.com>
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

namespace hyperion::mpl {
    /// @brief `identity` is basic metaprogramming type for performing the identity function.
    /// Useful for situations such as peforming a branching/conditional meta-function and the
    /// original type needs to be preserved in one (or more) branch(es).
    /// @tparam T  The type to perform the identity function on
    /// @ingroup mpl
    /// @headerfile "Hyperion/Identity.h"
	template<typename T>
	struct identity {
		using type = T;
	};

    /// @brief Convienence alias for `typename hyperion::mpl::identity<T>::type`
    /// @tparam T  The type to perform the identity function on
    /// @ingroup mpl
    /// @headerfile "Hyperion/Identity.h"
	template<typename T>
	using identity_t = typename identity<T>::type;
} // namespace hyperion::mpl
