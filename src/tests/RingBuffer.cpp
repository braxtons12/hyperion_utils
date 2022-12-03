/// @file RingBuffer.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for RingBuffer
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
#include <Hyperion/RingBuffer.h>
#include <Hyperion/Testing.h>

namespace hyperion {
	namespace detail::ringbuffer::test {
		IGNORE_UNUSED_MEMBER_FUNCTIONS_START
		class TestClass {
		  public:
			TestClass() noexcept = default;
			explicit TestClass(i32 val) noexcept : m_ptr(std::make_shared<i32>(val)) {
			}
			TestClass(const TestClass&) noexcept = default;
			TestClass(TestClass&&) noexcept = default;
			~TestClass() noexcept = default;
			auto operator=(const TestClass&) noexcept -> TestClass& = default;
			auto operator=(TestClass&&) noexcept -> TestClass& = default;

			// NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions))
			operator i32() const noexcept {
				return *m_ptr;
			}

			auto operator<=>(const TestClass& rhs) const noexcept -> std::strong_ordering {
				return *m_ptr <=> *(rhs.m_ptr);
			}

			auto operator<=>(i32 rhs) const noexcept -> std::strong_ordering {
				return *m_ptr <=> rhs;
			}

			auto operator==(const TestClass& rhs) const noexcept -> bool {
				return (*this <=> rhs) == std::strong_ordering::equal;
			}

			auto operator!=(const TestClass& rhs) const noexcept -> bool {
				return (*this <=> rhs) != std::strong_ordering::equal;
			}

			auto operator==(i32 rhs) const noexcept -> bool {
				return (*this <=> rhs) == std::strong_ordering::equal;
			}

			auto operator!=(i32 rhs) const noexcept -> bool {
				return (*this <=> rhs) != std::strong_ordering::equal;
			}

			std::shared_ptr<i32> m_ptr;
		};
		IGNORE_UNUSED_MEMBER_FUNCTIONS_STOP

	} // namespace detail::ringbuffer::test

