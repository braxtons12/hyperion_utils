#pragma once

#include <Hyperion/RingBuffer.h>

#include "microTest.h"

namespace hyperion::test {

	const suite RingBufferTests = [] { // NOLINT
		"defaults"_test = [] {
			auto buffer = RingBuffer<bool, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;
			expect(buffer.size() == 0_ul);
			expect(buffer.capacity() == _ul(capacity));
			expect(buffer.empty() == TRUE);

			for(auto& elem : buffer) {
				expect(elem == FALSE);
			}
		};

		"initialCapacity"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto buffer = RingBuffer<bool, RingBufferType::NotThreadSafe>(32U);
			expect(buffer.size() == 0_ul);
			expect(buffer.capacity() == 32_ul);
			expect(buffer.empty() == TRUE);

			for(auto& elem : buffer) {
				expect(elem == FALSE);
			}
		};

		"initialCapacityAndValue"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto buffer = RingBuffer<bool, RingBufferType::NotThreadSafe>(32U, true);
			expect(buffer.size() == 32_ul);
			expect(buffer.capacity() == 32_ul);

			for(auto& elem : buffer) {
				expect(elem == TRUE);
			}
		};

		"pushBackAndAt"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.push_back(i);
			}

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				expect(buffer.at(i) == _i(i));
			}
		};

		"pushBackAndAtLooping"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.push_back(i);
			}
			for(auto i = static_cast<int>(capacity - 1); i >= 0; --i) {
				buffer.push_back(i);
			}

			for(auto i = 0, j = static_cast<int>(capacity - 1); i < static_cast<int>(capacity);
				++i, --j) {
				expect(buffer.at(i) == _i(j));
			}
		};

		"emplaceBackAndAt"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.emplace_back(i);
			}

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				expect(buffer.at(i) == _i(i));
			}
		};

		"emplaceBackAndAtLooping"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.emplace_back(i);
			}
			for(auto i = static_cast<int>(capacity - 1); i >= 0; --i) {
				buffer.emplace_back(i);
			}

			for(auto i = 0, j = static_cast<int>(capacity - 1); i < static_cast<int>(capacity);
				++i, --j) {
				expect(buffer.at(i) == _i(j));
			}
		};

		"reserveAndLooping"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			uint32_t initialCapacity = 8U;
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>(initialCapacity);
			expect(buffer.size() == 0_ul);
			expect(buffer.empty() == TRUE);
			expect(buffer.capacity() == _ul(initialCapacity));

			for(auto i = 0ULL; i < initialCapacity; ++i) {
				buffer.push_back(static_cast<int>(i));
			}
			for(auto i = 0ULL; i < initialCapacity; ++i) {
				expect(buffer.at(i) == _i(static_cast<int>(i)));
			}
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto newCapacity = 16U;
			buffer.reserve(newCapacity);
			for(auto i = 0ULL; i < initialCapacity; ++i) {
				expect(buffer.at(i) == _i(static_cast<int>(i)));
			}
			for(auto i = initialCapacity; i < newCapacity + initialCapacity; ++i) {
				buffer.push_back(static_cast<int>(i));
			}
			for(auto i = 0ULL; i < newCapacity; ++i) {
				expect(buffer.at(i) == _i(static_cast<int>(i + initialCapacity)));
			}
			for(auto i = 0ULL; i < newCapacity; ++i) {
				buffer.push_back(static_cast<int>(i));
			}
			for(auto i = 0ULL; i < newCapacity; ++i) {
				expect(buffer.at(i) == _i(static_cast<int>(i)));
			}
		};

		"front"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(2);
			expect(buffer.front() == 2_i);
		};

		"back"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(2);
			buffer.push_back(1);
			expect(buffer.back() == 1_i);
		};

		"insert"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(2);
			buffer.insert(buffer.begin(), 1);
			expect(buffer.front() == 1_i);
			expect(buffer.back() == 2_i);
		};

		"insertLooped"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0ULL; i < capacity; ++i) {
				buffer.push_back(static_cast<int>(i));
			}

			buffer.insert(buffer.begin(), static_cast<int>(capacity));
			expect(buffer.front() == _i(static_cast<int>(capacity)));
			expect(buffer.at(1) == 0_i);
			expect(buffer.back() == _i(static_cast<int>(capacity - 2)));
		};

		"insertLoopedAndAHalf"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto numWrites = static_cast<int>(capacity * 1.5);
			for(auto i = 0; i < numWrites; ++i) {
				buffer.push_back(i);
			}

			buffer.insert(buffer.begin(), numWrites);
			expect(buffer.front() == _i(numWrites));
			expect(buffer.at(1) == _i(numWrites - static_cast<int>(capacity)));
			expect(buffer.back() == _i(numWrites - 2));
		};

		"insertEmplace"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(2);
			buffer.insert_emplace(buffer.begin(), 1);
			expect(buffer.front() == 1_i);
			expect(buffer.back() == 2_i);
		};

		"insertEmplaceLooped"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0ULL; i < capacity; ++i) {
				buffer.push_back(static_cast<int>(i));
			}

			buffer.insert_emplace(buffer.begin(), static_cast<int>(capacity));
			expect(buffer.front() == _i(static_cast<int>(capacity)));
			expect(buffer.at(1) == 0_i);
			expect(buffer.back() == _i(static_cast<int>(capacity - 2)));
		};

		"insertEmplaceLoopedAndAHalf"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto numWrites = static_cast<int>(capacity * 1.5);
			for(auto i = 0; i < numWrites; ++i) {
				buffer.push_back(i);
			}

			buffer.insert_emplace(buffer.begin(), numWrites);
			expect(buffer.front() == _i(numWrites));
			expect(buffer.at(1) == _i(numWrites - static_cast<int>(capacity)));
			expect(buffer.back() == _i(numWrites - 2));
		};

		"erase"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(3);
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			buffer.push_back(5);

			expect(buffer.size() == 2_ul);
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			expect(buffer.back() == 5_i);
			auto iter = buffer.erase(buffer.end() - 1);
			expect(buffer.size() == 1_ul);
			expect(buffer.back() == 3_i);
			expect((iter == buffer.end()) == TRUE);
		};

		"eraseFullFromEnd"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 1));
			const auto size = buffer.size();
			auto iter = buffer.erase(buffer.end() - 1);
			expect(buffer.size() == _ul(static_cast<int>(capacity) - 1));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 2));
			expect((iter == (buffer.begin() + (size - 1))) == TRUE);
		};

		"eraseFullFromMiddle"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 1));
			const auto size = buffer.size();
			auto iter = buffer.erase(buffer.end() - 4);
			expect(buffer.size() == _ul(static_cast<int>(capacity) - 1));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 1));
			expect((iter == (buffer.begin() + (size - 4))) == TRUE);
		};

		"eraseFullAndAHalfFromEnd"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto numWrites = static_cast<int>(capacity * 1.5);
			for(auto i = 0; i < numWrites; ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(numWrites - 1));
			const auto size = buffer.size();
			auto iter = buffer.erase(buffer.end() - 1);
			expect(buffer.size() == _ul(static_cast<int>(capacity) - 1));
			expect(buffer.back() == _i(numWrites - 2));
			expect((iter == (buffer.begin() + (size - 1))) == TRUE);
		};

		"eraseFullAndAHalfFromMiddle"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto numWrites = static_cast<int>(capacity * 1.5);
			for(auto i = 0; i < numWrites; ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(numWrites - 1));
			const auto size = buffer.size();
			auto iter = buffer.erase(buffer.end() - 4);
			expect(buffer.size() == _ul(static_cast<int>(capacity) - 1));
			expect(buffer.back() == _i(numWrites - 1));
			expect((iter == (buffer.begin() + (size - 4))) == TRUE);
		};

		"eraseRange"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(3);
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			buffer.push_back(5);
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			buffer.push_back(6);

			expect(buffer.size() == 3_ul);
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			expect(buffer.back() == 6_i);
			auto iter = buffer.erase(buffer.begin() + 1, buffer.end());
			expect(buffer.size() == 1_ul);
			expect(buffer.back() == 3_i);
			expect((iter == buffer.end()) == TRUE);
		};

		"eraseRangeFull"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			for(auto i = 0; i < static_cast<int>(capacity); ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 1));
			auto iter = buffer.erase(buffer.end() - 2, buffer.end());
			expect(buffer.size() == _ul(static_cast<int>(capacity) - 2));
			expect(buffer.back() == _i(static_cast<int>(capacity) - 3));
			expect((iter == buffer.end()) == TRUE);
		};

		"eraseRangeFullAndAHalf"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();
			constexpr auto capacity
				= RingBuffer<int, RingBufferType::NotThreadSafe>::DEFAULT_CAPACITY;

			// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
			auto numWrites = static_cast<int>(capacity * 1.5);
			for(auto i = 0; i < numWrites; ++i) {
				buffer.push_back(i);
			}

			expect(buffer.size() == _ul(static_cast<int>(capacity)));
			expect(buffer.back() == _i(numWrites - 1));
			const auto startEraseIndex = 5;
			const auto numToErase = 5;
			const auto startIter = buffer.begin() + startEraseIndex;
			const auto endIter = startIter + numToErase;
			const auto valToCompare = buffer.at(startEraseIndex + numToErase);
			const auto backVal = buffer.back();
			const auto frontVal = buffer.front();

			const auto iter = buffer.erase(startIter, endIter);

			expect(buffer.size() == _ul(static_cast<int>(capacity) - numToErase));
			expect(buffer.back() == _i(backVal));
			expect(buffer.front() == _i(frontVal));
			expect(buffer.at(startEraseIndex) == _i(valToCompare));
			expect((iter == (buffer.begin() + startEraseIndex)) == TRUE);
		};

		"popBack"_test = [] {
			auto buffer = RingBuffer<int, RingBufferType::NotThreadSafe>();

			buffer.push_back(1);
			buffer.push_back(2);
			expect(buffer.size() == 2_ul);
			expect(buffer.back() == 2_i);
			expect(buffer.pop_back() == 2_i);
			expect(buffer.size() == 1_ul);
			expect(buffer.back() == 1_i);
		};
	};
} // namespace hyperion::test
