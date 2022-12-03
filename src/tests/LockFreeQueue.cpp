/// @file LockFreeQueue.cpp
/// @author Braxton Salyer <braxtonsalyer@gmail.com>
/// @brief Tests for LockFreeQueu
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
#include <Hyperion/LockFreeQueue.h>
#include <Hyperion/Testing.h>

namespace hyperion { // NOLINT
	namespace detail::lock_free_queue::test {
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

			auto operator=(i32 val) noexcept -> TestClass& {
				if(m_ptr != nullptr) {
					*m_ptr = val;
				}
				else {
					m_ptr = std::make_shared<i32>(val);
				}

				return *this;
			}

			// NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions))
			explicit(false) operator i32() const noexcept {
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
	} // namespace detail::lock_free_queue::test

	//	// NOLINTNEXTLINE
	//	TEST_SUITE("LockFreeQueue") {
	//		// NOLINTNEXTLINE
	//		TEST_CASE("DefaultConstructed") {
	//			auto buffer = LockFreeQueue<detail::lock_free_queue::test::TestClass>();
	//
	//			constexpr auto capacity
	//				= LockFreeQueue<detail::lock_free_queue::test::TestClass>::DEFAULT_CAPACITY;
	//
	//			SUBCASE("accessors") {
	//				CHECK_EQ(buffer.size(), 0_usize);
	//				CHECK_EQ(buffer.capacity(), capacity);
	//				CHECK(buffer.empty());
	//			}
	//
	//			SUBCASE("push_back") {
	//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
	//					buffer.push_back(detail::lock_free_queue::test::TestClass(i));
	//				}
	//
	//				SUBCASE("at") {
	//					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
	//						auto front = buffer.pop_front();
	//						CHECK(front.is_some());
	//						CHECK_EQ(front.unwrap(), i);
	//					}
	//				}
	//
	//				SUBCASE("looping") {
	//					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
	//						buffer.push_back(detail::lock_free_queue::test::TestClass(i));
	//					}
	//
	//					auto value = static_cast<i32>(capacity) - 1_i32;
	//					for(auto i = 0_usize; i < capacity; ++i, --value) {
	//						auto front = buffer.pop_front();
	//						CHECK(front.is_some());
	//						CHECK_EQ(front.unwrap(), value);
	//					}
	//				}
	//			}
	//
	//			SUBCASE("emplace_back") {
	//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
	//					buffer.emplace_back(i);
	//				}
	//
	//				SUBCASE("at") {
	//					for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
	//						auto front = buffer.pop_front();
	//						CHECK(front.is_some());
	//						CHECK_EQ(front.unwrap(), i);
	//					}
	//				}
	//
	//				SUBCASE("looping") {
	//					for(auto i = static_cast<i32>(capacity) - 1_i32; i >= 0; --i) {
	//						buffer.emplace_back(i);
	//					}
	//
	//					auto value = static_cast<i32>(capacity) - 1_i32;
	//					for(auto i = 0_usize; i < capacity; ++i, --value) {
	//						auto front = buffer.pop_front();
	//						CHECK(front.is_some());
	//						CHECK_EQ(front.unwrap(), value);
	//					}
	//				}
	//			}
	//
	//			SUBCASE("reserve") {
	//				for(auto i = 0_i32; i < static_cast<i32>(capacity); ++i) {
	//					buffer.emplace_back(i);
	//				}
	//
	//				const auto new_capacity = capacity * 2_usize;
	//				buffer.reserve(new_capacity);
	//
	//				CHECK_EQ(buffer.capacity(), new_capacity);
	//				CHECK_EQ(buffer.size(), capacity);
	//			}
	//
	//			SUBCASE("front") {
	//				buffer.emplace_back(2);
	//				CHECK_EQ(buffer.front().as_cref(), 2);
	//			}
	//
	//			SUBCASE("pop_front") {
	//				buffer.emplace_back(1);
	//				buffer.emplace_back(2);
	//
	//				CHECK_EQ(buffer.size(), 2_usize);
	//				CHECK_EQ(buffer.front().as_cref(), 1);
	//
	//				auto front = buffer.pop_front();
	//
	//				CHECK(front.is_some());
	//				CHECK_EQ(front.unwrap(), 1);
	//
	//				CHECK_EQ(buffer.size(), 1_usize);
	//				CHECK_EQ(buffer.front().as_cref(), 2);
	//			}
	//		}
	//	}
} // namespace hyperion
