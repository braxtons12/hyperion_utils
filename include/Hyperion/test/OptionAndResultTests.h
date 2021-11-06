/// @file OptionAndResultTests.h
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Unit Tests for `Option<T>` and `Result<T, E>`
/// @version 0.1
/// @date 2021-11-05
///
/// MIT License
/// @copyright Copyright (c) 2021 Braxton Salyer <braxtonsalyer@gmail.com>
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

#include <Hyperion/Option.h>
#include <Hyperion/Result.h>

#if HYPERION_DEFINE_TESTS
namespace hyperion {

	// NOLINTNEXTLINE(modernize-use-trailing-return-type)
	TEST_SUITE("Option") {
		TEST_CASE("Some") {
			auto some = Some(5_i32);

			CHECK(some.is_some());
			CHECK_FALSE(some.is_none());

			SUBCASE("as_const") {
				CHECK_EQ(some.as_const(), 5_i32);
			}

			SUBCASE("as_mut") {
				CHECK_EQ(some.as_mut(), 5_i32);

				some.as_mut() = 2_i32;
				CHECK_EQ(some.unwrap(), 2_i32);
			}

			SUBCASE("unwrap") {
				CHECK_EQ(some.unwrap(), 5_i32);
			}

			SUBCASE("unwrap_or") {
				CHECK_EQ(some.unwrap_or(2_i32), 5_i32);
			}

			SUBCASE("unwrap_or_else") {
				CHECK_EQ(some.unwrap_or_else([]() { return 2_i32; }), 5_i32);
			}

			SUBCASE("map") {
				auto mapped = some.map([](const i32 value) noexcept { return value * 2_i32; });

				CHECK(mapped.is_some());
				CHECK_FALSE(mapped.is_none());
				CHECK_EQ(mapped.unwrap(), 10_i32);
			}

			SUBCASE("map_or") {
				auto mapped
					= some.map_or([](const i32 value) noexcept { return value * 2_i32; }, 3_i32);

				CHECK_EQ(mapped, 10_i32);
			}

			SUBCASE("map_or_else") {
				auto mapped
					= some.map_or_else([](const i32 value) noexcept { return value * 2_i32; },
									   []() { return 3_i32; });

				CHECK_EQ(mapped, 10_i32);
			}

			SUBCASE("ok_or") {
				auto error = error::SystemError(2);
				auto maybe_ok = some.ok_or(error);

				CHECK(maybe_ok.is_ok());
				CHECK_FALSE(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap(), 5_i32);
			}

			SUBCASE("ok_or_else") {
				auto maybe_ok = some.ok_or_else([]() { return error::SystemError(2); });

				CHECK(maybe_ok.is_ok());
				CHECK_FALSE(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap(), 5_i32);
			}

			SUBCASE("and_then") {
				auto next = some.and_then([](i32 value) noexcept -> Option<u32> {
					return Some(static_cast<u32>(value * 2_i32));
				});

				CHECK(next.is_some());
				CHECK_FALSE(next.is_none());
				CHECK_EQ(next.unwrap(), 10_u32);
			}

			SUBCASE("or_else") {
				auto next = some.or_else([]() noexcept -> Option<i32> { return Some(2_i32); });

				CHECK(next.is_some());
				CHECK_FALSE(next.is_none());
				CHECK_EQ(next.unwrap(), 5_i32);
			}

			SUBCASE("move") {
				constexpr auto move_test = [](Option<i32>&& value) noexcept -> void {
					CHECK(value.is_some()); // NOLINT(clang-analyzer-cplusplus.Move)
					CHECK_FALSE(value.is_none());
					CHECK_EQ(value.unwrap(), 5_i32);
				};

				move_test(std::move(some));
			}
		}
	}

	TEST_CASE("None") {
		Option<i32> none = None();

		CHECK_FALSE(none.is_some());
		CHECK(none.is_none());

		SUBCASE("as_const") {
			// can't test currently because doctest doesn't have an aborts macro
		}

		SUBCASE("as_mut") {
			// can't test currently because doctest doesn't have an aborts macro
		}

		SUBCASE("unwrap") {
			// can't test currently because doctest doesn't have an aborts macro
		}

		SUBCASE("unwrap_or") {
			CHECK_EQ(none.unwrap_or(2_i32), 2_i32);
		}

		SUBCASE("unwrap_or_else") {
			CHECK_EQ(none.unwrap_or_else([]() { return 2_i32; }), 2_i32);
		}

		SUBCASE("map") {
			auto mapped = none.map([](const i32 value) noexcept { return value * 2_i32; });

			CHECK_FALSE(mapped.is_some());
			CHECK(mapped.is_none());
		}

		SUBCASE("map_or") {
			auto mapped
				= none.map_or([](const i32 value) noexcept { return value * 2_i32; }, 3_i32);

			CHECK_EQ(mapped, 3_i32);
		}

		SUBCASE("map_or_else") {
			auto mapped
				= none.map_or_else([](const i32 value) noexcept { return value * 2_i32; },
								   []() { return 3_i32; });

			CHECK_EQ(mapped, 3_i32);
		}

		SUBCASE("ok_or") {
			auto error = error::SystemError(2);
			auto maybe_ok = none.ok_or(error);

			CHECK_FALSE(maybe_ok.is_ok());
			CHECK(maybe_ok.is_err());
			CHECK_EQ(maybe_ok.unwrap_err(), error::SystemError(2));
		}

		SUBCASE("ok_or_else") {
			auto maybe_ok = none.ok_or_else([]() { return error::SystemError(2); });

			CHECK_FALSE(maybe_ok.is_ok());
			CHECK(maybe_ok.is_err());
			CHECK_EQ(maybe_ok.unwrap_err(), error::SystemError(2));
		}

		SUBCASE("and_then") {
			auto next = none.and_then([](i32 value) noexcept -> Option<u32> {
				return Some(static_cast<u32>(value * 2_i32));
			});

			CHECK_FALSE(next.is_some());
			CHECK(next.is_none());
		}

		SUBCASE("or_else") {
			auto next = none.or_else([]() noexcept -> Option<i32> { return Some(2_i32); });

			CHECK(next.is_some());
			CHECK_FALSE(next.is_none());
			CHECK_EQ(next.unwrap(), 2_i32);
		}

		SUBCASE("move") {
			constexpr auto move_test = [](Option<i32>&& value) noexcept -> void {
				CHECK_FALSE(value.is_some());
				CHECK(value.is_none());
			};

			move_test(std::move(none));
		}
	}
} // namespace hyperion
#endif // HYPERION_DEFINE_TESTS
