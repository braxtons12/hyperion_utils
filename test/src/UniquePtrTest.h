#pragma once

#include <Hyperion/Memory.h>

#include "microTest.h"

namespace hyperion::test {

	const suite UniquePtrTest = [] { // NOLINT
		"constructorAndAssignment"_test = [] {
			auto ptr = UniquePtr<i32>();
			auto ptr2 = UniquePtr<i32>(new i32(0));
			auto ptr3 = hyperion::make_unique<i32>(2);

			expect((ptr == nullptr) == TRUE);
			expect((ptr2 != nullptr) == TRUE);
			expect((ptr3 != nullptr) == TRUE);
			expect(*ptr2 == 0_i);
			expect(*ptr3 == 2_i);

			auto ptr4 = std::move(ptr3);
			expect((ptr3 == nullptr) == TRUE); // NOLINT
			expect((ptr4 != nullptr) == TRUE);
			expect(*ptr4 == 2_i);
		};

		"accessorsAndModifiers"_test = [] {
			auto ptr = hyperion::make_unique<i32>(2);

			expect(static_cast<bool>(ptr) == TRUE);
			expect((ptr.get() != nullptr) == TRUE);
			expect(*ptr == 2_i);
			expect(*(ptr.get()) == 2_i); // NOLINT

			auto* ptr2 = ptr.release();
			expect((ptr.get() == nullptr) == TRUE);
			expect(static_cast<bool>(ptr) == FALSE);
			expect(*ptr2 == 2_i);

			*ptr2 = 3;
			ptr.reset(ptr2); // NOLINT
			expect((ptr.get() != nullptr) == TRUE);
			expect(static_cast<bool>(ptr) == TRUE);
			expect(*ptr == 3_i);
			expect(*(ptr.get()) == 3_i); // NOLINT
		};
	};
} // namespace hyperion::test
