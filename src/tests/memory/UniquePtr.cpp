/// @file UniquePtr.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for UniquePtr
/// @version 0.1
/// @date 2022-12-03
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
#include <Hyperion/Testing.h>
#include <Hyperion/memory/UniquePtr.h>

namespace hyperion {
	// NOLINTNEXTLINE
	TEST_SUITE("UniquePtr") {
		TEST_CASE("Constructor") {
			auto ptr1 = UniquePtr<i32>();
			// NOLINTNEXTLINE
			auto ptr2 = UniquePtr<i32>(new i32(3_i32));
			auto ptr3 = hyperion::make_unique<i32>(2_i32);

			CHECK_EQ(ptr1, nullptr);
			CHECK_NE(ptr2, nullptr);
			CHECK_NE(ptr3, nullptr);
			CHECK_EQ(*ptr2, 3_i32);
			CHECK_EQ(*ptr3, 2_i32);

			SUBCASE("move") {
				auto ptr4 = std::move(ptr3);
				// NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved)
				CHECK_EQ(ptr3, nullptr);
				CHECK_NE(ptr4, nullptr);
				CHECK_EQ(*ptr4, 2_i32);
			}

			SUBCASE("accessors_and_modifiers") {
				CHECK(ptr3);
				CHECK(static_cast<bool>(ptr3));
				CHECK_NE(ptr3.get(), nullptr);
				CHECK_EQ(*(ptr3.get()), 2_i32);

				auto* ptr4 = ptr3.release();
				CHECK_EQ(ptr3, nullptr);
				CHECK_NE(ptr4, nullptr);
				CHECK_EQ(*ptr4, 2_i32);

				*ptr4 = 4_i32;
				ptr3.reset(ptr4);
				CHECK_NE(ptr3, nullptr);
				CHECK(ptr3);
				CHECK_EQ(*ptr3, 4_i32);
				CHECK_EQ(*(ptr3.get()), 4_i32);
			}
		}
	}
} // namespace hyperion