	// NOLINTNEXTLINE
	TEST_SUITE("RingBuffer") {
		// NOLINTNEXTLINE(readability-function-cognitive-complexity)
		TEST_CASE("DefaultConstructed") {
			auto buffer = RingBuffer<detail::ringbuffer::test::TestClass>();

			constexpr auto capacity
				= RingBuffer<detail::ringbuffer::test::TestClass>::DEFAULT_CAPACITY;

			SUBCASE("accessors") {
				CHECK_EQ(buffer.size(), 0_usize);
				CHECK_EQ(buffer.capacity(), capacity);
				CHECK(buffer.empty());

				for(const auto& elem : buffer) {
					CHECK_EQ(elem, 0_i32);
				}
			}

			SUBCASE("push_back") {
				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
					buffer.push_back(detail::ringbuffer::test::TestClass(i));
				}

				SUBCASE("at") {
					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
						CHECK_EQ(buffer.at(i), i);
					}
				}

				SUBCASE("looping") {
					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
						buffer.push_back(detail::ringbuffer::test::TestClass(i));
					}

					auto value = static_cast<i32>(capacity) - 1_i32;
					for(auto i = 0_usize; i < capacity; ++i, --value) {
						CHECK_EQ(buffer.at(i), value);
					}
				}
			}

			SUBCASE("emplace_back") {
				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
					buffer.emplace_back(i);
				}

				SUBCASE("at") {
					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
						CHECK_EQ(buffer.at(i), i);
					}
				}

				SUBCASE("looping") {
					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
						buffer.emplace_back(i);
					}

					auto value = static_cast<i32>(capacity) - 1_i32;
					for(auto i = 0_usize; i < capacity; ++i, --value) {
						CHECK_EQ(buffer.at(i), value);
					}
				}
			}

			SUBCASE("reserve") {
				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
					buffer.emplace_back(i);
				}

				const auto new_capacity = capacity * 2_usize;
				buffer.reserve(new_capacity);

				CHECK_EQ(buffer.capacity(), new_capacity);
				CHECK_EQ(buffer.size(), capacity);

				for(auto i = 0_usize; i < capacity; ++i) {
					CHECK_EQ(buffer.at(i), static_cast<i32>(i));
				}

				SUBCASE("looping") {
					for(auto i = capacity; i < new_capacity + capacity; ++i) {
						buffer.emplace_back(i);
					}

					for(auto i = 0_usize; i < new_capacity; ++i) {
						CHECK_EQ(buffer.at(i), static_cast<i32>(i + capacity));
					}

					for(auto i = 0_usize; i < new_capacity; ++i) {
						buffer.emplace_back(i);
					}

					for(auto i = 0_usize; i < new_capacity; ++i) {
						CHECK_EQ(buffer.at(i), static_cast<i32>(i));
					}
				}
			}

			SUBCASE("front") {
				buffer.emplace_back(2);
				CHECK_EQ(buffer.front(), 2);
			}

			SUBCASE("back") {
				buffer.emplace_back(2);
				buffer.emplace_back(1);
				CHECK_EQ(buffer.back(), 1);
			}

			SUBCASE("insert") {
				SUBCASE("single") {
					buffer.emplace_back(2);
					buffer.insert(buffer.begin(), detail::ringbuffer::test::TestClass(1));

					CHECK_EQ(buffer.front(), 1);
					CHECK_EQ(buffer.back(), 2);
				}

				SUBCASE("looping") {
					for(auto i = 0_usize; i < capacity; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					buffer.insert(buffer.begin(),
								  detail::ringbuffer::test::TestClass(static_cast<i32>(capacity)));

					CHECK_EQ(buffer.front(), static_cast<i32>(capacity));
					CHECK_EQ(buffer.at(1), 0);
					CHECK_EQ(buffer.back(), static_cast<i32>(capacity - 2));
				}

				SUBCASE("looping_in_middle") {
					const auto num_writes = (capacity * 3) / 2;

					for(auto i = 0_usize; i < num_writes; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					buffer.insert(
						buffer.begin(),
						detail::ringbuffer::test::TestClass(static_cast<i32>(num_writes)));

					CHECK_EQ(buffer.front(), static_cast<i32>(num_writes));
					CHECK_EQ(buffer.at(1), static_cast<i32>(num_writes - capacity));
					CHECK_EQ(buffer.back(), static_cast<i32>(num_writes - 2));
				}
			}

			SUBCASE("insert_emplace") {
				SUBCASE("single") {
					buffer.emplace_back(2);
					buffer.insert_emplace(buffer.begin(), 1);

					CHECK_EQ(buffer.front(), 1);
					CHECK_EQ(buffer.back(), 2);
				}

				SUBCASE("looping") {
					for(auto i = 0_usize; i < capacity; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					buffer.insert_emplace(buffer.begin(), static_cast<i32>(capacity));

					CHECK_EQ(buffer.front(), static_cast<i32>(capacity));
					CHECK_EQ(buffer.at(1), 0);
					CHECK_EQ(buffer.back(), static_cast<i32>(capacity - 2));
				}

				SUBCASE("looping_in_middle") {
					const auto num_writes = (capacity * 3) / 2;

					for(auto i = 0_usize; i < num_writes; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					buffer.insert_emplace(buffer.begin(), static_cast<i32>(num_writes));

					CHECK_EQ(buffer.front(), static_cast<i32>(num_writes));
					CHECK_EQ(buffer.at(1), static_cast<i32>(num_writes - capacity));
					CHECK_EQ(buffer.back(), static_cast<i32>(num_writes - 2));
				}
			}

			SUBCASE("erase") {
				SUBCASE("not_full") {
					buffer.emplace_back(3);
					// NOLINTNEXTLINE (magic numbers)
					buffer.emplace_back(5);

					CHECK_EQ(buffer.size(), 2_usize);
					CHECK_EQ(buffer.back(), 5);

					auto iter = buffer.erase(buffer.end() - 1);
					CHECK_EQ(buffer.size(), 1_usize);
					CHECK_EQ(buffer.back(), 3);
					CHECK_EQ(iter, buffer.end());
				}

				SUBCASE("full") {
					for(auto i = 0_usize; i < capacity; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					SUBCASE("from_end") {
						const auto size = buffer.size();
						const auto iter = buffer.erase(buffer.end() - 1);
						CHECK_EQ(buffer.size(), capacity - 1);
						CHECK_EQ(buffer.back(), static_cast<i32>(capacity - 2));
						CHECK_EQ(iter, (buffer.begin() + (size - 1)));
					}

					SUBCASE("from_middle") {
						const auto size = buffer.size();
						const auto iter = buffer.erase(buffer.end() - 4);
						CHECK_EQ(buffer.size(), capacity - 1);
						CHECK_EQ(buffer.back(), static_cast<i32>(capacity - 1));
						CHECK_EQ(iter, (buffer.begin() + (size - 4)));
					}

					SUBCASE("full_plus_half_capacity") {
						const auto total_writes = (capacity * 3) / 2;
						for(auto i = capacity; i < total_writes; ++i) {
							buffer.emplace_back(static_cast<i32>(i));
						}

						CHECK_EQ(buffer.size(), capacity);
						CHECK_EQ(buffer.back(), static_cast<i32>(total_writes - 1));

						SUBCASE("from_end") {
							const auto size = buffer.size();
							const auto iter = buffer.erase(buffer.end() - 1);

							CHECK_EQ(buffer.size(), capacity - 1);
							CHECK_EQ(buffer.back(), static_cast<i32>(total_writes - 2));
							CHECK_EQ(iter, (buffer.begin() + (size - 1)));
						}

						SUBCASE("from_middle") {
							const auto size = buffer.size();
							const auto iter = buffer.erase(buffer.end() - 4);

							CHECK_EQ(buffer.size(), capacity - 1);
							CHECK_EQ(buffer.back(), static_cast<i32>(total_writes - 1));
							CHECK_EQ(iter, (buffer.begin() + (size - 4)));
						}
					}
				}
			}

			SUBCASE("erase_range") {
				SUBCASE("not_full") {
					buffer.emplace_back(3);
					// NOLINTNEXTLINE (magic numbers)
					buffer.emplace_back(5);
					// NOLINTNEXTLINE (magic numbers)
					buffer.emplace_back(6);

					CHECK_EQ(buffer.size(), 3_usize);
					CHECK_EQ(buffer.back(), 6);

					auto iter = buffer.erase(buffer.begin() + 1, buffer.end());
					CHECK_EQ(buffer.size(), 1_usize);
					CHECK_EQ(buffer.back(), 3);
					CHECK_EQ(iter, buffer.end());
				}

				SUBCASE("full") {
					for(auto i = 0_usize; i < capacity; ++i) {
						buffer.emplace_back(static_cast<i32>(i));
					}

					SUBCASE("from_end") {
						const auto iter = buffer.erase(buffer.end() - 2, buffer.end());
						CHECK_EQ(buffer.size(), capacity - 2);
						CHECK_EQ(buffer.back(), static_cast<i32>(capacity - 3));
						CHECK_EQ(iter, buffer.end());
					}

					SUBCASE("full_plus_half_capacity") {
						const auto total_writes = (capacity * 3) / 2;
						for(auto i = capacity; i < total_writes; ++i) {
							buffer.emplace_back(static_cast<i32>(i));
						}

						CHECK_EQ(buffer.size(), capacity);
						CHECK_EQ(buffer.back(), static_cast<i32>(total_writes - 1));

						SUBCASE("from_end") {
							const auto start_erase_index = 5;
							const auto num_to_erase = 5;
							const auto start_iter = buffer.begin() + start_erase_index;
							const auto end_iter = start_iter + num_to_erase;
							const auto val_to_compare = buffer.at(start_erase_index + num_to_erase);
							const auto back_val = buffer.back();
							const auto front_val = buffer.front();

							const auto iter = buffer.erase(start_iter, end_iter);

							CHECK_EQ(buffer.size(), capacity - num_to_erase);
							CHECK_EQ(buffer.back(), back_val);
							CHECK_EQ(buffer.front(), front_val);
							CHECK_EQ(buffer.at(start_erase_index), val_to_compare);
							CHECK_EQ(iter, buffer.begin() + start_erase_index);
						}
					}
				}
			}

			SUBCASE("pop_back") {
				buffer.emplace_back(1);
				buffer.emplace_back(2);

				CHECK_EQ(buffer.size(), 2_usize);
				CHECK_EQ(buffer.back(), 2);
				CHECK_EQ(buffer.front(), 1);

				auto back = buffer.pop_back();

				CHECK(back.is_some());
				CHECK_EQ(back.unwrap(), 2);

				CHECK_EQ(buffer.size(), 1_usize);
				CHECK_EQ(buffer.back(), 1);
				CHECK_EQ(buffer.front(), 1);
			}

			SUBCASE("pop_front") {
				buffer.emplace_back(1);
				buffer.emplace_back(2);

				CHECK_EQ(buffer.size(), 2_usize);
				CHECK_EQ(buffer.back(), 2);
				CHECK_EQ(buffer.front(), 1);

				auto front = buffer.pop_front();

				CHECK(front.is_some());
				CHECK_EQ(front.unwrap(), 1);

				CHECK_EQ(buffer.size(), 1_usize);
				CHECK_EQ(buffer.back(), 2);
				CHECK_EQ(buffer.front(), 2);
			}
		}
	}
} // namespace hyperion
