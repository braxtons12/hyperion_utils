#pragma once

#include <tuple>

#include "HyperionUtils/Error.h"
#include "HyperionUtils/Monads.h"
#include "gtest/gtest.h"

namespace hyperion::test {

	TEST(ResultTest, okAsMutValue) {
		Result<bool, Error> ok = Ok(true);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(static_cast<bool>(ok));
		ASSERT_FALSE(ok.is_err());

		auto* gotten_mut = ok.as_mut();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_TRUE(*gotten_mut);
		*gotten_mut = false;
		gotten_mut = ok.as_mut();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_FALSE(*gotten_mut);
	}

	TEST(ResultTest, okAsMutPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(static_cast<bool>(ok));
		ASSERT_FALSE(ok.is_err());

		auto* gotten_mut = ok.as_mut();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_TRUE(*gotten_mut);
		*gotten_mut = false;
		gotten_mut = ok.as_mut();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_FALSE(*gotten_mut);
	}

	TEST(ResultTest, errAsMutValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(static_cast<bool>(err));
		ASSERT_DEATH(ignore(err.as_mut()), "as_mut called on an Error result, terminating");
	}

	TEST(ResultTest, errAsMutPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool*, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(static_cast<bool>(err));
		ASSERT_DEATH(ignore(err.as_mut()), "as_mut called on an Error result, terminating");
	}

	TEST(ResultTest, okAsConstValue) {
		Result<bool, Error> ok = Ok(true);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(static_cast<bool>(ok));
		ASSERT_FALSE(ok.is_err());

		auto* gotten_mut = ok.as_const();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_TRUE(*gotten_mut);
	}

	TEST(ResultTest, okAsConstPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(static_cast<bool>(ok));
		ASSERT_FALSE(ok.is_err());

		const auto* gotten_mut = ok.as_const();
		ASSERT_TRUE(gotten_mut != nullptr);
		ASSERT_TRUE(*gotten_mut);
	}

	TEST(ResultTest, errAsConstValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(static_cast<bool>(err));
		ASSERT_DEATH(ignore(err.as_const()), "as_const called on an Error result, terminating");
	}

	TEST(ResultTest, errAsConstPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool*, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(static_cast<bool>(err));
		ASSERT_DEATH(ignore(err.as_const()), "as_const called on an Error result, terminating");
	}

	TEST(ResultTest, okUnwrapValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_TRUE(ok.unwrap());
	}

	TEST(ResultTest, okUnwrapPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		auto* ptr = ok.unwrap();
		ASSERT_TRUE(*ptr);
	}

	TEST(ResultTest, errUnwrapValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);

