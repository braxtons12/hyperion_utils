#pragma once

#include <tuple>

#include "HyperionUtils/Error.h"
#include "HyperionUtils/Monads.h"
#include "gtest/gtest.h"

namespace hyperion::test {

	TEST(OptionTest, someMapping) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		ASSERT_TRUE(
			some.map([](const bool some_value) noexcept -> bool { return some_value; }).is_some());
		ASSERT_FALSE(
			some.map([](const bool some_value) noexcept -> bool { return some_value; }).is_none());
		ASSERT_TRUE(
			some.map_or([](const bool some_value) noexcept -> bool { return some_value; }, false));
		ASSERT_TRUE(
			some.map_or_else([](const bool some_value) noexcept -> bool { return some_value; },
							 []() noexcept -> bool { return false; }));
	}

	TEST(OptionTest, noneMapping) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_TRUE(
			none.map([](const bool some_value) noexcept -> bool { return some_value; }).is_none());
		ASSERT_FALSE(
			none.map([](const bool some_value) noexcept -> bool { return some_value; }).is_some());
		ASSERT_FALSE(
			none.map_or([](const bool some_value) noexcept -> bool { return some_value; }, false));
		ASSERT_FALSE(
			none.map_or_else([](const bool some_value) noexcept -> bool { return some_value; },
							 []() noexcept -> bool { return false; }));
	}

	TEST(OptionTest, someOkOrValue) {
		auto some = Some(true);
		auto error = Error("TestErrorMessage");

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());

		auto res = some.ok_or(std::move(error));
		ASSERT_TRUE(res.is_ok());
		ASSERT_TRUE(res.unwrap());
	}

	TEST(OptionTest, someOkOrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		auto some = Some(value);
		auto error = Error("TestErrorMessage");

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());

		auto res = some.ok_or(std::move(error));
		ASSERT_TRUE(res.is_ok());
		auto ok = res.ok();
		ASSERT_TRUE(ok.is_some());
		auto* unwrapped = ok.unwrap();
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, noneOkOrValue) {
		Option<bool> none = None();
		auto error = Error("TestErrorMessage");

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());

		auto res = none.ok_or(std::move(error));
		ASSERT_TRUE(res.is_err());
		ASSERT_TRUE(res.unwrap_err().message() == std::string("TestErrorMessage"));
	}

	TEST(OptionTest, someOkOrElseValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());

		auto res = some.ok_or_else([]() { return Error("TestErrorMessage"); });
		ASSERT_TRUE(res.is_ok());
		ASSERT_TRUE(res.unwrap());
	}

	TEST(OptionTest, someOkOrElsePointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		auto some = Some(value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());

		auto res = some.ok_or_else([]() { return Error("TestErrorMessage"); });
		ASSERT_TRUE(res.is_ok());
		auto* unwrapped = res.unwrap();
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, noneOkOrElseValue) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());

		auto res = none.ok_or_else([]() { return Error("TestErrorMessage"); });
		ASSERT_TRUE(res.is_err());
		ASSERT_TRUE(res.unwrap_err().message() == std::string("TestErrorMessage"));
	}

	TEST(OptionTest, someUnwrapValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		ASSERT_TRUE(some.unwrap());
	}

	TEST(OptionTest, someUnwrapPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		auto some = Some(value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());

		auto* unwrapped = some.unwrap();
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, noneUnwrap) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());

		ASSERT_DEATH(ignore(none.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(OptionTest, someUnwrapOrValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		ASSERT_TRUE(some.unwrap_or(false));
	}

	TEST(OptionTest, someUnwrapOrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* some_value = new bool(true);
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* none_value = new bool(false);
		auto some = Some(some_value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		auto* unwrapped = some.unwrap_or(none_value);
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, noneUnwrapOrValue) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_FALSE(none.unwrap_or(false));
	}

	TEST(OptionTest, noneUnwrapOrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* none_value = new bool(false);
		Option<bool*> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		auto* unwrapped = none.unwrap_or(none_value);
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_FALSE(*unwrapped);
	}

	TEST(OptionTest, someUnwrapOrElseValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		ASSERT_TRUE(some.unwrap_or_else([]() { return false; }));
	}

	TEST(OptionTest, someUnwrapOrElsePointer) {

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* some_value = new bool(true);
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* none_value = new bool(false);
		auto some = Some(some_value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		auto* unwrapped = some.unwrap_or_else([none_value]() { return none_value; });
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, noneUnwrapOrElseValue) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_FALSE(none.unwrap_or_else([]() { return false; }));
	}

	TEST(OptionTest, noneUnwrapOrElsePointer) {
		Option<bool*> none = None();
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* none_value = new bool(false);

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		auto* unwrapped = none.unwrap_or_else([none_value]() { return none_value; });
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_FALSE(*unwrapped);
	}

	TEST(OptionTest, someAsMutValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		auto* gotten_mut = some.as_mut();
		ASSERT_TRUE(*gotten_mut);
		*gotten_mut = false;
		gotten_mut = some.as_mut();
		ASSERT_FALSE(*gotten_mut);
	}

	TEST(OptionTest, someAsMutPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* some_value = new bool(true);
		auto some = Some(some_value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		auto* gotten_mut = some.as_mut();
		ASSERT_TRUE(*gotten_mut);
		*gotten_mut = false;
		gotten_mut = some.as_mut();
		ASSERT_FALSE(*gotten_mut);
	}

	TEST(OptionTest, noneAsMutValue) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_DEATH(ignore(none.as_mut()), "as_mut called on a None, terminating");
	}

	TEST(OptionTest, noneAsMutPointer) {
		Option<bool*> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_DEATH(ignore(none.as_mut()), "as_mut called on a None, terminating");
	}

	TEST(OptionTest, someAsConstValue) {
		auto some = Some(true);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		const auto* gotten_const = some.as_const();
		ASSERT_TRUE(*gotten_const);
		//*gotten_const = false; won't compile, as desired
	}

	TEST(OptionTest, someAsConstPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* some_value = new bool(true);
		auto some = Some(some_value);

		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		const auto* gotten_const = some.as_const();
		ASSERT_TRUE(*gotten_const);
		//*gotten_const = false; won't compile, as desired
	}

	TEST(OptionTest, noneAsConstValue) {
		Option<bool> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_DEATH(ignore(none.as_const()), "as_const called on a None, terminating");
	}

	TEST(OptionTest, noneAsConstPointer) {
		Option<bool*> none = None();

		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_DEATH(ignore(none.as_const()), "as_const called on a None, terminating");
	}

	// NOLINTNEXTLINE(misc-definitions-in-headers)
	auto some_move_test(Option<bool*>&& some) noexcept -> void {
		ASSERT_TRUE(some.is_some());
		ASSERT_FALSE(some.is_none());
		auto* unwrapped = some.unwrap();
		ASSERT_TRUE(unwrapped != nullptr);
		ASSERT_TRUE(*unwrapped);
	}

	TEST(OptionTest, someMovePointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* some_value = new bool(true);
		{
			auto some = Some(some_value);
			some_move_test(std::move(some));
		}
	}

	// NOLINTNEXTLINE(misc-definitions-in-headers, readability-function-cognitive-complexity)
	auto none_move_test(Option<bool*>&& none) noexcept -> void {
		ASSERT_TRUE(none.is_none());
		ASSERT_FALSE(none.is_some());
		ASSERT_DEATH(ignore(none.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(OptionTest, noneMovePointer) {
		Option<bool*> none = None();
		none_move_test(std::move(none));
	}
} // namespace hyperion::test
