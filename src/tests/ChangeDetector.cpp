/// @file ChangeDetctor.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for ChangeDetector
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
#include <Hyperion/ChangeDetector.h>
#include <Hyperion/Testing.h>

namespace hyperion {
	// NOLINTNEXTLINE
	TEST_SUITE("ChangeDetector") {
		TEST_CASE("value") {
			auto detector = ChangeDetector<i32>();
			CHECK_EQ(detector.value(), 0_i32);

			SUBCASE("changed_lvalue") {
				auto val = 2_i32;
				CHECK(detector.changed(val));
				CHECK_EQ(detector.value(), 2_i32);
			}

			SUBCASE("changed_rvalue") {
				CHECK(detector.changed(2_i32));
				CHECK_EQ(detector.value(), 2_i32);
			}
		}

		TEST_CASE("pointer") {
			auto value = 0_i32;
			auto detector = ChangeDetector<i32*>(&value);
			CHECK_EQ(*(detector.value()), 0_i32);

			SUBCASE("changed") {
				auto val = 2_i32;
				CHECK(detector.changed(&val));
				CHECK_EQ(*(detector.value()), 2_i32);
			}
		}
	}
} // namespace hyperion