		ASSERT_DEATH(ignore(err.unwrap()), "unwrap called on an Error result, terminating");
	}

	TEST(ResultTest, errUnwrapPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool*, Error> err = Err(std::move(error));

		ASSERT_TRUE(err.is_err());
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);

		ASSERT_DEATH(ignore(err.unwrap()), "unwrap called on an Error result, terminating");
	}

	TEST(ResultTest, okUnwrapOrValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_TRUE(ok.unwrap_or(false));
	}

	TEST(ResultTest, okUnwrapOrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value2 = new bool(false);
		ASSERT_TRUE(*ok.unwrap_or(value2));
	}

	TEST(ResultTest, errUnwrapOrValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_FALSE(err.unwrap_or(false));
	}

	TEST(ResultTest, errUnwrapOrPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool*, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value2 = new bool(false);
		ASSERT_FALSE(*err.unwrap_or(value2));
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value2;
	}

	TEST(ResultTest, okUnwrapOrElseValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_TRUE(ok.unwrap_or_else([]() noexcept -> bool { return false; }));
	}

	TEST(ResultTest, okUnwrapOrElsePointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value2 = new bool(false);
		ASSERT_TRUE(*ok.unwrap_or_else([val = value2]() noexcept -> bool* { return val; }));

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value;
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value2;
	}

	TEST(ResultTest, errUnwrapOrElseValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_FALSE(err.unwrap_or_else([]() noexcept -> bool { return false; }));
	}

	TEST(ResultTest, errUnwrapOrElsePointer) {
		auto error = Error("TestErrorMessage");
		Result<bool*, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(false);
		ASSERT_FALSE(*err.unwrap_or_else([val = value]() noexcept -> bool* { return val; }));

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value;
	}

	TEST(ResultTest, okUnwrapErrValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_DEATH(ignore(ok.unwrap_err()), "unwrap_err called on an Ok result, terminating");
	}

	TEST(ResultTest, okUnwrapErrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_DEATH(ignore(ok.unwrap_err()), "unwrap_err called on an Ok result, terminating");
	}

	TEST(ResultTest, errUnwrapErrValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.unwrap_err().message(), "TestErrorMessage");
	}

	TEST(ResultTest, errUnwrapErrPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.unwrap_err()->message(), "TestErrorMessage");
	}

	TEST(ResultTest, okokValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		auto maybe_ok = ok.ok();
		ASSERT_TRUE(maybe_ok.is_some());
		ASSERT_TRUE(maybe_ok.unwrap());
	}

	TEST(ResultTest, okokPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		auto maybe_ok = ok.ok();
		ASSERT_TRUE(maybe_ok.is_some());
		ASSERT_TRUE(*maybe_ok.unwrap());

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value;
	}

	TEST(ResultTest, errOkValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		auto maybe_ok = err.ok();
		ASSERT_TRUE(maybe_ok.is_none());
		ASSERT_DEATH(ignore(maybe_ok.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(ResultTest, errOkPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		auto maybe_ok = err.ok();
		ASSERT_TRUE(maybe_ok.is_none());
		ASSERT_DEATH(ignore(maybe_ok.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(ResultTest, okErrValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		auto maybe_ok = ok.err();
		ASSERT_TRUE(maybe_ok.is_none());
		ASSERT_DEATH(ignore(maybe_ok.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(ResultTest, okErrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		auto maybe_ok = ok.err();
		ASSERT_TRUE(maybe_ok.is_none());
		ASSERT_DEATH(ignore(maybe_ok.unwrap()), "unwrap called on a None, terminating");
	}

	TEST(ResultTest, errerrValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		auto maybe_ok = err.err();
		ASSERT_TRUE(maybe_ok.is_some());
		ASSERT_EQ(maybe_ok.unwrap().message(), "TestErrorMessage");
	}

	TEST(ResultTest, errerrPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		auto maybe_ok = err.err();
		ASSERT_TRUE(maybe_ok.is_some());
		ASSERT_EQ(maybe_ok.unwrap()->message(), "TestErrorMessage");
	}

	TEST(ResultTest, okMapValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map([](const bool _val) noexcept -> int {
						ignore(_val);
						return 2;
					}).unwrap(),
				  2);
	}

	TEST(ResultTest, okMapPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map([](const bool* _val) noexcept -> int {
						ignore(_val);
						return 2;
					}).unwrap(),
				  2);
	}

	TEST(ResultTest, errMapValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map([](const bool _val) noexcept -> int {
						 ignore(_val);
						 return 2;
					 })
					  .unwrap_err()
					  .message(),
				  "TestErrorMessage");
	}

	TEST(ResultTest, errMapPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map([](const bool _val) noexcept -> int {
						 ignore(_val);
						 return 2;
					 })
					  .unwrap_err()
					  ->message(),
				  "TestErrorMessage");
	}

	TEST(ResultTest, okMapOrValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map_or(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  4),
				  2);
	}

	TEST(ResultTest, okMapOrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map_or(
					  [](const bool* _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  4),
				  2);
	}

	TEST(ResultTest, errMapOrValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map_or(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  4),
				  4);
	}

	TEST(ResultTest, errMapOrPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map_or(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  4),
				  4);
	}

	TEST(ResultTest, okMapOrElseValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map_or_else(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  []() { return 4; }),
				  2);
	}

	TEST(ResultTest, okMapOrElsePointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		ASSERT_EQ(ok.map_or_else(
					  [](const bool* _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  []() { return 4; }),
				  2);
	}

	TEST(ResultTest, errMapOrElseValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map_or_else(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  []() { return 4; }),
				  4);
	}

	TEST(ResultTest, errMapOrElsePointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		ASSERT_EQ(err.map_or_else(
					  [](const bool _val) noexcept -> int {
						  ignore(_val);
						  return 2;
					  },
					  []() { return 4; }),
				  4);
	}

	TEST(ResultTest, okMapErrValue) {
		auto value = true;
		Result<bool, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		// NOLINTNEXTLINE(performance-unnecessary-value-param)
		ASSERT_EQ(ok.map_err([](const Error _val) noexcept -> Error {
						ignore(_val);
						return Error("TestErrorMessage");
					}).unwrap(),
				  true);
	}

	TEST(ResultTest, okMapErrPointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		Result<bool*, Error> ok = Ok(value);

		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());

		// NOLINTNEXTLINE(performance-unnecessary-value-param)
		ASSERT_EQ(*ok.map_err([](const Error _val) noexcept -> Error {
						 ignore(_val);
						 return Error("TestErrorMessage");
					 }).unwrap(),
				  true);
	}

	TEST(ResultTest, errMapErrValue) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error> err = Err(error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		// NOLINTNEXTLINE(performance-unnecessary-value-param)
		ASSERT_EQ(err.map_err([](const Error _val) noexcept -> Error {
						 ignore(_val);
						 return Error("TestErrorMessage2");
					 })
					  .unwrap_err()
					  .message(),
				  "TestErrorMessage2");
	}

	TEST(ResultTest, errMapErrPointer) {
		auto error = Error("TestErrorMessage");
		Result<bool, Error*> err = Err(&error);

		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());

		// NOLINTNEXTLINE(performance-unnecessary-value-param)
		ASSERT_EQ(err.map_err([](const Error* _val) noexcept -> Error {
						 ignore(_val);
						 return Error("TestErrorMessage2");
					 })
					  .unwrap_err()
					  .message(),
				  "TestErrorMessage2");
	}

	// NOLINTNEXTLINE(misc-definitions-in-headers)
	auto ok_move_test(Result<bool*, Error>&& ok) noexcept -> void {
		ASSERT_TRUE(ok.is_ok());
		ASSERT_TRUE(ok);
		ASSERT_FALSE(ok.is_err());
		ASSERT_TRUE(*ok.unwrap());
	}

	TEST(ResultTest, okMovePointer) {
		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		auto* value = new bool(true);
		{
			Result<bool*, Error> ok = Ok(value);
			ok_move_test(std::move(ok));
		}

		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
		delete value;
	}

	// NOLINTNEXTLINE(misc-definitions-in-headers)
	auto err_move_test(Result<bool*, Error*>&& err) noexcept -> void {
		ASSERT_FALSE(err.is_ok());
		ASSERT_FALSE(err);
		ASSERT_TRUE(err.is_err());
		ASSERT_EQ(err.unwrap_err()->message(), "TestErrorMessage");
	}

	TEST(ResultTest, errMovePointer) {
		auto error = Error("TestErrorMessage");
		{
			Result<bool*, Error*> err = Err(&error);
			err_move_test(std::move(err));
		}
	}
} // namespace hyperion::test
