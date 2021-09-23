#pragma once
#include "HyperionUtils/ChangeDetector.h"
#include "HyperionUtils/Memory.h"
#include "microTest.h"

namespace hyperion::test {

	const suite ChangeDetectorTests = [] { // NOLINT
		"constructor"_test = [] {
			auto detector = ChangeDetector<bool>();

			expect(detector.value() == FALSE);
		};

		"changedLValue"_test = [] {
			auto detector = ChangeDetector<bool>();
			auto newTrue = true;
			expect(detector.changed(newTrue) == TRUE);
		};

		"changedRValue"_test = [] {
			auto detector = ChangeDetector<bool>();

			expect(detector.changed(true) == TRUE);
		};

		"changedPointer"_test = [] {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* initialValue = new bool(false);
			auto detector = ChangeDetector<bool*>(initialValue);

			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			auto* newTrue = new bool(true);
			expect(detector.changed(newTrue) == TRUE);

			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			delete initialValue;
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			delete newTrue;
		};
	};
} // namespace hyperion::test
