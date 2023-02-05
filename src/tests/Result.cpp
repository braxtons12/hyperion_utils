/// @file Result.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for Result
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
#include <Hyperion/Option.h>
#include <Hyperion/Result.h>
#include <Hyperion/Testing.h>

namespace hyperion {
	// NOLINTNEXTLINE
	TEST_SUITE("Result") {
		TEST_CASE("Ok") {
			// NOLINTNEXTLINE(readability-identifier-length)
			Result<i32> ok = Ok(5_i32);

			SUBCASE("accessors") {
				CHECK(ok.is_ok());
				CHECK_FALSE(ok.is_err());
				CHECK(ok);
				CHECK(static_cast<bool>(ok));
			}

			SUBCASE("as_cref") {
				CHECK_EQ(ok.as_cref(), 5_i32);
			}

			SUBCASE("as_ref") {
				CHECK_EQ(ok.as_ref(), 5_i32);

				ok.as_ref() = 2_i32;
				CHECK_EQ(ok.as_ref(), 2_i32);
				CHECK_EQ(ok.as_cref(), 2_i32);
			}

			SUBCASE("unwrap") {
				CHECK_EQ(ok.unwrap(), 5_i32);
			}

			SUBCASE("unwrap_or") {
				CHECK_EQ(ok.unwrap_or(2_i32), 5_i32);
			}

			SUBCASE("unwrap_or_else") {
				CHECK_EQ(ok.unwrap_or_else([]() { return 2_i32; }), 5_i32);
			}

			SUBCASE("expect") {
				CHECK_EQ(ok.expect("Test failed!"), 5_i32);
			}

			SUBCASE("unwrap_err") {
				// can't test currently as doctest doesn't support an aborts test
			}

			SUBCASE("ok") {
				auto maybe_ok = ok.ok();
				CHECK(maybe_ok.is_some());
				CHECK_EQ(maybe_ok.unwrap(), 5_i32);
			}

			SUBCASE("err") {
				auto maybe_err = ok.err();
				CHECK(maybe_err.is_none());
			}

			SUBCASE("map") {
				auto maybe_ok = ok.map([](const i32 value) { return value * 2_i32; });
				CHECK(maybe_ok.is_ok());
				CHECK_FALSE(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap(), 10_i32);
			}

			SUBCASE("map_or") {
				CHECK_EQ(ok.map_or([](const i32 value) { return value * 2_i32; }, 4_i32), 10_i32);
			}

			SUBCASE("map_or_else") {
				CHECK_EQ(ok.map_or_else([](const i32 value) { return value * 2_i32; },
										[]() { return 4_i32; }),
						 10_i32);
			}

			SUBCASE("map_err") {
				// NOLINTNEXTLINE(bugprone-exception-escape, readability-identifier-length)
				auto maybe_ok = ok.map_err([]([[maybe_unused]] const error::SystemError& _) noexcept
										   -> error::SystemError { return {3}; });

				CHECK(maybe_ok.is_ok());
				CHECK_FALSE(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap(), 5_i32);
			}

			SUBCASE("match") {
				CHECK_EQ(
					ok.match([](const i32 value) noexcept { return std::to_string(value * 2_i32); },
							 [](const error::SystemError& error) noexcept -> std::string {
								 return error.message();
							 }),
					std::to_string(10_i32));
			}

			SUBCASE("and_then") {
				auto next = ok.and_then([](const i32 value) noexcept -> Result<std::string> {
					return Ok(std::to_string(value));
				});

				CHECK(next.is_ok());
				CHECK_FALSE(next.is_err());
				CHECK_EQ(next.unwrap(), std::to_string(5_i32));
			}

			SUBCASE("or_else") {
				auto next = ok.or_else([](const error::SystemError& error) noexcept -> Result<i32> {
					return error::SystemError(error.code().value() * 2_i32);
				});

				CHECK(next.is_ok());
				CHECK_FALSE(next.is_err());
				CHECK_EQ(next.unwrap(), 5_i32);
			}

			SUBCASE("move") {
				constexpr auto move_test = [](Result<i32>&& value) noexcept -> void {
					CHECK(value.is_ok());
					CHECK_FALSE(value.is_err());
					CHECK(value);
					CHECK_EQ(value.unwrap(), 5_i32);
				};

				move_test(std::move(ok));
			}
		}

		TEST_CASE("Err") {
			Result<i32> err = error::SystemError(2_i32);

			SUBCASE("accessors") {
				CHECK_FALSE(err.is_ok());
				CHECK(err.is_err());
				CHECK_FALSE(err);
				CHECK_FALSE(static_cast<bool>(err));
			}

			SUBCASE("as_cref") {
				// can't test currently as doctest doesn't support an aborts test
			}

			SUBCASE("as_ref") {
				// can't test currently as doctest doesn't support an aborts test
			}

			SUBCASE("unwrap") {
				// can't test currently as doctest doesn't support an aborts test
			}

			SUBCASE("unwrap_or") {
				CHECK_EQ(err.unwrap_or(2_i32), 2_i32);
			}

			SUBCASE("unwrap_or_else") {
				CHECK_EQ(err.unwrap_or_else([]() { return 2_i32; }), 2_i32);
			}

			SUBCASE("expect") {
				// can't test currently as doctest doesn't support an aborts test
			}

			SUBCASE("unwrap_err") {
				CHECK_EQ(err.unwrap_err(), error::SystemError(2_i32));
			}

			SUBCASE("ok") {
				auto maybe_ok = err.ok();
				CHECK_FALSE(maybe_ok.is_some());
			}

			SUBCASE("err") {
				auto maybe_err = err.err();
				CHECK(maybe_err.is_some());
				CHECK_FALSE(maybe_err.is_none());
				CHECK_EQ(maybe_err.unwrap(), error::SystemError(2_i32));
			}

			SUBCASE("map") {
				auto maybe_ok = err.map([](const i32 value) { return value * 2_i32; });
				CHECK_FALSE(maybe_ok.is_ok());
				CHECK(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap_err(), error::SystemError(2_i32));
			}

			SUBCASE("map_or") {
				CHECK_EQ(err.map_or([](const i32 value) { return value * 2_i32; }, 4_i32), 4_i32);
			}

			SUBCASE("map_or_else") {
				CHECK_EQ(err.map_or_else([](const i32 value) { return value * 2_i32; },
										 []() { return 4_i32; }),
						 4_i32);
			}

			SUBCASE("map_err") {
				auto maybe_ok
					// NOLINTNEXTLINE(bugprone-exception-escape, readability-identifier-length)
					= err.map_err([]([[maybe_unused]] const error::SystemError& _) noexcept
								  -> error::SystemError { return {3}; });

				CHECK_FALSE(maybe_ok.is_ok());
				CHECK(maybe_ok.is_err());
				CHECK_EQ(maybe_ok.unwrap_err(), error::SystemError(3_i32));
			}

			SUBCASE("match") {
				CHECK_EQ(err.match(
							 [](const i32 value) noexcept { return std::to_string(value * 2_i32); },
							 [](const error::SystemError& error) noexcept -> std::string {
								 return error.message();
							 }),
						 error::SystemError(2).message());
			}

			SUBCASE("and_then") {
				auto next = err.and_then([](const i32 value) noexcept -> Result<std::string> {
					return Ok(std::to_string(value));
				});

				CHECK_FALSE(next.is_ok());
				CHECK(next.is_err());
				CHECK_EQ(next.unwrap_err(), error::SystemError(2_i32));
			}

			SUBCASE("or_else") {
				auto next
					= err.or_else([](const error::SystemError& error) noexcept -> Result<i32> {
						  return error::SystemError(error.code().value() * 2_i32);
					  });

				CHECK_FALSE(next.is_ok());
				CHECK(next.is_err());
				CHECK_EQ(next.unwrap_err(), error::SystemError(4_i32));
			}

			SUBCASE("move") {
				constexpr auto move_test = [](Result<i32>&& value) noexcept -> void {
					CHECK_FALSE(value.is_ok());
					CHECK(value.is_err());
					CHECK_FALSE(value);
					CHECK_EQ(value.unwrap_err(), error::SystemError(2_i32));
				};

				move_test(std::move(err));
			}
		}
	}
} // namespace hyperion
