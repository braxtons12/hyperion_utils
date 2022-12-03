/// @file Enum.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for Enum
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
#include <Hyperion/Enum.h>
#include <Hyperion/Testing.h>

namespace hyperion {
	// NOLINTNEXTLINE(cert-dcl59-cpp, google-build-namespaces, modernize-use-trailing-return-type)
	TEST_SUITE("Enum") {
		struct TestStruct1 {
			TestStruct1(usize _val1, f64 _val2) noexcept : val1(_val1), val2(_val2) {
			}
			usize val1;
			f64 val2;
		};

		struct TestStruct2 {
			// NOLINTNEXTLINE(modernize-pass-by-value)
			TestStruct2(i64 _val1, const std::string& _val2) noexcept : val1(_val1), val2(_val2) {
			}
			i64 val1;
			std::string val2;
		};

		// NOLINTNEXTLINE(readability-function-cognitive-complexity)
		TEST_CASE("VerificationSet1") {
			using test_enum = Enum<TestStruct1, TestStruct2>;

			test_enum val(enum_tag<TestStruct1>{}, 2_usize, 1.0_f64);

			CHECK(val.is_variant(0_usize));
			CHECK_EQ(val.get_index(), 0_usize);
			CHECK_FALSE(val.is_valueless());

			const auto& current = val.get<TestStruct1>();
			CHECK_EQ(current.val1, 2_usize);
			CHECK_LT(current.val2, 1.01_f64);
			CHECK_GT(current.val2, 0.99_f64);
			CHECK_NE(val.get_if<0_usize>(), nullptr);
			CHECK_EQ(val.get_if<1_usize>(), nullptr);

			SUBCASE("Inspection") {
				enum_inspect(val) {
					enum_variant(TestStruct1, const auto& [valusize, valf64]) {
						CHECK_EQ(valusize, 2_usize);
						CHECK_LT(valf64, 1.01_f64);
						CHECK_GT(valf64, 0.99_f64);
					}
					enum_variant(TestStruct2) {
						// we shouldn't get here
						CHECK_FALSE(true);
					}
				}
			}

			SUBCASE("MatchWithOverload") {
				val.match(Overload{[](const TestStruct1& val1) noexcept -> void {
									   CHECK_EQ(val1.val1, 2_usize);
									   CHECK_LT(val1.val2, 1.01_f64);
									   CHECK_GT(val1.val2, 0.99_f64);
								   },
								   []([[maybe_unused]] const TestStruct2& val2) noexcept -> void {
									   // we shouldn't get here
									   CHECK_FALSE(true);
								   }});
			}

			SUBCASE("Assignment") {
				val = TestStruct2(3_i32, "TestString");

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Emplacement") {
				val.emplace<TestStruct2>(3_i32, "TestString");

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Copy") {
				auto _val = test_enum(enum_tag<TestStruct2>{}, 3_i32, "TestString");
				val = _val;

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}

			SUBCASE("Move") {
				auto _val = test_enum(enum_tag<TestStruct2>{}, 3_i32, "TestString");
				val = std::move(_val);

				CHECK(val.is_variant(1_usize));
				CHECK_EQ(val.get_index(), 1_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& new_val = val.get<TestStruct2>();
				CHECK_EQ(new_val.val1, 3_i64);
				CHECK_EQ(new_val.val2, std::string("TestString"));

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct1) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
						enum_variant(TestStruct2, const auto& [vali64, valstr]) {
							CHECK_EQ(vali64, 3_i64);
							CHECK_EQ(valstr, std::string("TestString"));
						}
					}
				}

				SUBCASE("Match") {
					val.match(
						[]([[maybe_unused]] const TestStruct1& val1) noexcept -> void {
							// we shouldn't get here
							CHECK_FALSE(true);
						},
						[](const TestStruct2& val2) noexcept -> void {
							CHECK_EQ(val2.val1, 3_i64);
							CHECK_EQ(val2.val2, std::string("TestString"));
						});
				}
			}
		}

		struct TestStruct3 {
			TestStruct3(usize* _val1, f64 _val2) noexcept : val1(_val1), val2(_val2) {
				(*val1) += 1_usize;
			}
			TestStruct3(const TestStruct3& test) noexcept : val1(test.val1), val2(test.val2) {
				(*val1) += 1_usize;
			}
			TestStruct3(TestStruct3&& test) noexcept : val1(test.val1), val2(test.val2) {
				test.val1 = nullptr;
			}
			~TestStruct3() noexcept {
				if(val1 != nullptr) {
					(*val1) -= 1_usize;
				}
			}

			auto operator=(const TestStruct3&) noexcept -> TestStruct3& = default;
			auto operator=(TestStruct3&& test) noexcept -> TestStruct3& = default;

			usize* val1;
			f64 val2;
		};

		struct TestStruct4 {
			// NOLINTNEXTLINE(modernize-pass-by-value)
			TestStruct4(i64* _val1, const std::string& _val2) noexcept : val1(_val1), val2(_val2) {
				(*val1) += 1_i64;
			}
			TestStruct4(const TestStruct4& test) noexcept : val1(test.val1), val2(test.val2) {
				(*val1) += 1_i64;
			}
			TestStruct4(TestStruct4&& test) noexcept : val1(test.val1), val2(std::move(test.val2)) {
				test.val1 = nullptr;
			}
			~TestStruct4() noexcept {
				if(val1 != nullptr) {
					(*val1) -= 1_i64;
				}
			}
			auto operator=(const TestStruct4&) noexcept -> TestStruct4& = default;
			auto operator=(TestStruct4&& test) noexcept -> TestStruct4& = default;

			i64* val1;
			std::string val2;
		};

		// case to check that constructors and destructors are run the expected number of times
		// NOLINTNEXTLINE(readability-function-cognitive-complexity)
		TEST_CASE("VerificationSet2") {
			using test_enum = Enum<TestStruct3, TestStruct4>;

			auto struct3_instances = 0_usize;
			auto struct4_instances = 0_i64;

			SUBCASE("NumDestructorsRun") {
				auto val = test_enum(enum_tag<TestStruct3>{}, &struct3_instances, 1.0_f64);

				CHECK(val.is_variant(0_usize));
				CHECK_EQ(val.get_index(), 0_usize);
				CHECK_FALSE(val.is_valueless());

				const auto& current = val.get<TestStruct3>();
				CHECK_EQ((*current.val1), 1_usize);
				CHECK_EQ((*current.val1), struct3_instances);
				CHECK_EQ(struct4_instances, 0_i64);
				CHECK_LT(current.val2, 1.01_f64);
				CHECK_GT(current.val2, 0.99_f64);
				CHECK_NE(val.get_if<0_usize>(), nullptr);
				CHECK_EQ(val.get_if<1_usize>(), nullptr);

				SUBCASE("Inspection") {
					enum_inspect(val) {
						enum_variant(TestStruct3, const auto& [valusizeptr, valf64]) {
							CHECK_EQ((*valusizeptr), 1_usize);
							CHECK_LT(valf64, 1.01_f64);
							CHECK_GT(valf64, 0.99_f64);
						}
						enum_variant(TestStruct4) {
							// we shouldn't get here
							CHECK_FALSE(true);
						}
					}
				}

				SUBCASE("MatchWithOverload") {
					val.match(
						Overload{[](const TestStruct3& val1) noexcept -> void {
									 CHECK_EQ((*val1.val1), 1_usize);
									 CHECK_LT(val1.val2, 1.01_f64);
									 CHECK_GT(val1.val2, 0.99_f64);
								 },
								 []([[maybe_unused]] const TestStruct4& val2) noexcept -> void {
									 // we shouldn't get here
									 CHECK_FALSE(true);
								 }});
				}

				SUBCASE("Assignment") {
					{
						val = TestStruct4(&struct4_instances, "TestString");
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Emplacement") {
					val.emplace<TestStruct4>(&struct4_instances, "TestString");

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Copy") {
					{
						auto _val
							= test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
						val = _val;
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}

				SUBCASE("Move") {
					{
						auto _val
							= test_enum(enum_tag<TestStruct4>{}, &struct4_instances, "TestString");
						test_enum _val2(std::move(_val));
						val = std::move(_val2);
					}

					CHECK(val.is_variant(1_usize));
					CHECK_EQ(val.get_index(), 1_usize);
					CHECK_FALSE(val.is_valueless());

					const auto& new_val = val.get<TestStruct4>();
					CHECK_EQ((*new_val.val1), 1_i64);
					CHECK_EQ((*new_val.val1), struct4_instances);
					CHECK_EQ(struct3_instances, 0_usize);
					CHECK_EQ(new_val.val2, std::string("TestString"));

					SUBCASE("Inspection") {
						enum_inspect(val) {
							enum_variant(TestStruct3) {
								// we shouldn't get here
								CHECK_FALSE(true);
							}
							enum_variant(TestStruct4, const auto& [vali64ptr, valstr]) {
								CHECK_EQ((*vali64ptr), 1_i64);
								CHECK_EQ(valstr, std::string("TestString"));
							}
						}
					}

					SUBCASE("Match") {
						val.match(
							[]([[maybe_unused]] const TestStruct3& val1) noexcept -> void {
								// we shouldn't get here
								CHECK_FALSE(true);
							},
							[](const TestStruct4& val2) noexcept -> void {
								CHECK_EQ((*val2.val1), 1_i64);
								CHECK_EQ(val2.val2, std::string("TestString"));
							});
					}
				}
			}

			CHECK_EQ(struct3_instances, 0_usize);
			CHECK_EQ(struct4_instances, 0_i64);
		}
	}
} // namespace hyperion
