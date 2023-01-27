/// @file HasValue.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Concept requiring a type has a `static constexpr` member variable `value`
/// @version 0.1
/// @date 2023-01-26
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

#include <Hyperion/BasicTypes.h>
#include <concepts>

namespace hyperion::mpl::detail {
	template<typename T, T value>
	using is_static_constexpr_value_impl = T;

	template<typename T>
	concept HasStaticConstexprValue
		= std::same_as < detail::is_static_constexpr_value_impl<decltype(T::value), T::value>,
	decltype(T::value) > ;
} // namespace hyperion::mpl::detail

namespace hyperion::mpl {
	/// @brief Concept that requires T is a type with a `static constexpr` member variable named
	/// `value`
	/// @ingroup mpl
	/// @headerfile "Hyperion/mpl/HasValue.h"
	template<typename T>
	concept HasValue = requires() {
		T::value;
		requires detail::HasStaticConstexprValue<T>;
	};

	namespace detail::test {
		struct HasValueTest1 { };

		struct HasValueTest2 {
			usize value;
		};

		struct HasValueTest3 {
			static usize value; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		};

		struct HasValueTest4 {
			static constexpr usize value = 0;
		};

		static_assert(!HasValue<HasValueTest1>, "HasValueTest1 failing");
		static_assert(!HasValue<HasValueTest2>, "HasValueTest2 failing");
		static_assert(!HasValue<HasValueTest3>, "HasValueTest3 failing");
		static_assert(HasValue<HasValueTest4>, "HasValueTest4 failing");
	} // namespace detail::test
} // namespace hyperion::mpl
