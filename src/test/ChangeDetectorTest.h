#pragma once
#include "../../include/ChangeDetector.h"
#include "gtest/gtest.h"

namespace hyperion::utils::test {

	TEST(ChangeDetectorTest, constructor) {
		auto detector = ChangeDetector<bool>();

		ASSERT_FALSE(detector.value());
	}

	TEST(ChangeDetectorTest, changedLValue) {
		auto detector = ChangeDetector<bool>();
		auto newTrue = true;
		auto newFalse = false;
		if(detector.value()) {
			ASSERT_TRUE(detector.changed(newFalse));
		}
		else {
			ASSERT_TRUE(detector.changed(newTrue));
		}
	}

	TEST(ChangeDetectorTest, changedRValue) {
		auto detector = ChangeDetector<bool>();

		if(detector.value()) {
			ASSERT_TRUE(detector.changed(false));
		}
		else {
			ASSERT_TRUE(detector.changed(true));
		}
	}

	TEST(ChangeDetectorTest, changedPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* initialValue = new bool(false);
		auto detector = ChangeDetector<bool*>(initialValue);

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* newTrue = new bool(true);
		ASSERT_TRUE(detector.changed(newTrue));

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete initialValue;
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete newTrue;
	}
} // namespace hyperion::utils::test
